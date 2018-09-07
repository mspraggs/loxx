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
benchmarks directory 20 times on an Intel i5-4300U (1.90 GHz) with a 3 MB L3
cache:

| **File**            | **Min.** | **Mean** | **Median** | **Max.** | **Std. Dev.** |
|---------------------|---------:|---------:|-----------:|---------:|--------------:|
| binary_trees.lox    |     0.78 |     0.81 |       0.81 |     0.85 |          0.02 |
| equality.lox        |     5.24 |     5.30 |       5.27 |     5.56 |          0.08 |
| fib.lox             |     0.19 |     0.19 |       0.19 |     0.20 |          0.00 |
| invocation.lox      |     0.73 |     0.74 |       0.73 |     0.76 |          0.01 |
| method_call.lox     |     0.35 |     0.36 |       0.36 |     0.41 |          0.02 |
| properties.lox      |     0.81 |     0.82 |       0.82 |     0.85 |          0.01 |
| string_equality.lox |     2.08 |     2.11 |       2.10 |     2.16 |          0.02 |

Compared to Clox, Bob Nystrom's C implementation, this isn't so bad:

| **File**            | **Min.** | **Mean** | **Median** | **Max.** | **Std. Dev.** |
|---------------------|---------:|---------:|-----------:|---------:|--------------:|
| binary_trees.lox    |     0.58 |     0.59 |       0.59 |     0.60 |          0.01 |
| equality.lox        |     5.32 |     5.35 |       5.33 |     5.51 |          0.05 |
| fib.lox             |     0.12 |     0.12 |       0.12 |     0.13 |          0.00 |
| invocation.lox      |     0.48 |     0.49 |       0.49 |     0.49 |          0.00 |
| method_call.lox     |     0.27 |     0.27 |       0.27 |     0.28 |          0.00 |
| properties.lox      |     0.66 |     0.66 |       0.66 |     0.67 |          0.00 |

