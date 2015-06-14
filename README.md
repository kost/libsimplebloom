# libsimplebloom
Small bloom filter implementation in plain C with utils

Introduction
------------
This is libsimplebloom, a simple and small bloom filter implementation in C.

If you are reading this you probably already know about bloom filters
and why you might use one. If not, the wikipedia article is a good intro:
http://en.wikipedia.org/wiki/Bloom_filter


Building
--------
Building is done using CMake. That means:
```bash
cd libsimplebloom 
mkdir build
cmake ..
make 
make install
```

By default it builds an optimized 32bit libsimplebloom. See Makefile comments
for other build options.

Binaries and library will be under ./build directory.

Sample Usage
------------

```c
#include "bloom.h"

struct bloom bloom;
bloom_init(&bloom, 1000000, 0.01);
bloom_add(&bloom, buffer, buflen);

if (bloom_check(&bloom, buffer, buflen)) {
  printf("It may be there!\n");
}
```


Documentation
-------------
Read bloom.h for more detailed documentation on the public interfaces.


License
-------
This code (except MurmurHash2) is under BSD license. MurmurHash2 is
public domain. See LICENSE file.
This project is based on libbloom by Jyri J. Virkki. This version adds
different features: more items (int vs unsigned long), saving and 
loading of bloom filter structure, CMake etc.

See murmur2/README for info on MurmurHash2 (Public Domain).
