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

#include "detail/common.hpp"
#include "CodeObject.hpp"
#include "Instruction.hpp"
#include "logging.hpp"
#include "Object.hpp"
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
    if (token.type() == TokenType::Eof) {
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
    const std::uint8_t* ip = output.bytecode.data();
    while (ip <= &output.bytecode.back()) {
      ip = print_instruction(output, ip);
    }
  }


  const std::uint8_t* print_instruction(const CodeObject& output,
                                        const std::uint8_t* ip)
  {
    const auto& bytecode = output.bytecode;
    const auto& constants = output.constants;
    const auto instruction = static_cast<Instruction>(*ip);

    const auto pos = static_cast<std::size_t>(ip - bytecode.data());
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

    const std::uint8_t* ret = ip + 1;

    switch (instruction) {

    case Instruction::Add:
    case Instruction::CloseUpvalue:
    case Instruction::Divide:
    case Instruction::Equal:
    case Instruction::False:
    case Instruction::Greater:
    case Instruction::Less:
    case Instruction::Multiply:
    case Instruction::Negate:
    case Instruction::Nil:
    case Instruction::Not:
    case Instruction::Pop:
    case Instruction::Print:
    case Instruction::Push:
    case Instruction::Return:
    case Instruction::Subtract:
    case Instruction::True:
      break;

    case Instruction::ConditionalJump:
    case Instruction::Jump: {
      const auto param = read_integer_at_pos<InstrArgSByte>(ret);
      std::cout << pos << " -> " << pos + param + sizeof(InstrArgSByte) + 1;
      ret += sizeof(InstrArgSByte);
      break;
    }

    case Instruction::CreateClosure: {
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

    case Instruction::Call: {
      const auto num_args = read_integer_at_pos<InstrArgUByte>(ret);
      ret += sizeof(InstrArgUByte);
      std::cout << num_args;
      break;
    }

    case Instruction::CreateClass:
    case Instruction::CreateMethod:
    case Instruction::CreateSubclass:
    case Instruction::DefineGlobal:
    case Instruction::GetGlobal:
    case Instruction::GetProperty:
    case Instruction::GetSuperFunc:
    case Instruction::SetGlobal:
    case Instruction::SetProperty:
    case Instruction::LoadConstant: {
      const auto param = read_integer_at_pos<InstrArgUByte>(ret);
      std::cout << static_cast<unsigned int>(param)
                << " '" << constants[param] << "'";
      ret += sizeof(InstrArgUByte);
      break;
    }

    case Instruction::GetLocal:
    case Instruction::GetUpvalue:
    case Instruction::SetLocal:
    case Instruction::SetUpvalue: {
      const auto param = read_integer_at_pos<InstrArgUByte>(ret);
      std::cout << static_cast<unsigned int>(param);
      ret += sizeof(InstrArgUByte);
      break;
    }

    case Instruction::Invoke:
      const auto param = read_integer_at_pos<InstrArgUByte>(ret);
      ret += sizeof(InstrArgUByte);
      const auto num_args = read_integer_at_pos<InstrArgUByte>(ret);
      ret += sizeof(InstrArgUByte);
      std::cout << num_args << ", " << param << " '" << constants[param] << "'";
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
