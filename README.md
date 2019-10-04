# utf8streams

The *utf8streams* C++ library provides a small extension to ```std::istream```
to read data encoded in UTF-16 or UTF-32 as if it was encoded in UTF-8.

Features:

* Compatible with all streams derived from ```std::istream```
* Support for the following input encodings:

  * UTF-8
  * UTF-16 Little Endian
  * UTF-16 Big Endian
  * UTF-32 Little Endian
  * UTF-32 Big Endian

* Detection of Byte Order Marks (BOM)
* No dynamic memory allocation

Tested on:

* Linux g++ 8.3.0
* macOS Clang 10.0.1
* Windows Visual Studio 2019

## Requirements

* CMake (version 3.10 or later)
* A C++11 compatible compiler such as g++ or clang
* Googletest (only for tests)

## How to build?

On Unix systems:

```
mkdir build
cd build

cmake ..      # -DUTF8STREAMS_BUILD_TESTS=OFF to disable tests
make
```

On Windows:

In the Visual Studio command line:

```
mkdir build
cd build

cmake -G "NMake Makefiles" ..      # -DUTF8STREAMS_BUILD_TESTS=OFF to disable tests
nmake
```

In case Googletest is not found download the
[source code](https://github.com/google/googletest), build it and rerun cmake:

```
cmake -G "NMake Makefiles" -DGTEST_LIBRARY=googletest-release-1.8.1\googletest\build\gtestd.lib -DGTEST_MAIN_LIBRARY=googletest-release-1.8.1\googletest\build\gtest_maind.lib -DGTEST_INCLUDE_DIR=googletest-release-1.8.1\googletest\include ..
```

Please also note possible incompatibilities between /MDd and /MTd. For more
information see
[this thread](https://stackoverflow.com/questions/12540970/how-to-make-gtest-build-mdd-instead-of-mtd-by-default-using-cmake).

If tests are enabled, they can be started by executing ```ctest``` in the same folder.

## Usage

The usage of the library is demonstrated in the *Example* and *Tests* folders.
