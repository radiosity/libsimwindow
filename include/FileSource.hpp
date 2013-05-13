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

#include <string>
#include <cmath>
#include <exception>
#include <memory>
#include <algorithm>
#include <limits>

#include "DataSource.hpp"
#include "AsyncIOImpl.hpp"

using std::string;
using std::unique_ptr; 
using std::ifstream;
using std::exception; 
using std::move;
using std::numeric_limits;

namespace libsim 
{
	
template <class T>
class FileSourceImpl;
	
template <class T>
class FileSource : public DataSource<T> {
	
	private:
		unique_ptr<FileSourceImpl<T>> impl;
	
	public:
		FileSource(string _fn, unsigned int _wsize, launch _policy, int _datapoints) : DataSource<T>(_wsize)
		{
			impl = unique_ptr<FileSourceImpl<T>>(new FileSourceImpl<T>(_fn, _wsize, _datapoints));
		}
		
		FileSource(string _fn, unsigned int _wsize, int _datapoints) : DataSource<T>(_wsize)
		{
			impl = unique_ptr<FileSourceImpl<T>>(new FileSourceImpl<T>(_fn, _wsize, launch::deferred, _datapoints));
		}
			
		FileSource(string _fn, unsigned int _wsize, launch _policy) : DataSource<T>(_wsize)
		{
			impl = unique_ptr<FileSourceImpl<T>>(new FileSourceImpl<T>(_fn, _wsize, _policy, numeric_limits<unsigned int>::max()));
		}
		
		FileSource(string _fn, unsigned int _wsize) : DataSource<T>(_wsize)
		{
			impl = unique_ptr<FileSourceImpl<T>>(new FileSourceImpl<T>(_fn, _wsize, launch::deferred, numeric_limits<unsigned int>::max()));
		}
		
		//No copying. That would leave this object in a horrendous state
		//and I don't want to figure out how to do it. 
		FileSource(FileSource<T> const & cpy) = delete; 
		FileSource<T>& operator =(const FileSource<T>& cpy) = delete; 
	
		//Moving is fine, so support rvalue move and move assignment operators.
		FileSource(FileSource<T> && mv) : DataSource<T>(mv.windowsize), impl(move(mv.impl)) {}
		FileSource<T>& operator =(FileSource<T> && mv) { impl = move(mv.impl); return *this; }
		~FileSource() = default; 
		
		inline virtual T * get() override { return impl->get(); };
		inline virtual void tick() override { impl->tock(); };
		inline virtual bool eods() override { return impl->eods(); };

};

template <class T>
class FileSourceImpl : public AsyncIOImpl<T> {
	
	private:
		ifstream file;
	
		virtual vector<T> ioinit() override { 
			
			auto tmpdata = vector<T>();
			tmpdata.reserve(this->read_extent);
			
			unsigned int i = 0;
			
			for( ; i < this->read_extent; i++)  {
				if(this->datapoints_read == this->datapoints_limit) break; 
				if(file.eof()) break;
				
				string stemp; 
				getline(file, stemp);
				
				stringstream ss(stemp);
				T temp;
				ss >> temp; 
				
				tmpdata.push_back(temp);
				
				this->datapoints_read++;
			}
			
			this->readyio = true; 
			
			return tmpdata;
			
		}
		
		virtual vector<T> ionext() override {
		
			//Create and configure the 
			//return
			auto tmpdata = vector<T>();
			tmpdata.reserve(this->windowsize);
			
			//Now the load
			
			unsigned int i = 0; 
			
			for( ; i < this->windowsize; i++)  {
				if(this->datapoints_read == this->datapoints_limit) break; 
				if(file.eof()) break;
				
				string stemp; 
				getline(file, stemp);
				
				stringstream ss(stemp);
				T temp;
				ss >> temp; 
			
				tmpdata.push_back(temp);

				this->datapoints_read++;
			}
			
			this->readyio = true; 
			
			return tmpdata;
			
		}
	
		
	public:
		FileSourceImpl(string filename, unsigned int _wsize, launch _policy, unsigned int datapoints) :
			AsyncIOImpl<T>(_wsize, _policy, datapoints),
			file(filename) 
		{
			
			
		}
		
		//Absolutely no copying. 
		FileSourceImpl(FileSourceImpl<T> const & cpy) = delete; 
		FileSourceImpl<T>& operator =(const FileSourceImpl<T>& cpy) = delete; 
		
		FileSourceImpl(FileSourceImpl<T> && mv) = delete; 
		FileSourceImpl<T>& operator =(FileSourceImpl<T> && mv) = delete; 
		~FileSourceImpl() = default; 
		
};

}

#endif
