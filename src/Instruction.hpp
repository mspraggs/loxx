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
    ADD,
    CALL,
    CLOSE_UPVALUE,
    CONDITIONAL_JUMP,
    CREATE_CLASS,
    CREATE_CLOSURE,
    CREATE_METHOD,
    CREATE_SUBCLASS,
    DEFINE_GLOBAL,
    DIVIDE,
    EQUAL,
    FALSE,
    GET_GLOBAL,
    GET_LOCAL,
    GET_PROPERTY,
    GET_SUPER_FUNC,
    GET_UPVALUE,
    GREATER,
    INVOKE,
    JUMP,
    LESS,
    LOAD_CONSTANT,
    LOOP,
    MULTIPLY,
    NEGATE,
    NIL,
    NOT,
    POP,
    PRINT,
    PROFILE_TYPE,
    RETURN,
    SET_GLOBAL,
    SET_LOCAL,
    SET_PROPERTY,
    SET_UPVALUE,
    SUBTRACT,
    TRUE
  };


  template <typename Stream>
  Stream& operator<<(Stream& stream, const Instruction instruction)
  {
    switch (instruction) {
    case Instruction::ADD:
      stream << "ADD";
      break;
    case Instruction::CALL:
      stream << "CALL";
      break;
    case Instruction::CLOSE_UPVALUE:
      stream << "CLOSE_UPVALUE";
      break;
    case Instruction::CONDITIONAL_JUMP:
      stream << "CONDITIONAL_JUMP";
      break;
    case Instruction::CREATE_CLASS:
      stream << "CREATE_CLASS";
      break;
    case Instruction::CREATE_CLOSURE:
      stream << "CREATE_CLOSURE";
      break;
    case Instruction::CREATE_METHOD:
      stream << "CREATE_METHOD";
      break;
    case Instruction::CREATE_SUBCLASS:
      stream << "CREATE_SUBCLASS";
      break;
    case Instruction::DEFINE_GLOBAL:
      stream << "DEFINE_GLOBAL";
      break;
    case Instruction::DIVIDE:
      stream << "DIVIDE";
      break;
    case Instruction::EQUAL:
      stream << "EQUAL";
      break;
    case Instruction::FALSE:
      stream << "FALSE";
      break;
    case Instruction::GET_GLOBAL:
      stream << "GET_GLOBAL";
      break;
    case Instruction::GET_LOCAL:
      stream << "GET_LOCAL";
      break;
    case Instruction::GET_PROPERTY:
      stream << "GET_PROPERTY";
      break;
    case Instruction::GET_SUPER_FUNC:
      stream << "GET_SUPER_FUNC";
      break;
    case Instruction::GET_UPVALUE:
      stream << "GET_UPVALUE";
      break;
    case Instruction::GREATER:
      stream << "GREATER";
      break;
    case Instruction::INVOKE:
      stream << "INVOKE";
      break;
    case Instruction::JUMP:
      stream << "JUMP";
      break;
    case Instruction::LESS:
      stream << "LESS";
      break;
    case Instruction::LOAD_CONSTANT:
      stream << "LOAD_CONST";
      break;
    case Instruction::LOOP:
      stream << "LOOP";
      break;
    case Instruction::MULTIPLY:
      stream << "MULTIPLY";
      break;
    case Instruction::NEGATE:
      stream << "NEGATE";
      break;
    case Instruction::NIL:
      stream << "NIL";
      break;
    case Instruction::NOT:
      stream << "NOT";
      break;
    case Instruction::POP:
      stream << "POP";
      break;
    case Instruction::PRINT:
      stream << "PRINT";
      break;
    case Instruction::PROFILE_TYPE:
      stream << "PROFILE_TYPE";
      break;
    case Instruction::RETURN:
      stream << "RETURN";
      break;
    case Instruction::SET_GLOBAL:
      stream << "SET_GLOBAL";
      break;
    case Instruction::SET_LOCAL:
      stream << "SET_LOCAL";
      break;
    case Instruction::SET_PROPERTY:
      stream << "SET_PROPERTY";
      break;
    case Instruction::SET_UPVALUE:
      stream << "SET_UPVALUE";
      break;
    case Instruction::SUBTRACT:
      stream << "SUBTRACT";
      break;
    case Instruction::TRUE:
      stream << "TRUE";
      break;
    }

    return stream;
  }
}

#endif // LOXX_INSTRUCTIONS_HPP
