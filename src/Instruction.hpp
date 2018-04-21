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
    Call,
    CloseUpvalue,
    ConditionalJump,
    CreateClass,
    CreateClosure,
    CreateMethod,
    CreateSubclass,
    DefineGlobal,
    Divide,
    Equal,
    False,
    GetGlobal,
    GetLocal,
    GetProperty,
    GetUpvalue,
    Greater,
    GreaterEqual,
    Jump,
    Less,
    LessEqual,
    LoadConstant,
    Multiply,
    Negate,
    Nil,
    Not,
    NotEqual,
    Pop,
    Print,
    Push,
    Return,
    SetBase,
    SetGlobal,
    SetLocal,
    SetProperty,
    SetUpvalue,
    Subtract,
    True
  };


  template <typename Stream>
  Stream& operator<<(Stream& stream, const Instruction instruction)
  {
    switch (instruction) {
    case Instruction::Add:
      stream << "ADD";
      break;
    case Instruction::Call:
      stream << "CALL";
      break;
    case Instruction::CloseUpvalue:
      stream << "CLOSE_UPVALUE";
      break;
    case Instruction::ConditionalJump:
      stream << "CONDITIONAL_JUMP";
      break;
    case Instruction::CreateClass:
      stream << "CREATE_CLASS";
      break;
    case Instruction::CreateClosure:
      stream << "CREATE_CLOSURE";
      break;
    case Instruction::CreateMethod:
      stream << "CREATE_METHOD";
      break;
    case Instruction::CreateSubclass:
      stream << "CREATE_SUBCLASS";
      break;
    case Instruction::DefineGlobal:
      stream << "DEFINE_GLOBAL";
      break;
    case Instruction::Divide:
      stream << "DIVIDE";
      break;
    case Instruction::Equal:
      stream << "EQUAL";
      break;
    case Instruction::False:
      stream << "FALSE";
      break;
    case Instruction::GetGlobal:
      stream << "GET_GLOBAL";
      break;
    case Instruction::GetLocal:
      stream << "GET_LOCAL";
      break;
    case Instruction::GetProperty:
      stream << "GET_PROPERTY";
      break;
    case Instruction::GetUpvalue:
      stream << "GET_UPVALUE";
      break;
    case Instruction::Greater:
      stream << "GREATER";
      break;
    case Instruction::GreaterEqual:
      stream << "GREATER_EQUAL";
      break;
    case Instruction::Jump:
      stream << "JUMP";
      break;
    case Instruction::Less:
      stream << "LESS";
      break;
    case Instruction::LessEqual:
      stream << "LESS_EQUAL";
      break;
    case Instruction::LoadConstant:
      stream << "LOAD_CONST";
      break;
    case Instruction::Multiply:
      stream << "MULTIPLY";
      break;
    case Instruction::Negate:
      stream << "NEGATE";
      break;
    case Instruction::Nil:
      stream << "NIL";
      break;
    case Instruction::Not:
      stream << "NOT";
      break;
    case Instruction::NotEqual:
      stream << "NOT_EQUAL";
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
    case Instruction::SetBase:
      stream << "SET_BASE";
      break;
    case Instruction::SetGlobal:
      stream << "SET_GLOBAL";
      break;
    case Instruction::SetLocal:
      stream << "SET_LOCAL";
      break;
    case Instruction::SetProperty:
      stream << "SET_PROPERTY";
      break;
    case Instruction::SetUpvalue:
      stream << "SET_UPVALUE";
      break;
    case Instruction::Subtract:
      stream << "SUBTRACT";
      break;
    case Instruction::True:
      stream << "TRUE";
      break;
    }

    return stream;
  }
}

#endif // LOXX_INSTRUCTIONS_HPP
