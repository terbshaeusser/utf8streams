# utf8streams Example

This folder contains an example application illustrating how the utf8streams
library can be used.

The example programs takes a passed file name, guesses the encoding based on an
optional BOM and outputs the UTF-8 encoded data on the console.

## How to build?

On Unix systems:

```
mkdir build
cd build

cmake ..
make
```

On Windows:

In the Visual Studio command line:

```
mkdir build
cd build

cmake -G "NMake Makefiles" ..
nmake
```
