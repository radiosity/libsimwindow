/*
Copyright (c) 2013, Richard Martin
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of Richard Martin nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL RICHARD MARTIN BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*

The purpose of this class (and its PIMPL) is to provide a useful implementation 
of a sliding window which allows pointer-base access to the fundamental datatype 
array. This is mostly useful for compatability with existing C programmes. Chief 
among these, at least the primary motivation for this library, is to interface 
with the GNU Scientific Library, which expects arrays of base types. 

Simply, the class looks like this:

........1010100110101111001010100100100........................

	|--------| window (valid 0 to 9 inc)
	 |--------| window + one tick. (valid 1 to 10 inc)
 
But this data is a member of (potentially) larger file:

0100010010101001101011110010101001001000101100101111101110101000

This class provides a mechanism for the asynchronous loading of data from the file
and presenting it in a moving-window interface that is compatabile with the 
DataSource parent class.

*/


#ifndef FileSource_HEADER
#define FileSource_HEADER

#include <vector>
#include <string>
#include <utility>
#include <fstream>
#include <future>
#include <functional>
#include <cmath>
#include <iostream>
#include <sstream>
#include <exception>
#include <atomic>

#include "DataSource.hpp"

using std::string;
using std::auto_ptr; 
using std::ifstream;
using std::future;
using std::function;
using std::async; 
using std::vector;
using std::cout; 
using std::endl; 
using std::stringstream; 
using std::exception; 
using std::atomic; 
using std::launch;

namespace libsim 
{
	
class FileSourceInvalidException : public exception {

	virtual const char * what()  const noexcept {
		return "File source is invalid: no window, no pending io.";
	}
	
};
	
template <class T>
class FileSourceImpl;
	
template <class T>
class FileSource : public DataSource<T> {
	
	private:
		auto_ptr<FileSourceImpl<T>> impl;
	
	public:
		FileSource(string _filename, unsigned int _windowsize, launch _policy, int _datapoints) : DataSource<T>(_windowsize)
		{
			impl = auto_ptr<FileSourceImpl<T>>(new FileSourceImpl<T>(_filename, _windowsize, _datapoints));
		}
		
		FileSource(string _filename, unsigned int _windowsize, int _datapoints) : DataSource<T>(_windowsize)
		{
			impl = auto_ptr<FileSourceImpl<T>>(new FileSourceImpl<T>(_filename, _windowsize, launch::deferred, _datapoints));
		}
			
		FileSource(string _filename, unsigned int _windowsize, launch _policy) : DataSource<T>(_windowsize)
		{
			impl = auto_ptr<FileSourceImpl<T>>(new FileSourceImpl<T>(_filename, _windowsize, _policy, -1));
		}
		
		FileSource(string _filename, unsigned int _windowsize) : DataSource<T>(_windowsize)
		{
			impl = auto_ptr<FileSourceImpl<T>>(new FileSourceImpl<T>(_filename, _windowsize, launch::deferred, -1));
		}
		
		//No copying. That would leave this object in a horrendous state
		//and I don't want to figure out how to do it. 
		FileSource(FileSource<T> const & cpy) = delete; 
		FileSource<T>& operator =(const FileSource<T>& cpy) = delete; 
	
		//Moving is fine, so support rvalue move and move assignment operators.
		FileSource(FileSource<T> && mv) : DataSource<T>(mv.windowsize), impl(mv.impl) {}
		FileSource<T>& operator =(FileSource<T> && mv) { impl = mv.impl; return *this; }
		~FileSource() = default; 
		
		inline virtual T * get() override { return impl->get(); };
		inline virtual void tick() override { impl->tock(); };
		inline virtual bool eods() override { return impl->eods(); };

};

template <class T>
class FileSourceImpl {
	
	private:
		vector<T> data;
		ifstream file;
		future<vector<T>> ft; 
	
		const int datapoints_limit; 
		unsigned int datapoints_read; 
		const unsigned int windowsize;  
		unsigned int start; 
	
		atomic<bool> pendingio;
		atomic<bool> readyio; 
	
		const launch policy; 
	
		unsigned int read_extent;
	
