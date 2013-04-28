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

#include <string>
#include <utility>
#include <fstream>
#include <future>
#include <functional>

#include "DataSource.hpp"

using std::string;
using std::auto_ptr; 
using std::ifstream;
using std::future;
using std::async; 
using std::function; 

namespace libsim 
{

	
template <class T>
class FileSourceImpl;
	
template <class T>
class FileSource : public DataSource {
	
	public:
		FileSource(string filename, unsigned int windowsize, int datapoints = -1 )
	{
	
	}


};

template <class T>
class FileSourceImpl : public DataSource {
	
	private:
		auto_ptr<T> data;
		ifstream file;
		future<void> func; 
		
	public:
		FileSourceImpl(string filename, unsigned int windowsize, int datapoints = -1 ) :
			data(NULL), 
			file(filename) 
		{
	
		}

};

}

#endif


