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
		
		//Unfortunately the get operator can't be const, as it may (as in this case)
		//need to modify state for (for example) reading from files. 
		T get() override { return 0; };
		void tock() override {};

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
				
				for(int i = 0; i < read_extent; i++)  {
					if(datapoints_read == datapoints_limit) break; 
					file >> data[i];
					datapoints_read++;
				}
				
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

};

}

#endif


