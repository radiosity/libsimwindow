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
This class will NOT perform input validation. Of any kind. CHECK YOUR INPUTS. If you want to pass
drop table, you can, and it'll run it. 
*/

#ifndef SQLiteSource_HEADER
#define SQLiteSource_HEADER

#include <vector>
#include <string>
#include <utility>
#include <future>
#include <functional>
#include <cmath>
#include <iostream>
#include <sstream>
#include <exception>
#include <atomic>
#include <memory>
#include <algorithm>
#include <limits>
#include <sqlite3.h>

#include "DataSource.hpp"

using std::string;
using std::unique_ptr; 
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
using std::numeric_limits;

namespace libsim 
{
	
class SQLiteSourceInvalidException : public exception {

	virtual const char * what()  const noexcept {
		return "SQLite source is invalid: no window, no pending io.";
	}
	
};
	
template <class T>
class SQLiteSourceImpl;
	
template <class T>
class SQLiteSource : public DataSource<T> {
	
	private:
		unique_ptr<SQLiteSourceImpl<T>> impl;
	
	public:
		SQLiteSource(sqlite3 * _db, string _query, unsigned int _windowsize, launch _policy, int _datapoints) : DataSource<T>(_windowsize)
		{
			impl = unique_ptr<SQLiteSourceImpl<T>>(new SQLiteSourceImpl<T>(_db, _query, _windowsize, _datapoints));
		}
		
		SQLiteSource(sqlite3 * _db, string _query, unsigned int _windowsize, int _datapoints) : DataSource<T>(_windowsize)
		{
			impl = unique_ptr<SQLiteSourceImpl<T>>(new SQLiteSourceImpl<T>(_db, _query, _windowsize, launch::deferred, _datapoints));
		}
			
		SQLiteSource(sqlite3 * _db, string _query, unsigned int _windowsize, launch _policy) : DataSource<T>(_windowsize)
		{
			impl = unique_ptr<SQLiteSourceImpl<T>>(new SQLiteSourceImpl<T>(_db, _query, _policy, numeric_limits<unsigned int>::max()));
		}

		SQLiteSource(sqlite3 * _db, string _query, unsigned int _windowsize) : DataSource<T>(_windowsize)
		{
			impl = unique_ptr<SQLiteSourceImpl<T>>(new SQLiteSourceImpl<T>(_db, _query, _windowsize, launch::deferred, numeric_limits<unsigned int>::max()));
		}
		
		//No copying. That would leave this object in a horrendous state
		//and I don't want to figure out how to do it. 
		SQLiteSource(SQLiteSource<T> const & cpy) = delete; 
		SQLiteSource<T>& operator =(const SQLiteSource<T>& cpy) = delete; 
	
		//Moving is fine, so support rvalue move and move assignment operators.
		SQLiteSource(SQLiteSource<T> && mv) : DataSource<T>(mv.windowsize), impl(move(mv.impl)) {}
		SQLiteSource<T>& operator =(SQLiteSource<T> && mv) { impl = move(mv.impl); return *this; }
		~SQLiteSource() = default; 
		
		inline virtual T * get() override { return impl->get(); };
		inline virtual void tick() override { impl->tock(); };
		inline virtual bool eods() override { return impl->eods(); };

};

template<>
class SQLiteSourceImpl<double> : public AsyncIOImpl<double> {

	private:
		sqlite3 * const db;
		sqlite3_stmt * statement;
	
		virtual vector<double> ioinit() override { 
			
			auto tmpdata = vector<double>();
			tmpdata.reserve(this->read_extent);
			
			unsigned int i = 0;
			
			sqlite3_bind_int(statement, 1, this->windowsize * 3);
			sqlite3_bind_int(statement, 2, this->datapoints_read);

			int res = sqlite3_step(statement);
			
			for( ; i < this->read_extent; i++)  {
				if(this->datapoints_read == this->datapoints_limit) break; 
				if(res != SQLITE_ROW) break;
				
				double temp; 
				
				temp = sqlite3_column_double(statement, 0);
				
				tmpdata.push_back(temp);
				
				this->datapoints_read++;
				
				res = sqlite3_step(statement);
			}
			
			sqlite3_reset(statement);
			
			this->readyio = true; 
			
			return tmpdata;
			
		}
		
