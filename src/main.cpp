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

#define BOOST_TEST_MAIN 
#include <boost/test/included/unit_test.hpp>

#include <utility>
#include <iostream>
#include <future>
#include <functional>
#include <vector>

#include "FileSource.hpp"
#include "VectorSource.hpp"
#include "SharedSource.hpp"
#include "RingSource.hpp"
#include "SQLiteSource.hpp"

using std::cout; 
using std::endl; 
using std::future; 
using std::async;
using std::vector;

using namespace libsim;

BOOST_AUTO_TEST_CASE(filesource_test) {
	
	auto fs = FileSource<unsigned int>("test/data", 10);
	
	for(unsigned int i = 0 ; i < 30; i++) {
	
		BOOST_CHECK(!fs.eods());
	
		for (unsigned int j = 0 ; j < 10; j++) {
			BOOST_CHECK_EQUAL(i+j, fs.get()[j]);
		}
		
		fs.tick();
		
	}
	
}

BOOST_AUTO_TEST_CASE(filesource_test2) {
	
	auto fs = FileSource<unsigned int>("test/data", 5, 30);
	
	for(unsigned int i = 0 ; i <= 25; i++)  {
	
		BOOST_CHECK(!fs.eods());
		
		for (unsigned int j = 0 ; j < 5; j++) {
			BOOST_CHECK_EQUAL(i+j, fs.get()[j]);
		}
		
		fs.tick();
		
	}
	
	BOOST_CHECK(fs.eods());
	
}

// Vectors

BOOST_AUTO_TEST_CASE(vectorsource_test) {
	
	auto data = vector<unsigned int>();
	for(unsigned int i = 0; i < 30; i++) {
		data.push_back(i);
	}
	
	auto fs = VectorSource<unsigned int>(data, 5);
	
	for(unsigned int i = 0 ; i <= 25; i++)  {
	
		BOOST_CHECK(!fs.eods());
		
		for (unsigned int j = 0 ; j < 5; j++) {
			BOOST_CHECK_EQUAL(i+j, fs.get()[j]);
		}
		
		fs.tick();
		
	}
	
	BOOST_CHECK(fs.eods());
	
}

// Shared

BOOST_AUTO_TEST_CASE(sharedsource_test) {
	
	unsigned int * data = new unsigned int[30];
	for(unsigned int i = 0; i < 30; i++) {
		data[i]=i;
	}
	
	auto fs = SharedSource<unsigned int>(data, 30, 5);
	
	for(unsigned int i = 0 ; i <= 25; i++)  {
	
		BOOST_CHECK(!fs.eods());
		
		for (unsigned int j = 0 ; j < 5; j++) {
			BOOST_CHECK_EQUAL(i+j, fs.get()[j]);
		}
		
		fs.tick();
		
	}
	
	BOOST_CHECK(fs.eods());
	
	delete data;
	
}

// Ring

BOOST_AUTO_TEST_CASE(ringsource_test) {
	
	auto data = vector<unsigned int>();
	for(unsigned int i = 0; i < 6; i++) {
		data.push_back(i);
	}
	
	auto fs = RingSource<unsigned int>(data, 5);
	
	for(unsigned int i = 0 ; i <= 12; i++)  {
		
		for (unsigned int j = 0 ; j < 5; j++) {
			BOOST_CHECK_EQUAL((i+j) % 6, fs.get()[j]);
		}
		
		fs.tick();
		
	}
	
}

// Sqlite

BOOST_AUTO_TEST_CASE(sqlite3_test) {

  
	sqlite3 * database;
	sqlite3_open("test/testdb", &database);
	
	string sql = "SELECT * from test LIMIT ? OFFSET ?;";
	
	auto fs = SQLiteSource<unsigned int>(database, sql, 5);
	
	for(unsigned int i = 1 ; i <= 40; i++)  {
		
		for (unsigned int j = 0 ; j < 5; j++) {
			BOOST_CHECK_EQUAL(i+j, fs.get()[j]);
		}
		
		fs.tick();
		
	}
	
	sqlite3_close(database);
	
}


