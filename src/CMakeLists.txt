cmake_minimum_required(VERSION 3.0)
project(loxx)

set(SRC
  AstPrinter.hpp
  Callable.hpp
  Environment.hpp
  Expr.hpp
  FuncCallable.hpp
  Generic.hpp
  Interpreter.hpp
  logging.hpp
  NativeCallable.hpp
  Parser.hpp
  Resolver.hpp
  RuntimeError.hpp
  Scanner.hpp
  Stack.hpp
  Stmt.hpp
  Token.hpp

  AstPrinter.cpp
  Environment.cpp
  FuncCallable.cpp
  Generic.cpp
  Interpreter.cpp
  logging.cpp
  main.cpp
  Parser.cpp
  Resolver.cpp
  Scanner.cpp
  Token.cpp)

add_executable(loxx ${SRC})
target_link_libraries(loxx)