		virtual vector<double> ionext() override {
		
			//Create andconfigure the 
			//return
			auto tmpdata = vector<double>();
			tmpdata.reserve(this->windowsize);
			
			//Now the load
			
			unsigned int i = 0; 
			
			sqlite3_bind_int(statement, 1, this->windowsize);
			sqlite3_bind_int(statement, 2, this->datapoints_read);
			
			int res = sqlite3_step(statement);
			
			for( ; i < this->windowsize; i++)  {
				if(this->datapoints_read == this->datapoints_limit) break; 
				if(res != SQLITE_ROW) break;
				
				double temp; 
				
				temp = sqlite3_column_double(statement, 0);
				
				tmpdata.push_back(temp);
				
				this->datapoints_read++;
				
				res = sqlite3_step(statement);
			}
			
			sqlite3_reset(statement);
			
			this->readyio = true; 
			
			return tmpdata;
			
		}
	
		
	public:
		SQLiteSourceImpl(sqlite3 * _db, string _query, unsigned int _wsize, launch _policy, unsigned int datapoints) :
			AsyncIOImpl<double>(_wsize, _policy, datapoints),
			db(_db) 
		{
			
			int result = sqlite3_prepare_v2(db, _query.c_str(), -1, &statement, 0);	
			if(result != SQLITE_OK && result != SQLITE_DONE) throw -1;

		}
		
		//Absolutely no copying. _
		SQLiteSourceImpl(SQLiteSourceImpl<double> const & cpy) = delete; 
		SQLiteSourceImpl<double>& operator =(const SQLiteSourceImpl<double>& cpy) = delete; 
		
		SQLiteSourceImpl(SQLiteSourceImpl<double> && mv) = delete; 
		SQLiteSourceImpl<double>& operator =(SQLiteSourceImpl<double> && mv) = delete; 
		~SQLiteSourceImpl() {
			
			sqlite3_finalize(statement);
			
		}			
		
};


template<>
class SQLiteSourceImpl<unsigned int> : public AsyncIOImpl<unsigned int> {

	private:
		sqlite3 * const db;
		sqlite3_stmt * statement;
	
		virtual vector<unsigned int> ioinit() override { 
			
			auto tmpdata = vector<unsigned int>();
			tmpdata.reserve(this->read_extent);
			
			unsigned int i = 0;
			
			sqlite3_bind_int(statement, 1, this->windowsize * 3);
			sqlite3_bind_int(statement, 2, this->datapoints_read);
			
			int res = sqlite3_step(statement);
			
			for( ; i < this->read_extent; i++)  {
				if(this->datapoints_read == this->datapoints_limit) break; 
				if(res != SQLITE_ROW) break;
				
				unsigned int temp; 
				
				temp = (unsigned int) sqlite3_column_int(statement, 0);
				
				tmpdata.push_back(temp);
				
				this->datapoints_read++;
				
				res = sqlite3_step(statement);
			}
			
			sqlite3_reset(statement);
			
			this->readyio = true; 
			
			return tmpdata;
			
		}
		
		virtual vector<unsigned int> ionext() override {
		
			//Create andconfigure the 
			//return
			auto tmpdata = vector<unsigned int>();
			tmpdata.reserve(this->windowsize);
			
			//Now the load
			
			unsigned int i = 0; 
			
			sqlite3_bind_int(statement, 1, this->windowsize);
			sqlite3_bind_int(statement, 2, this->datapoints_read);
			
			int res = sqlite3_step(statement);
			
			for( ; i < this->windowsize; i++)  {
				if(this->datapoints_read == this->datapoints_limit) break; 
				if(res != SQLITE_ROW) break;
				
				unsigned int temp; 
				
				temp = (unsigned int) sqlite3_column_int(statement, 0);
				
				tmpdata.push_back(temp);
				
				this->datapoints_read++;
				
				res = sqlite3_step(statement);
			}
			
			sqlite3_reset(statement);
			
			this->readyio = true; 
			
			return tmpdata;
			
		}
	
		
	public:
		SQLiteSourceImpl(sqlite3 * _db, string _query, unsigned int _wsize, launch _policy, unsigned int datapoints) :
			AsyncIOImpl<unsigned int>(_wsize, _policy, datapoints),
			db(_db) 
		{
			
			int result = sqlite3_prepare_v2(db, _query.c_str(), -1, &statement, 0);	
			if(result != SQLITE_OK && result != SQLITE_DONE) throw -1;

		}
		
		//Absolutely no copying. _
		SQLiteSourceImpl(SQLiteSourceImpl<unsigned int> const & cpy) = delete; 
		SQLiteSourceImpl<unsigned int>& operator =(const SQLiteSourceImpl<unsigned int>& cpy) = delete; 
		
		SQLiteSourceImpl(SQLiteSourceImpl<unsigned int> && mv) = delete; 
		SQLiteSourceImpl<unsigned int>& operator =(SQLiteSourceImpl<unsigned int> && mv) = delete; 
		~SQLiteSourceImpl() {
			
			sqlite3_finalize(statement);
			
		}			
		
};


}

#endif