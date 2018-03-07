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

#include "RuntimeError.hpp"
#include "VirtualMachine.hpp"


namespace loxx
{
  VirtualMachine::VirtualMachine()
      : ip_(0)
  {
    // TODO: Stack and frame space size reservation
  }


  void VirtualMachine::execute(const std::vector<std::uint8_t>& byte_code)
  {
    ip_ = 0;

    while (true) {

      const auto instruction =
          static_cast<Instruction>(byte_code[ip_]);

      if (instruction == Instruction::Add or
          instruction == Instruction::Subtract or
          instruction == Instruction::Multiply or
          instruction == Instruction::Divide) {
        execute_binary_op(instruction);
      }
      else if (instruction == Instruction::LoadConstant) {
        std::size_t value_index = 0;

        for (std::size_t i = 0; i < sizeof(std::size_t); ++i) {
          value_index |= (byte_code[++ip_] << 8 * i);
        }

        stack_.push(constants_[value_index]);
      }

      else if (instruction == Instruction::Print) {
        print_object(stack_.pop());
      }

      ++ip_;

      if (ip_ >= byte_code.size()) {
        return;
      }
    }
  }


  void VirtualMachine::print_object(Obj variant) const
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
      const Obj& first, const Obj& second) const
  {
    if (not holds_alternative<double>(first) and
        not holds_alternative<double>(second)) {
      // TODO: Error handling
    }
  }
}
