/*
 * This file is part of loxx.
 *
 * loxx is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * loxx is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Created by Matt Spraggs on 05/03/2018.
 */

#ifndef LOXX_VIRTUALMACHINE_HPP
#define LOXX_VIRTUALMACHINE_HPP

#include <functional>
#include <list>
#include <unordered_map>
#include <vector>

#include "CodeObject.hpp"
#include "jit/CodeProfiler.hpp"
#include "globals.hpp"
#include "HashTable.hpp"
#include "Instruction.hpp"
#include "Object.hpp"
#include "RuntimeError.hpp"
#include "Stack.hpp"
#include "StackFrame.hpp"
#include "utils.hpp"
#include "Value.hpp"


namespace loxx
{
  struct RuntimeContext
  {
    const Stack<Value, max_stack_size>& stack;
    const CodeObject& code;
    const ClosureObject& func;
    StringHashTable<Value>& globals;
  };


  class VirtualMachine
  {
  public:
    VirtualMachine(jit::CodeProfiler& profiler, const bool debug);

    void execute(std::unique_ptr<CodeObject> code_object);

  private:
    void execute_call();
    void call_object(const InstrArgUByte num_args, ObjectPtr const obj);
    void execute_create_closure();

    UpvalueObject* capture_upvalue(Value& local);
    void close_upvalues(Value& last);

    void call(ClosureObject* closure, const std::size_t num_args);

    template <typename T>
    T read_integer();
    Value read_constant();
    loxx::StringObject* read_string();
    void check_number_operands(const Value& first,
                               const Value& second) const;
    bool are_equal(const Value& first, const Value& second) const;
    bool is_truthy(const Value& value) const;
    void incorrect_arg_num(const InstrArgUByte arity,
                           const InstrArgUByte num_args) const;
    RuntimeError make_runtime_error(const std::string& msg) const;

    bool debug_;
    CodeObject::InsPtr ip_;
    const CodeObject* code_object_;
    StringHashTable<Value> globals_;
    Stack<Value, max_stack_size> stack_;
    Stack<StackFrame, max_call_frames> call_stack_;
    std::list<UpvalueObject*> open_upvalues_;
    StringObject* init_lexeme_;
    jit::CodeProfiler* profiler_;
  };


  template <typename T>
  T VirtualMachine::read_integer()
  {
    const T integer = read_integer_at_pos<T>(ip_);
    ip_ += sizeof(T);
    return integer;
  }
}

#endif // LOXX_VIRTUALMACHINE_HPP
