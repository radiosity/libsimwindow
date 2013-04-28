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

#include "DataSource.hpp"

using std::string;
using std::auto_ptr; 
using std::ifstream;
using std::future;
using std::async; 
using std::function; 
using std::packaged_task;
using std::vector;

namespace libsim 
{
	
template <class T>
class FileSourceImpl;
	
template <class T>
class FileSource : public DataSource<T> {
	
	private:
		auto_ptr<FileSourceImpl<T>> impl;
	
	public:
		FileSource(string _filename, unsigned int _windowsize, int _datapoints = -1 ) : DataSource<T>(_windowsize)
		{
			impl = auto_ptr<FileSourceImpl<T>>(new FileSourceImpl<T>(_filename, _windowsize, _datapoints));
		}
		
		//No copying. That would leave this object in a horrendous state
		//and I don't want to figure out how to do it. 
		FileSource(FileSource<T> const & cpy) = delete; 
		FileSource<T>& operator =(const FileSource<T>& cpy) = delete; 
		
		//Moving is fine, so support rvalue move and move assignment operators.
		FileSource(FileSource<T> && mv) : DataSource<T>(mv.windowsize), impl(mv.impl) {}
		FileSource<T>& operator =(FileSource<T> && mv) { impl = mv.impl; return *this; }
		~FileSource() = default; 
		
		virtual T * get() override { return impl->get(); };
		virtual void tock() override { impl->tock(); };
		virtual bool eods() override { impl->eods(); };

};

template <class T>
class FileSourceImpl {
	
	private:
		vector<T> data;
		ifstream file;
		future<void> ft; 
	
		int datapoints_limit; 
		unsigned int datapoints_read; 
		unsigned int windowsize; 
		unsigned int validwindow; 
		unsigned int start; 
	
		bool ready; 
	
		unsigned int read_extent;
		
	public:
		FileSourceImpl(string filename, unsigned int _windowsize, int datapoints = -1)  :
			data(), 
			file(filename), 
			ft(),
			datapoints_limit(datapoints),
			datapoints_read(0),
			windowsize(_windowsize),
			validwindow(0),
			start(0),
			ready(false)
		{
			
			//First, calculate the read extent from the window size. 
			//What I'd like to do is to read at least three times the 
			//windowsize, rounded up to the nearest multiple of 1024.
			//
			//1024 is by no means a magic number supposed to align the 
			//read to a page boundary or somesuch; the input file is 
			//ASCII formatted T, so the number of lines read will only 
			//losely conform to a number of bytes. 1024 is Just a nice number. 
			
			read_extent = 
				(unsigned int) 
					floor(
						(
							(
								(double) (windowsize * 3)
							)
						) / 1024.0
					) * 1024;
			
			data.reserve(read_extent);

			auto func = [&]() {
				
				unsigned int i = 0;
				validwindow = 0; 
				
				for( ; i < read_extent; i++)  {
					if(datapoints_read == datapoints_limit) break; 
					if(file.eof()) break;
					file >> data[i];
					datapoints_read++;
				}
				
				validwindow = i + 1;
				
				ready = true; 
				
			};
			
 			ft = async(func);
			
		}
		
		//Absolutely no copying. 
		FileSourceImpl(FileSourceImpl<T> const & cpy) = delete; 
		FileSourceImpl<T>& operator =(const FileSourceImpl<T>& cpy) = delete; 
		
		FileSourceImpl(FileSourceImpl<T> && mv) = delete; 
		FileSourceImpl<T>& operator =(FileSourceImpl<T> && mv) = delete; 
		~FileSourceImpl() = default; 
		
		T * get() { 
			if(!ready) {
				ft.get();
			}
			return data.data() + start;
		}
		
		void tock() {
			start++;
			if(start == ((windowsize * 2) - 1)) {
			
				//Time to load a new file slice. 
				
				//Mark as unready. 
				ready = false; 
				
				//Redisg the read_extent. We read in three, one is left, so we only need
				//to read in two. This isn't quite optimal as it recalculates for each time
				//this function runs. I'll figure that out later. 
				
				read_extent = 
				(unsigned int) 
					floor(
						(
							(
								(double) (windowsize * 2)
							)
						) / 1024.0
					) * 1024;
				
				auto func = [&]() {
					
					//First things first, lets delete the items in the vector that we 
					//no longer need. 
					
					data.erase(data.begin(), data.begin() + (windowsize * 2));
					
					//Reset the start and the valid window
					
					start = 0; 
					validwindow = 0; 
				
					//Now the load
					
					unsigned int i = 0; 
					
					for( ; i < read_extent; i++)  {
						if(datapoints_read == datapoints_limit) break; 
						if(file.eof()) break;
						file >> data[i];
						datapoints_read++;
					}
					
					validwindow = (i+1) + windowsize;
					
					ready = true; 
				
				};
				
				ft = async(func);
				
			}
		}
		
		inline bool eods() const {
			//end of data stream. 
			//
			//basically, is the window still valid. 
			//
			//This is fairly easy to check, as it comes down to two things
			//Firstly. Is the pointer within the valid window. 
			
			//If there are 30 valid items in the array, there are items 
			//from 0-29. 
			//
			//The last valid window would be from 20-29. The window becomes
			//invalid when the start pointer is equal to 21 or more. So:
			//		   30      31    21
			if(start >= ((validwindow +1) - windowsize) return true; 
				
			//Secondly, is the pointer within the maximum window. This follows
			//the same idea, but uses the maximum windowsize (data.size())
			//for situations where 
			if(start >= ((data.size() +1) - windowsize) return true;
			
			return false; 
			
		}
	
};

}

#endif
