cmake_minimum_required(VERSION 3.0)
project(loxx)

set(SRC
  AstPrinter.hpp
  AsyncWorker.hpp
  CodeObject.hpp
  Compiler.hpp
  Expr.hpp
  FunctionScope.hpp
  globals.hpp
  HashSet.hpp
  HashTable.hpp
  Instruction.hpp
  logging.hpp
  Object.hpp
  ObjectTracker.hpp
  Optional.hpp
  Parser.hpp
  RuntimeError.hpp
  Scanner.hpp
  Stack.hpp
  StackFrame.hpp
  Stmt.hpp
  StringHashTable.hpp
  Token.hpp
  utils.hpp
  Value.hpp
  Variant.hpp
  VirtualMachine.hpp
  detail/common.hpp
  detail/HashImpl.hpp
  detail/HashStructIterator.hpp
  detail/VariantImpl.hpp
  jit/CodeProfiler.hpp
  jit/Compiler.hpp
  jit/JITError.hpp
  jit/logging.hpp
  jit/RegisterAllocator.hpp
  jit/RegisterX86.hpp
  jit/SSAGenerator.hpp
  jit/SSAInstruction.hpp

  AstPrinter.cpp
  Compiler.cpp
  FunctionScope.cpp
  logging.cpp
  main.cpp
  Object.cpp
  ObjectTracker.cpp
  Parser.cpp
  Scanner.cpp
  StackFrame.cpp
  StringHashTable.cpp
  Token.cpp
  VirtualMachine.cpp
  jit/CodeProfiler.cpp
  jit/Compiler.cpp
  jit/logging.cpp
  jit/RegisterAllocator.cpp
  jit/RegisterX86.cpp
  jit/SSAGenerator.cpp
  jit/SSAInstruction.cpp
)

add_executable(loxx ${SRC})
target_link_libraries(loxx pthread)
