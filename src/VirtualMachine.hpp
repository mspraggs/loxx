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

#include <unordered_map>
#include <vector>

#include "globals.hpp"
#include "Instruction.hpp"
#include "Variant.hpp"
#include "VirtualMachine.hpp"
#include "Stack.hpp"
#include "StackFrame.hpp"


namespace loxx
{
  class VirtualMachine
  {
  public:
    explicit VirtualMachine(const bool debug);

    void execute(const CompilationOutput& compiler_output);

    ByteCodeArg make_constant(const std::string& lexeme, const StackVar& value);

    ByteCodeArg get_constant(const std::string& lexeme) const;

  private:
    void print_object(StackVar object) const;
    void execute_binary_op(const Instruction instruction);

    std::string stringify(const StackVar& object) const;
    void print_stack() const;
    void disassemble_instruction() const;
    template <typename T>
    T read_integer();
    template <typename T>
    T read_integer_at_pos(const std::size_t pos) const;
    void check_number_operands(const StackVar& first,
                               const StackVar& second) const;
    bool are_equal(const StackVar& first, const StackVar& second) const;
    bool is_truthy(const StackVar& value) const;
    unsigned int get_current_line() const;

    bool debug_;
    std::size_t ip_;
    const CompilationOutput* compiler_output_;
    std::unordered_map<std::string, ByteCodeArg> constant_map_;
    std::vector<StackVar> constants_;
    std::unordered_map<std::string, StackVar> globals_;
    Stack<StackVar> stack_;
    Stack<StackFrame> call_stack_;
  };


  template <typename T>
  T VirtualMachine::read_integer()
  {
    T integer = 0;

    for (std::size_t i = 0; i < sizeof(T); ++i) {
      integer |= (compiler_output_->bytecode[++ip_] << 8 * i);
    }

    return integer;
  }


  template <typename T>
  T VirtualMachine::read_integer_at_pos(const std::size_t pos) const
  {
    T integer = 0;

    for (std::size_t i = 0; i < sizeof(T); ++i) {
      integer |= (compiler_output_->bytecode[pos + i] << 8 * i);
    }

    return integer;
  }
}

#endif // LOXX_VIRTUALMACHINE_HPP
