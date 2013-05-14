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

#ifndef MutableSource_HEADER
#define MutableSource_HEADER

#include <vector>
#include <utility>

#include "VectorSource.hpp"

using std::vector;
using std::move; 

namespace libsim 
{

template<class T>
class MutableSource : public VectorSource<T> {

	public:
		MutableSource(vector<T> _data, unsigned int _windowsize) : VectorSource<T>(_data, _windowsize) {}
		
		//Again, no copying
		MutableSource(MutableSource<T> const & cpy) = delete; 
		MutableSource<T>& operator =(const MutableSource<T>& cpy) = delete; 
	
		//Moving is fine, so support rvalue move and move assignment operators.
		MutableSource(MutableSource<T> && mv) : VectorSource<T>(mv.data, mv.windowsize) {}
		MutableSource<T>& operator = (MutableSource<T> && mv) { this->data = move(mv.data); this->start = mv.start; return *this; }
		~MutableSource() = default; 
    
		void push_back(T temp) {
		
			this->data.push_back(temp);
		  
		}
		
};

}

#endif