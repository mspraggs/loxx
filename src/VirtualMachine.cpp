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
      : instruction_ptr_(0)
  {
    // TODO: Stack and frame space size reservation
  }


  void VirtualMachine::execute(const std::vector<std::uint8_t>& byte_code,
                               const std::vector<Obj>& constants)
  {
    instruction_ptr_ = 0;

    while (true) {

      const auto instruction =
          static_cast<Instruction>(byte_code[instruction_ptr_]);

      if (instruction == Instruction::Add) {
        const auto first = stack_.pop();
        const auto second = stack_.pop();

        if (not holds_alternative<double>(first) or
            not holds_alternative<double>(second)) {
          return;
        }

        stack_.push(get<double>(first) + get<double>(second));
      }

      else if (instruction == Instruction::LoadConstant) {
        std::size_t value_index = 0;

        for (std::size_t i = 0; i < sizeof(std::size_t); ++i) {
          value_index |= (byte_code[++instruction_ptr_] << 8 * i);
        }

        stack_.push(constants[value_index]);
      }

      else if (instruction == Instruction::Print) {
        print_object(stack_.pop());
      }

      ++instruction_ptr_;

      if (instruction_ptr_ >= byte_code.size()) {
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
}
