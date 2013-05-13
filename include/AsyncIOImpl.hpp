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

The purpose of this class  is to provide a useful implementation 
of a sliding window which allows pointer-base access to the fundamental datatype 
array. This is mostly useful for compatability with existing C programmes. Chief 
among these, at least the primary motivation for this library, is to interface 
with the GNU Scientific Library, which expects arrays of base types. 

Simply, the class looks like this:

........1010100110101111001010100100100........................

	|--------| window (valid 0 to 9 inc)
	 |--------| window + one tick. (valid 1 to 10 inc)
 
But this data is a member of (potentially) larger file/database/whatever:

0100010010101001101011110010101001001000101100101111101110101000

This class provides a mechanism for the asynchronous loading of data from the whatever
and presenting it in a moving-window interface.

*/


#ifndef AsyncIOImpl_HEADER
#define AsyncIOImpl_HEADER

#include <vector>
#include <string>
#include <utility>
#include <future>
#include <functional>
#include <cmath>
#include <atomic>
#include <memory>
#include <algorithm>

#include "DataSource.hpp"

using std::string;
using std::unique_ptr; 
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
using std::move;

namespace libsim 
{
	
class AsyncIOInvalidException : public exception {

	virtual const char * what()  const noexcept {
		return "IO source is invalid: no window, no pending io.";
	}
	
};

template <class T>
class AsyncIOImpl {
	
	protected:
		vector<T> data;
		future<vector<T>> ft; 
	
		const unsigned int datapoints_limit; 
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
					throw AsyncIOInvalidException();
				}
			}
		}
			
		virtual vector<T> ioinit() = 0; 
		virtual vector<T> ionext() = 0; 
		
	public:
		AsyncIOImpl(unsigned int _wsize, launch _policy, int datapoints)  :
			data(), 
			ft(),
			datapoints_limit(datapoints),
			datapoints_read(0),
			windowsize(_wsize),
			start(0),
			pendingio(true),
			readyio(false),
			policy(_policy)
		{
			
			read_extent = windowsize * 3; 
			
			data.reserve(read_extent);
			
			function<vector<T>()> finit = [&]() { return this->ioinit(); };
			
			ft = async(policy, finit);
			
		}
		
		//Absolutely no copying. 
		AsyncIOImpl(AsyncIOImpl<T> const & cpy) = delete; 
		AsyncIOImpl<T>& operator =(const AsyncIOImpl<T>& cpy) = delete; 
		
		AsyncIOImpl(AsyncIOImpl<T> && mv) = delete; 
		AsyncIOImpl<T>& operator =(AsyncIOImpl<T> && mv) = delete; 
		~AsyncIOImpl() = default; 
		
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
				
				function<vector<T>()> fnext = [&](){ return ionext(); }; 
			
				ft = async(policy, fnext);
				
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