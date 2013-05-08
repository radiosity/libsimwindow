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