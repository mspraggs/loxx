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
 * Created by Matt Spraggs on 01/11/17.
 */

#include <unordered_map>

#include "Object.hpp"
#include "Token.hpp"


namespace loxx
{
  struct TokenTypeHasher
  {
    std::size_t operator()(TokenType type) const
    {
      return static_cast<std::size_t>(type);
    }
  };


  std::ostream& operator<<(std::ostream& os, const TokenType type)
  {
    static std::unordered_map<TokenType, const char*, TokenTypeHasher> type_map{
        {TokenType::LEFT_PAREN, "LEFT_PAREN"},
        {TokenType::RIGHT_PAREN, "RIGHT_PAREN"},
        {TokenType::LEFT_BRACE, "LEFT_BRACE"},
        {TokenType::RIGHT_BRACE, "RIGHT_BRACE"},
        {TokenType::COMMA, "COMMA"},
        {TokenType::DOT, "DOT"},
        {TokenType::MINUS, "MINUS"},
        {TokenType::PLUS, "PLUS"},
        {TokenType::SEMI_COLON, "SEMI_COLON"},
        {TokenType::SLASH, "SLASH"},
        {TokenType::STAR, "STAR"},
        {TokenType::BANG, "BANG"},
        {TokenType::BANG_EQUAL, "BANG_EQUAL"},
        {TokenType::EQUAL, "Equal"},
        {TokenType::EQUAL_EQUAL, "EQUAL_EQUAL"},
        {TokenType::GREATER, "GREATER"},
        {TokenType::GREATER_EQUAL, "GREATER_EQUAL"},
        {TokenType::LESS, "LESS"},
        {TokenType::LESS_EQUAL, "LESS_EQUAL"},
        {TokenType::IDENTIFIER, "IDENTIFIER"},
        {TokenType::STRING, "STRING"},
        {TokenType::NUMBER, "NUMBER"},
        {TokenType::AND, "AND"},
        {TokenType::CLASS, "CLASS"},
        {TokenType::ELSE, "ELSE"},
        {TokenType::FALSE, "FALSE"},
        {TokenType::FUN, "FUN"},
        {TokenType::FOR, "FOR"},
        {TokenType::IF, "If"},
        {TokenType::NIL, "NIL"},
        {TokenType::OR, "OR"},
        {TokenType::PRINT, "PRINT"},
        {TokenType::RETURN, "RETURN"},
        {TokenType::SUPER, "SUPER"},
        {TokenType::THIS, "THIS"},
        {TokenType::TRUE, "TRUE"},
        {TokenType::VAR, "VAR"},
        {TokenType::WHILE, "WHILE"},
        {TokenType::END_OF_FILE, "EOF"}
    };
    os << type_map[type];
    return os;
  }


  std::ostream& operator<<(std::ostream& os, const Token& token)
  {
    std::string literal_string = "None";

    const Value& literal = token.literal();

    if (token.type() == TokenType::NUMBER) {
      literal_string = std::to_string(get<double>(literal));
    }
    else if (token.type() == TokenType::STRING) {
      literal_string =
          static_cast<std::string>(*get_object<StringObject>(literal));
    }
    else if (token.type() == TokenType::TRUE) {
      literal_string = "true";
    }
    else if (token.type() == TokenType::FALSE) {
      literal_string = "false";
    }
    else if (token.type() == TokenType::NIL) {
      literal_string = "nil";
    }

    os << token.type() << ' ' << token.lexeme() << ' ' << literal_string;
    return os;
  }
}
