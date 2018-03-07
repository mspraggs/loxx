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

#ifndef LOXX_INSTRUCTIONS_HPP
#define LOXX_INSTRUCTIONS_HPP

#include <cstdint>


namespace loxx
{
  enum class Instruction : std::uint8_t
  {
    Add,
    Divide,
    LoadConstant,
    Multiply,
    Pop,
    Print,
    Push,
    Return,
    Subtract,
  };


  template <typename Stream>
  Stream& operator<<(Stream& stream, const Instruction instruction)
  {
    switch (instruction) {
    case Instruction::Add:
      stream << "ADD";
      break;
    case Instruction::Divide:
      stream << "DIVIDE";
      break;
    case Instruction::LoadConstant:
      stream << "LOAD_CONST";
      break;
    case Instruction::Multiply:
      stream << "MULTIPLY";
      break;
    case Instruction::Pop:
      stream << "POP";
      break;
    case Instruction::Print:
      stream << "PRINT";
      break;
    case Instruction::Push:
      stream << "PUSH";
      break;
    case Instruction::Return:
      stream << "RETURN";
      break;
    case Instruction::Subtract:
      stream << "SUBTRACT";
      break;
    }

    return stream;
  }
}

#endif // LOXX_INSTRUCTIONS_HPP
