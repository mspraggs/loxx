cmake_minimum_required(VERSION 3.0)
project(loxx)

set(SRC
  AstPrinter.hpp
  Expr.hpp
  Generic.hpp
  Interpreter.hpp
  logging.hpp
  Parser.hpp
  Scanner.hpp
  Token.hpp

  AstPrinter.cpp
  Interpreter.cpp
  logging.cpp
  main.cpp
  Parser.cpp
  Scanner.cpp
  Token.cpp)

add_executable(loxx ${SRC})
target_link_libraries(loxx)