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
    std::cout << error.what() << "\n[line " << error.token().line() << ']'
              << std::endl;

    had_runtime_error = true;
  }


  void print_byte_code(const std::vector<std::uint8_t>& byte_code)
  {
    std::size_t ip = 0;

    while (ip < byte_code.size()) {

      const auto instruction = static_cast<Instruction>(byte_code[ip]);

      switch (instruction) {

      case Instruction::Add:
      case Instruction::Divide:
      case Instruction::Multiply:
      case Instruction::Pop:
      case Instruction::Print:
      case Instruction::Push:
      case Instruction::Return:
      case Instruction::Subtract:
        std::cout << std::setw(8) << std::right << ip << ' ' << instruction
                  << '\n';
        break;
      case Instruction::LoadConstant: {
        std::cout << std::setw(8) << std::right << ip << ' ';

        std::size_t param = 0;
        for (std::size_t i = 0; i < sizeof(std::size_t); ++i) {
          param |= (byte_code[++ip] << 8 * i);
        }

        std::cout << std::setw(20) << std::left << instruction << param << '\n';
        break;
      }
      }

      ++ip;
    }
  }
}
