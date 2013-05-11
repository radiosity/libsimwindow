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
This class implements a window over a ring. It operates over a vector for now. 
*/

#ifndef RingSource_HEADER
#define RingSource_HEADER

#include <vector>
#include <utility>

#include "DataSource.hpp"

using std::vector;
using std::move; 

namespace libsim 
{

class RingSourceInvalidException : public exception {

	virtual const char * what()  const noexcept {
		return "vector is smaller than the windowsize";
	}
	
};
	
template<class T>
class RingSource : public DataSource<T> {
	
	private:
		vector<T> data;
		vector<T> patch; 
		unsigned int start; 

	public:
		RingSource(vector<T> _data, unsigned int _windowsize) : 
			DataSource<T>(_windowsize), 
			data(_data), 
			patch(),
			start(0) 
		{
			
			if(_windowsize > data.size()) throw RingSourceInvalidException();
			
			patch.reserve((_windowsize - 1) * 2);
			
			patch.insert(patch.begin(), data.end() - (_windowsize - 1) , data.end());
			patch.insert(patch.end(), data.begin(), data.begin() + (_windowsize - 1));
			
		}
		
		RingSource(RingSource<T> const & cpy) = delete; 
		RingSource<T>& operator =(const RingSource<T>& cpy) = delete; 
	
		//Moving is fine, so support rvalue move and move assignment operators.
		RingSource(RingSource<T> && mv) : DataSource<T>(mv.windowsize), data(move(mv.data)), patch(move(mv.patch)), start(mv.start) {}
		RingSource<T>& operator =(RingSource<T> && mv) { data = move(mv.data); data = move(mv.patch); start = mv.start; return *this; }
		~RingSource() = default; 
    
		//get a pointer to the start of the window
		T * get()  {
			
			unsigned int m = start % data.size(); 
			
			if(m <  (data.size() - (DataSource<T>::windowsize - 1))) {
				//we're on the main data. 
				return data.data() + m; 
			}
			else {
				//we're on the patch. 
				unsigned int idx = m - (data.size() - (DataSource<T>::windowsize - 1)); 
				return patch.data() + idx; 
			}
			
		}
		
		//increment the start pointer
		void tick() { start++; }
		
		//check that the window is still valid. This is always with a ring source.
		bool eods() { return false;  }
		
};

}

#endif