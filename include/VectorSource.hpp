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
The vector data source is simple; it takes in a vector of data (compiled by another class etc) and 
utilises it as the data passed out through the window. 
*/

#ifndef VectorSource_HEADER
#define VectorSource_HEADER

#include <vector>

#include "DataSource.hpp"

using std::vector; 

namespace libsim 
{

template<class T>
class VectorSource : public DataSource<T> {
	
	private:
		vector<T> data;
		unsigned int start; 

	public:
		VectorSource(vector<T> _data, unsigned int _windowsize) : DataSource<T>(_windowsize), data(_data), start(0) {}
		
		VectorSource(VectorSource<T> const & cpy) = delete; 
		VectorSource<T>& operator =(const VectorSource<T>& cpy) = delete; 
	
		//Moving is fine, so support rvalue move and move assignment operators.
		VectorSource(VectorSource<T> && mv) : DataSource<T>(mv.windowsize), data(mv.data), start(mv.start) {}
		VectorSource<T>& operator =(VectorSource<T> && mv) { data = mv.data; start = mv.start; return *this; }
		~VectorSource() = default; 
    
		//get a pointer to the start of the window
		T * get()  {
			return data.data() + start;
		}
		
		//increment the start pointer
		void tick() { start++; }
		
		//check that the window is still valid
		bool eods() { return start > (data.size() - DataSource<T>::windowsize); }
		
};

}

#endif