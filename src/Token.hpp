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

#ifndef LOXX_TOKEN_HPP
#define LOXX_TOKEN_HPP

#include <ostream>
#include <string>

#include "Generic.hpp"


namespace loxx
{
  enum class TokenType {
    // Single characters
    LeftParen, RightParen, LeftBrace, RightBrace, Comma, Dot, Minus, Plus,
    SemiColon, Slash, Star,
    // One or two character tokens
    Bang, BangEqual, Equal, EqualEqual, Greater, GreaterEqual, Less, LessEqual,
    // Literals
    Identifier, String, Number,
    // Keywords
    And, Class, Else, False, Fun, For, If, Nil, Or, Print, Return,
    Super, This, True, Var, While,
    // End of file
    Eof
  };

  class Token
  {
  public:
    Token(const TokenType type, std::string lexeme, Generic literal,
          const unsigned int line)
        : type_(type), lexeme_(std::move(lexeme)), literal_(std::move(literal)),
          line_(line)
    {}

    TokenType type() const { return type_; }
    const std::string& lexeme() const { return lexeme_; }
    unsigned int line() const { return line_; }

    const Generic& literal() const { return literal_; }

  private:
    TokenType type_;
    std::string lexeme_;
    Generic literal_;
    unsigned int line_;
  };


  std::ostream& operator<<(std::ostream& os, const TokenType type);


  std::ostream& operator<<(std::ostream& os, const Token& token);
}

#endif //LOXX_TOKEN_HPP
