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
  Optional.hpp
  Parser.hpp
  RuntimeError.hpp
  Scanner.hpp
  Scope.hpp
  Stack.hpp
  StackFrame.hpp
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
  Scope.cpp
  StackFrame.cpp
  Token.cpp
  VirtualMachine.cpp)

add_executable(loxx ${SRC})
target_link_libraries(loxx)
