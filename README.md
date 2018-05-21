# Loxx

**Master**|[![Build Status](https://travis-ci.org/mspraggs/loxx.svg?branch=master)](https://travis-ci.org/mspraggs/loxx)
:---:|:---:
**Development**|[![Build Status](https://travis-ci.org/mspraggs/loxx.svg?branch=devel)](https://travis-ci.org/mspraggs/loxx)

Loxx is a C++14 implementation of Bob Nystrom's toy language, [Lox](https://craftinginterpreters.com).

## Compilation

Loxx uses no dependencies other than the C++14 standard library, so make sure
you have a compiler that supports that standard. To run the build, you'll need
CMake version 3 or greater. Once you've downloaded the source, in your terminal
enter the root of the Loxx source tree and run:

```
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
```

## Usage

Lox is an interpreted language. To run stuff interactively using a REPL, do

```
./loxx
```

Alternatively, execute source files like so

```
./loxx <your source filename>
```
