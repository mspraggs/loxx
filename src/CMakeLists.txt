cmake_minimum_required(VERSION 3.0)
project(loxx)

set(SRC
  logging.hpp
  Scanner.hpp
  Token.hpp

  logging.cpp
  main.cpp
  Scanner.cpp Token.cpp)

add_executable(loxx ${SRC})
target_link_libraries(loxx)