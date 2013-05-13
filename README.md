#libsimwindow

libsimwindow is a library of header-only functions designed to provide consisten access to mechanisms to vector, file, and raw pointer data in a format that is compatible with functions in the GNU Scientific Library. These classes utilise the advanced features of C++11 heavily and therefore require a C++11 compatabile compiler. 

FileSource:
--
FileSource provides access to data stored in a file on a filesystem. It does so in a memory-efficient manner by reading in only the sections of data that are required, and performing the reads asynchronously (if required) utilising the async and future mechanisms of C++11. 

You need to link with pthread. No seriously, LINK WITH PTHREAD. If you don't, the resultant programme will silently fail; you won't get a compile warning because the C++11 libraries pull in pthread at runtime.ed

Copies of this class are NOT supported; it is reccomended to explicityly std::move() the object. 

VectorSource:
--
This takes in a vector and iterates over it; the vector will be copied so be careful with large datasets here. 

Copies of this class are NOT supported; it is reccomended to explicityly std::move() the object. 

SharedSource:
--
This takes in a pointer to an array and can therefore a number of such objects can share a dataset. 

Copies of this class are NOT supported; it is reccomended to explicityly std::move() the object. 

RingSource:
--
This class takes in a vector and guarantees that for all indexes there is a contiguous array to return, utilising the vector in a circular manner. 

Copies of this class are NOT supported; it is reccomended to explicityly std::move() the object. 

SQLiteSource:
--
This class takes in a sqlite3 database via pointer, as well as a SQL statement to execute in order to return interesting rows. This SQL statement WILL BE EXECUTED VERBATIM. It has no qualms about destroying your entire DB, so do NOT use this in situations where the input is not trusted. 

The SQL query must provide two bindable parameters, "LIMIT ? OFFSET ?" as this is utilised by the underlying functions to provide the sliding window. In theory, this is the only restriction; complicated selects should be possible if you need to use them (although understand that this will negatively alter the performance of the class). Also be aware that the window may be damaged by changes to the underlying database. 

In various situations where I've tried it, SQLite hasn't given me any problems being used in a multi-threaded environment (although this is cautioned in the SQLite documentation). If this is the case, you may need to sqlite3_config(SQLITE_CONFIG_MULTITHREAD) before loading the database. 

Copies of this class are NOT supported; it is reccomended to explicityly std::move() the object. 