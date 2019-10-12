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
 * Created by Matt Spraggs on 31/10/17.
 */

#include <iomanip>
#include <iostream>
#include <sstream>

#include "CodeObject.hpp"
#include "Instruction.hpp"
#include "logging.hpp"
#include "Object.hpp"
#include "utils.hpp"
#include "VirtualMachine.hpp"


namespace loxx
{
  bool had_error = false;
  bool had_runtime_error = false;


  void error(const unsigned int line, const std::string& message)
  {
    report(line, "", message);
  }

  void report(const unsigned int line, const std::string& where,
              const std::string& message)
  {
    const auto location_padding = where.length() > 0 ? " " : "";
    std::cout << "[line " << line << "] Error"
              << location_padding << where << ": "
              << message << std::endl;
    had_error = true;
  }


  void error(const Token& token, const std::string& message)
  {
    if (token.type() == TokenType::END_OF_FILE) {
      report(token.line(), "at end", message);
    }
    else {
      report(token.line(), "at '" + token.lexeme() + "'", message);
    }
  }


  void runtime_error(const RuntimeError& error)
  {
    std::cout << error.what() << "\n[line " << error.line() << ']'
              << std::endl;

    had_runtime_error = true;
  }


  void print_bytecode(const std::string& name, const CodeObject& output)
  {
    std::cout << "=== " << name << " ===\n";
    auto ip = output.bytecode.begin();
    while (ip != output.bytecode.end()) {
      ip = print_instruction(output, ip);
    }
  }


  CodeObject::InsPtr print_instruction(const CodeObject& output,
                                       const CodeObject::InsPtr ip)
  {
    const auto& bytecode = output.bytecode;
    const auto& constants = output.constants;
    const auto instruction = static_cast<Instruction>(*ip);

    const auto pos = std::distance(bytecode.begin(), ip);
    static unsigned int last_line_num = 0;
    const unsigned int current_line_num = get_current_line(output, pos);

    std::stringstream line_num_ss;
    line_num_ss << std::right << std::setw(5) << std::setfill(' ');
    if (last_line_num < current_line_num) {
      line_num_ss << current_line_num;
    }
    else {
      line_num_ss << '|';
    }
    last_line_num = current_line_num;

    std::cout << std::setw(4) << std::setfill('0') << std::right << pos;
    std::cout << line_num_ss.str() << ' ';
    std::cout << std::setw(20) << std::setfill(' ') << std::left << instruction;

    auto ret = ip + 1;

    switch (instruction) {

    case Instruction::ADD:
    case Instruction::CLOSE_UPVALUE:
    case Instruction::DIVIDE:
    case Instruction::EQUAL:
    case Instruction::FALSE:
    case Instruction::GREATER:
    case Instruction::LESS:
    case Instruction::MULTIPLY:
    case Instruction::NEGATE:
    case Instruction::NIL:
    case Instruction::NOT:
    case Instruction::POP:
    case Instruction::PRINT:
    case Instruction::PUSH:
    case Instruction::RETURN:
    case Instruction::SUBTRACT:
    case Instruction::TRUE:
      break;

    case Instruction::CONDITIONAL_JUMP:
    case Instruction::JUMP: {
      const auto param = read_integer_at_pos<InstrArgUShort>(ret);
      std::cout << pos << " -> " << pos + param + sizeof(InstrArgUShort) + 1;
      ret += sizeof(InstrArgUShort);
      break;
    }

    case Instruction::CREATE_CLOSURE: {
      const auto& func_value =
          constants[read_integer_at_pos<InstrArgUByte>(ret)];
      const auto& func_obj = get<ObjectPtr>(func_value);
      auto func = static_cast<FuncObject*>(func_obj);

      ret += sizeof(InstrArgUByte);

      std::cout << func->lexeme() << ' ';

      for (unsigned int i = 0; i < func->num_upvalues(); ++i) {
        const auto is_local = read_integer_at_pos<InstrArgUByte>(ret) != 0;
        ret += sizeof(InstrArgUByte);
        const auto index = read_integer_at_pos<InstrArgUByte>(ret);
        ret += sizeof(InstrArgUByte);

        std::cout << '(' << (is_local ? "local" : "upvalue") << ", "
                  << static_cast<unsigned int>(index) << ')';

        if (i < func->num_upvalues() - 1) {
          std::cout << ", ";
        }
      }

      break;
    }

    case Instruction::CALL: {
      const auto num_args = read_integer_at_pos<InstrArgUByte>(ret);
      ret += sizeof(InstrArgUByte);
      std::cout << static_cast<int>(num_args);
      break;
    }

    case Instruction::CREATE_CLASS:
    case Instruction::CREATE_METHOD:
    case Instruction::CREATE_SUBCLASS:
    case Instruction::DEFINE_GLOBAL:
    case Instruction::GET_GLOBAL:
    case Instruction::GET_PROPERTY:
    case Instruction::GET_SUPER_FUNC:
    case Instruction::SET_GLOBAL:
    case Instruction::SET_PROPERTY:
    case Instruction::LOAD_CONSTANT: {
      const auto param = read_integer_at_pos<InstrArgUByte>(ret);
      std::cout << static_cast<unsigned int>(param)
                << " '" << constants[param] << "'";
      ret += sizeof(InstrArgUByte);
      break;
    }

    case Instruction::GET_LOCAL:
    case Instruction::GET_UPVALUE:
    case Instruction::SET_LOCAL:
    case Instruction::SET_UPVALUE: {
      const auto param = read_integer_at_pos<InstrArgUByte>(ret);
      std::cout << static_cast<unsigned int>(param);
      ret += sizeof(InstrArgUByte);
      break;
    }

    case Instruction::INVOKE: {
      const auto param = read_integer_at_pos<InstrArgUByte>(ret);
      ret += sizeof(InstrArgUByte);
      const auto num_args = read_integer_at_pos<InstrArgUByte>(ret);
      ret += sizeof(InstrArgUByte);
      std::cout << num_args << ", " << param << " '" << constants[param] << "'";
      break;
    }

    case Instruction::LOOP: {
      const auto param = read_integer_at_pos<InstrArgUShort>(ret);
      ret += sizeof(InstrArgUShort);
      std::cout << pos << " -> " << pos - param + sizeof(InstrArgUShort) + 1;
      break;
    }
    }

    std::cout << '\n';

    return ret;
  }


  unsigned int get_current_line(const CodeObject& output,
                                const std::size_t pos)
  {
    std::size_t instruction_counter = 0;
    unsigned int line = 0;

    for (const auto& table_row : output.line_num_table) {
      instruction_counter += std::get<1>(table_row);
      line += std::get<0>(table_row);

      if (instruction_counter >= pos) {
        break;
      }
    }

    return line;
  }
}
