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

#include <list>
#include <unordered_map>
#include <vector>

#include "globals.hpp"
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
    explicit VirtualMachine(const CodeObject& compiler_output,
                            const bool debug);

    void execute();

    UByteCodeArg add_named_constant(const std::string& lexeme,
                                    const Value& value);
    UByteCodeArg add_string_constant(const std::string& str);
    UByteCodeArg add_constant(const Value& value);
    const Value& get_constant(const UByteCodeArg index) const
    { return constants_[index]; }

  private:
    void print_object(Value object) const;
    void execute_binary_op(const Instruction instruction);
    void execute_call();
    void execute_create_closure();

    raw_ptr<UpvalueObject> capture_upvalue(Value& local);
    void close_upvalues(Value& last);

    void call_object(ObjectPtr obj, const std::size_t obj_pos,
                     const UByteCodeArg num_args);
    void print_stack() const;
    static ObjectPtr select_object(
        const Value& value, const std::vector<ObjectType>& valid_types);

    template <typename T>
    T read_integer();
    std::string read_string();
    void check_number_operands(const Value& first,
                               const Value& second) const;
    bool are_equal(const Value& first, const Value& second) const;
    bool is_truthy(const Value& value) const;
    RuntimeError runtime_error(const std::string& msg) const;

    bool debug_;
    raw_ptr<const std::uint8_t> ip_;
    raw_ptr<const CodeObject> compiler_output_;
    std::unordered_map<std::string, UByteCodeArg> constant_map_;
    std::vector<Value> constants_;
    std::unordered_map<std::string, Value> globals_;
    Stack<Value> stack_;
    Stack<StackFrame> call_stack_;
    std::list<raw_ptr<UpvalueObject>> open_upvalues_;
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
