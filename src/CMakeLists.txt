cmake_minimum_required(VERSION 3.0)
project(loxx)

set(SRC
  AstPrinter.hpp
  Compiler.hpp
  Environment.hpp
  Expr.hpp
  globals.hpp
  Instruction.hpp
  logging.hpp
  Parser.hpp
  RuntimeError.hpp
  Scanner.hpp
  Stack.hpp
  Stmt.hpp
  Token.hpp
  Variant.hpp
  VirtualMachine.hpp

  AstPrinter.cpp
  Compiler.cpp
  Environment.cpp
  logging.cpp
  main.cpp
  Parser.cpp
  Scanner.cpp
  Token.cpp
  VirtualMachine.cpp)

add_executable(loxx ${SRC})
target_link_libraries(loxx)