		inline unsigned int nvalidwindows() const {
			
			//if the vector is empty, we definitely don't have 
			//any valid windows. 
			if(data.size() == 0) return 0; 
			
			//if the vector contains fewer elements than the
			//windowsize, then we don't have any valid windows
			if((data.size() - start) < windowsize) return 0;
			
			//otherwise, if we have the same (or more) elements
			//as the windowsize, then we have at least one window. 
			return ((data.size() - start) - windowsize) + 1; 
																		
		}
		
		inline  bool hasvalidwindow() const {
			return nvalidwindows() > 0; 
		}
		
		inline bool completed() const {
			return datapoints_read == datapoints_limit; 
		}
		
		inline void read()  {
			//remove past values
			data.erase(data.begin(), data.begin() + start);
			//get an append the new data
			auto tmpdata = ft.get();
			data.insert(data.end(), tmpdata.begin(), tmpdata.end());
			//update values. 
			start = 0; 
			pendingio = false; 
			readyio = false; 
		}
		
		inline void check()  {
			//If there's something to pick up, pick it up
			if(readyio) {
				read();
			}
			//If there are no valid windows, we need to 
			//check if there is a pending (but incomplete)
			//io operation. 
			if(!hasvalidwindow()) {
				if(pendingio) {
					read();
				}
				else {
					throw FileSourceInvalidException();
				}
			}
		}
			
		
	public:
		FileSourceImpl(string filename, unsigned int _windowsize, launch _policy, int datapoints)  :
			data(), 
			file(filename), 
			ft(),
			datapoints_limit(datapoints),
			datapoints_read(0),
			windowsize(_windowsize),
			start(0),
			pendingio(true),
			readyio(false),
			policy(_policy)
		{
			
			read_extent = windowsize * 3; 
			
			data.reserve(read_extent);
			
			if(!file.good()) throw -1; 

			ft = async(policy, [&]() {
				
				auto tmpdata = vector<T>();
				tmpdata.reserve(read_extent);
				
				unsigned int i = 0;
				
				for( ; i < read_extent; i++)  {
					if((datapoints_read + i) == datapoints_limit) break; 
					if(file.eof()) break;
					
					string stemp; 
					getline(file, stemp);
					
					stringstream ss(stemp);
					T temp;
					ss >> temp; 
					
					tmpdata.push_back(temp);
					
					datapoints_read++;
				}
				
				readyio = true; 
				
				return tmpdata;
				
			});
			
		}
		
		//Absolutely no copying. 
		FileSourceImpl(FileSourceImpl<T> const & cpy) = delete; 
		FileSourceImpl<T>& operator =(const FileSourceImpl<T>& cpy) = delete; 
		
		FileSourceImpl(FileSourceImpl<T> && mv) = delete; 
		FileSourceImpl<T>& operator =(FileSourceImpl<T> && mv) = delete; 
		~FileSourceImpl() = default; 
		
		T * get() { 
			check();
			return data.data() + start;
		}
		
		void tock() {
			
			//Need to make sure that, if someone is going through the 
			//datastream quickly (skipping a few values for ex) then we 
			//don't miss a load. 
			check();
			
			start++;
			
			if(start == windowsize) {
			
				//Time to load a new file slice. 
			
				//Mark as pendingio. 
				pendingio = true; 
				
				ft = async(policy, [&]() {
					
					//Create and configure the 
					//return
					auto tmpdata = vector<T>();
					tmpdata.reserve(windowsize);
					
					//Now the load
					
					unsigned int i = 0; 
					
					for( ; i < windowsize; i++)  {
						if(datapoints_read == datapoints_limit) break; 
						if(file.eof()) break;
						
						string stemp; 
						getline(file, stemp);
						
						stringstream ss(stemp);
						T temp;
						ss >> temp; 
					
						tmpdata.push_back(temp);

						datapoints_read++;
					}
					
					readyio = true; 
					
					return tmpdata;
				
				});
				
			}
		}
		
		inline bool eods() {
			//End of data stream? Do we have a valid window
			
			//Make sure that we've done all the loading, if there are any 
			//IO operations pending
			check();
			
			return !hasvalidwindow();
			
		}
	
};

}

#endif
