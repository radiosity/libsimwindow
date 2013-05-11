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
This will NOT delete the pointer that's passed to it, so it is safe to use in situations where a single
buffer is allocated and passed to multiple SharedSources. This may be useful when, for example, you 
want to run an algorithm with varying windowsizes over a dataset, but without having to duplicate
all the data. 
*/

#ifndef SharedSource_HEADER
#define SharedSource_HEADER

#include <vector>

#include "DataSource.hpp"

using std::vector; 

namespace libsim 
{

template<class T>
class SharedSource : public DataSource<T> {
	
	private:
		T * data;
		const unsigned int size; 
		unsigned int start; 

	public:
		SharedSource(T * _data, unsigned int _size, unsigned int _windowsize) : DataSource<T>(_windowsize), data(_data), size(_size), start(0) {}
		
		SharedSource(SharedSource<T> const & cpy) = delete; 
		SharedSource<T>& operator =(const SharedSource<T>& cpy) = delete; 
	
		//Moving is fine, so support rvalue move and move assignment operators.
		SharedSource(SharedSource<T> && mv) : DataSource<T>(mv.windowsize), data(mv.data), size(mv.size), start(mv.start) {}
		SharedSource<T>& operator =(SharedSource<T> && mv) { data = mv.data; size = mv.size; start = mv.start; return *this; }
		~SharedSource() = default; 
    
		//get a pointer to the start of the window
		T * get()  {
			return data + start;
		}
		
		//increment the start pointer
		void tick() { start++; }
		
		//check that the window is still valid
		bool eods() { return start > (size - this->windowsize); }
		
};

}

#endif