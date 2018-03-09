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

#include <iostream>

#include "Compiler.hpp"
#include "logging.hpp"
#include "RuntimeError.hpp"
#include "VirtualMachine.hpp"


namespace loxx
{
  VirtualMachine::VirtualMachine() : ip_(0)
  {
    // TODO: Stack and frame space size reservation
  }


  void VirtualMachine::execute(const CompilationOutput& compiler_output)
  {
    compiler_output_ = &compiler_output;
    ip_ = 0;

    while (true) {

      const auto instruction =
          static_cast<Instruction>(compiler_output.bytecode[ip_]);

      if (instruction == Instruction::Add or
          instruction == Instruction::Subtract or
          instruction == Instruction::Multiply or
          instruction == Instruction::Divide) {
        execute_binary_op(instruction);
      }
      else if (instruction == Instruction::LoadConstant) {
        stack_.push(constants_[read_integer<std::size_t>()]);
      }

      else if (instruction == Instruction::Print) {
        print_object(stack_.pop());
      }

      ++ip_;

      if (ip_ >= compiler_output.bytecode.size()) {
        return;
      }
    }
  }


  void VirtualMachine::print_object(StackVar variant) const
  {
    if (holds_alternative<double>(variant)) {
      std::cout << get<double>(variant) << std::endl;
    }
  }


  void VirtualMachine::execute_binary_op(const Instruction instruction)
  {
    const auto second = stack_.pop();
    const auto first = stack_.pop();

    switch (instruction) {
    case Instruction::Multiply:
      check_number_operands(first, second);
      stack_.push(get<double>(first) * get<double>(second));
      break;
    case Instruction::Divide:
      check_number_operands(first, second);
      stack_.push(get<double>(first) / get<double>(second));
      break;
    case Instruction::Subtract:
      check_number_operands(first, second);
      stack_.push(get<double>(first) - get<double>(second));
      break;
    case Instruction::Add:
      check_number_operands(first, second);
      stack_.push(get<double>(first) + get<double>(second));
      break;
    default:
      break;
    }
  }


  void VirtualMachine::check_number_operands(
      const StackVar& first, const StackVar& second) const
  {
    if (not holds_alternative<double>(first) or
        not holds_alternative<double>(second)) {
      throw_runtime_error("Binary operands must both be numbers.");
    }
  }


  void VirtualMachine::throw_runtime_error(const std::string& message) const
  {
    std::size_t instruction_counter = 0;
    unsigned int line = 0;

    for (const auto& table_row : compiler_output_->line_num_table) {
      instruction_counter += std::get<1>(table_row);
      line += std::get<0>(table_row);

      if (instruction_counter > ip_) {
        break;
      }
    }

    throw RuntimeError(line, message);
  }
}
