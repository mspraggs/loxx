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

#include <iostream>

#include "logging.hpp"


namespace loxx
{
  void error(const unsigned int line, const std::string& message)
  {
    report(line, "", message);
  }

  void report(const unsigned int line, const std::string& where,
              const std::string& message)
  {
    std::cout << "[line " << line << "] Error " << where << ": "
              << message << std::endl;
    had_error = true;
  }


  void error(const Token& token, const std::string& message)
  {
    if (token.type() == TokenType::Eof) {
      report(token.line(), " at end", message);
    }
    else {
      report(token.line(), " at '" + token.lexeme() + "'", message);
    }
  }


  void runtime_error(const Interpreter::RuntimeError& error)
  {
    std::cout << error.what() << "\n[line " << error.token().line() << ']'
              << std::endl;

    had_runtime_error = true;
  }
}
