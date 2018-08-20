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

#include "globals.hpp"
#include "HashTable.hpp"
#include "Instruction.hpp"
#include "Variant.hpp"
#include "VirtualMachine.hpp"
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

    UByteCodeArg add_named_constant(const std::string& lexeme,
                                    const Value& value);
    UByteCodeArg add_string_constant(const std::string& str);
    UByteCodeArg add_constant(const Value& value);
    const Value& get_constant(const UByteCodeArg index) const
    { return constants_[index]; }

  private:
    void print_object(Value object) const;
    void execute_call();
    void execute_create_closure();

    UpvalueObject* capture_upvalue(Value& local);
    void close_upvalues(Value& last);

    void call_object(ObjectPtr obj, const std::size_t obj_pos,
                     const UByteCodeArg num_args);
    void print_stack() const;
    static ObjectPtr select_object(const ObjectPtr) { return {}; }
    template <typename... Ts>
    static ObjectPtr select_object(
        const ObjectPtr value, const ObjectType type0, const Ts... types);

    template <typename T>
    T read_integer();
    raw_ptr <loxx::StringObject> read_string();
    void check_number_operands(const Value& first,
                               const Value& second) const;
    bool are_equal(const Value& first, const Value& second) const;
    bool is_truthy(const Value& value) const;
    RuntimeError make_runtime_error(const std::string& msg) const;

    bool debug_;
    std::size_t ip_;
    const CodeObject* code_object_;
    ConstStringHashTable<UByteCodeArg> constant_map_;
    std::vector<Value> constants_;
    StringHashTable<Value> globals_;
    Stack<Value> stack_;
    Stack<StackFrame, 256> call_stack_;
    std::list<UpvalueObject*> open_upvalues_;
    StringObject* init_lexeme_;
  };


  template <typename... Ts>
  ObjectPtr VirtualMachine::select_object(
      const ObjectPtr obj_ptr, const ObjectType type0, const Ts... types)
  {
    if (obj_ptr->type() == type0) {
      return obj_ptr;
    }

    return select_object(obj_ptr, types...);
  }


  template <typename T>
  T VirtualMachine::read_integer()
  {
    const T integer = read_integer_at_pos<T>(code_object_->bytecode, ip_);
    ip_ += sizeof(T);

    return integer;
  }
}

#endif // LOXX_VIRTUALMACHINE_HPP
