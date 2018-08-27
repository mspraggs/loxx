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
#include "globals.hpp"
#include "HashTable.hpp"
#include "Instruction.hpp"
#include "Object.hpp"
#include "RuntimeError.hpp"
#include "Stack.hpp"
#include "StackFrame.hpp"
#include "Value.hpp"


namespace loxx
{
  class VirtualMachine
  {
  public:
    explicit VirtualMachine(const bool debug);

    void execute(const CodeObject& code_object);

    const Value& get_constant(const UByteCodeArg index) const
    { return code_object_->constants[index]; }

  private:
    void print_object(Value object) const;
    void execute_call();
    void execute_create_closure();

    UpvalueObject* capture_upvalue(Value& local);
    void close_upvalues(Value& last);

    void call(ClosureObject* closure, const std::size_t num_args);
    void print_stack() const;

    template <typename T>
    T read_integer();
    loxx::StringObject* read_string();
    void check_number_operands(const Value& first,
                               const Value& second) const;
    bool are_equal(const Value& first, const Value& second) const;
    bool is_truthy(const Value& value) const;
    RuntimeError make_runtime_error(const std::string& msg) const;

    bool debug_;
    std::size_t ip_;
    const CodeObject* code_object_;
    StringHashTable<Value> globals_;
    Stack<Value, max_stack_size> stack_;
    Stack<StackFrame, max_call_frames> call_stack_;
    std::list<UpvalueObject*> open_upvalues_;
    StringObject* init_lexeme_;
  };


  template <typename T>
  T VirtualMachine::read_integer()
  {
    const T integer = read_integer_at_pos<T>(code_object_->bytecode, ip_);
    ip_ += sizeof(T);

    return integer;
  }
}

#endif // LOXX_VIRTUALMACHINE_HPP
