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

#include "Instruction.hpp"
#include "logging.hpp"


namespace loxx
{
  bool had_error = false;
  bool had_runtime_error = false;


  template <typename T>
  T read_integer(const std::vector<std::uint8_t>& byte_code,
                 std::size_t& pos)
  {
    T param = 0;
    for (std::size_t i = 0; i < sizeof(T); ++i) {
      param |= (byte_code[++pos] << 8 * i);
    }

    return param;
  }


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


  void print_instruction_pointer(const std::size_t ip)
  {
    std::cout << std::setw(8) << std::right << ip << ' ';
  }


  void print_instruction(const Instruction instruction)
  {
    std::cout << std::setw(20) << std::left << instruction;
  }


  void print_byte_code(const std::vector<std::uint8_t>& byte_code)
  {
    std::size_t ip = 0;

    while (ip < byte_code.size()) {

      const auto instruction = static_cast<Instruction>(byte_code[ip]);

      switch (instruction) {

      case Instruction::Add:
      case Instruction::Divide:
      case Instruction::Equal:
      case Instruction::Greater:
      case Instruction::GreaterEqual:
      case Instruction::Less:
      case Instruction::LessEqual:
      case Instruction::Multiply:
      case Instruction::NotEqual:
      case Instruction::Pop:
      case Instruction::Print:
      case Instruction::Push:
      case Instruction::Return:
      case Instruction::Subtract:
        print_instruction_pointer(ip);
        print_instruction(instruction);
        std::cout << '\n';
        break;

      case Instruction::Call: {
        print_instruction_pointer(ip);
        print_instruction(instruction);

        const auto return_ip = read_integer<ByteCodeArg>(byte_code, ip);
        const auto num_locals = read_integer<ByteCodeArg>(byte_code, ip);
        const auto num_args = read_integer<ByteCodeArg>(byte_code, ip);

        std::cout << return_ip << ", " << num_locals << ", "
                  << num_args << '\n';
        break;
      }

      case Instruction::DefineGlobal:
      case Instruction::GetGlobal:
      case Instruction::SetGlobal:
      case Instruction::LoadConstant: {
        print_instruction_pointer(ip);
        print_instruction(instruction);

        const auto param = read_integer<ByteCodeArg>(byte_code, ip);
        std::cout << param << '\n';
        break;
      }
      case Instruction::Nil:
        print_instruction_pointer(ip);
        print_instruction(instruction);
        std::cout << '\n';
      }

      ++ip;
    }
  }
}
