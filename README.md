# Loxx

**Master**|[![Build Status](https://travis-ci.org/mspraggs/loxx.svg?branch=master)](https://travis-ci.org/mspraggs/loxx)
:---:|:---:
**Development**|[![Build Status](https://travis-ci.org/mspraggs/loxx.svg?branch=devel)](https://travis-ci.org/mspraggs/loxx)

Loxx is a C++14 implementation of Bob Nystrom's toy language, [Lox](https://craftinginterpreters.com).
Lox is syntatically similar to C and uses strong, dynamic types. For example:

```
var sum = 0;

for (var i = 0; i < 10; i = i + 1) {
  sum = sum + i;
}

print sum;
```

For more details on Lox's syntax, check out the [description](http://craftinginterpreters.com/the-lox-language.html)
in Bob's book.

## Compilation

Loxx uses no dependencies other than the C++14 standard library, so make sure
you have a compiler that supports that standard. To run the build, you'll need
CMake version 3 or greater. Once you've downloaded the source, in your terminal
enter the root of the Loxx source tree and run:

```
mkdir build && cd build
cmake ..
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

## Tests

Testing takes the form of a series of functional end-to-end tests that I
effectively stole from [Bob Nystrom's implemenations of Lox](https://github.com/munificent/craftinginterpreters).
To run the tests, use the python test-runner in the tests directory:

```
python tests/run_tests.py build/loxx
```

## Benchmarks

Loxx is implemented as a bytecode virtual machine and performs reasonably well
without having to micro-optimise the code and fine-tune compiler arguments. Here
are some execution times (in seconds) from running each of the files in the
benchmarks directory 100 times on an Intel i5-2430M (2.40 GHz) with a 3 MB L3
cache:

| **File**            | **Min.** | **Mean** | **Median** | **Max.** | **Std. Dev.** |
|---------------------|---------:|---------:|-----------:|---------:|--------------:|
| binary_trees.lox    |     1.97 |     2.07 |       2.06 |     2.26 |          0.06 |
| equality.lox        |    11.43 |    11.59 |      11.54 |    12.50 |          0.18 |
| fib.lox             |     0.42 |     0.49 |       0.51 |     0.57 |          0.04 |
| invocation.lox      |     3.36 |     3.53 |       3.52 |     3.81 |          0.09 |
| method_call.lox     |     1.67 |     1.76 |       1.76 |     1.89 |          0.05 |
| properties.lox      |     5.98 |     6.20 |       6.17 |     6.89 |          0.15 |
| string_equality.lox |     6.06 |     6.18 |       6.15 |     6.61 |          0.11 |
