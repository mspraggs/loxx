cmake_minimum_required(VERSION 3.0)
project(loxx)

set(SRC
  AstPrinter.hpp
  CodeObject.hpp
  Compiler.hpp
  Expr.hpp
  FunctionScope.hpp
  globals.hpp
  HashTable.hpp
  Instruction.hpp
  logging.hpp
  ObjectTracker.hpp
  Optional.hpp
  Parser.hpp
  RuntimeError.hpp
  Scanner.hpp
  Stack.hpp
  StackFrame.hpp
  Stmt.hpp
  Token.hpp
  Value.hpp
  Variant.hpp
  VirtualMachine.hpp
  detail/HashImpl.hpp
  detail/HashStructIterator.hpp
  detail/VariantImpl.hpp

  AstPrinter.cpp
  Compiler.cpp
  FunctionScope.cpp
  logging.cpp
  main.cpp
  ObjectTracker.cpp
  Parser.cpp
  Scanner.cpp
  StackFrame.cpp
  Token.cpp
  Value.cpp
  VirtualMachine.cpp)

add_executable(loxx ${SRC})
target_link_libraries(loxx)
