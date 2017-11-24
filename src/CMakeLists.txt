cmake_minimum_required(VERSION 3.0)
project(loxx)

set(SRC
  AstPrinter.hpp
  Callable.hpp
  Environment.hpp
  Expr.hpp
  Generic.hpp
  Interpreter.hpp
  logging.hpp
  NativeCallable.hpp
  Parser.hpp
  RuntimeError.hpp
  Scanner.hpp
  Stack.hpp
  Stmt.hpp
  Token.hpp

  AstPrinter.cpp
  Environment.cpp
  Generic.cpp
  Interpreter.cpp
  logging.cpp
  main.cpp
  Parser.cpp
  Scanner.cpp
  Token.cpp)

add_executable(loxx ${SRC})
target_link_libraries(loxx)