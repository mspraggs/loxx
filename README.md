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
| binary_trees.lox    |     1.58 |     1.64 |       1.63 |     1.78 |          0.05 |
| equality.lox        |    15.81 |    15.98 |      15.94 |    17.15 |          0.17 |
| fib.lox             |     0.40 |     0.43 |       0.44 |     0.51 |          0.03 |
| invocation.lox      |     2.70 |     2.85 |       2.82 |     4.71 |          0.21 |
| method_call.lox     |     1.26 |     1.32 |       1.31 |     1.44 |          0.04 |
| properties.lox      |     3.18 |     3.28 |       3.26 |     3.77 |          0.08 |
| string_equality.lox |     9.71 |     9.89 |       9.86 |    10.45 |          0.21 |

Compared to Clox, Bob Nystrom's C implementation, this isn't so bad:

| **File**            | **Min.** | **Mean** | **Median** | **Max.** | **Std. Dev.** |
|---------------------|---------:|---------:|-----------:|---------:|--------------:|
| binary_trees.lox    |     0.83 |     0.90 |       0.90 |     1.02 |          0.04 |
| equality.lox        |    10.54 |    10.69 |      10.67 |    11.18 |          0.10 |
| fib.lox             |     0.14 |     0.18 |       0.18 |     0.24 |          0.03 |
| invocation.lox      |     0.51 |     0.55 |       0.56 |     0.61 |          0.03 |
| method_call.lox     |     0.37 |     0.41 |       0.42 |     0.48 |          0.03 |
| properties.lox      |     0.86 |     0.90 |       0.90 |     0.97 |          0.03 |

