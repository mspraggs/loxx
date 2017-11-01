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
        {TokenType::LeftParen, "LeftParen"},
        {TokenType::RightParen, "RightParen"},
        {TokenType::LeftBrace, "LeftBrace"},
        {TokenType::RightBrace, "RightBrace"},
        {TokenType::Comma, "Comma"},
        {TokenType::Dot, "Dot"},
        {TokenType::Minus, "Minus"},
        {TokenType::Plus, "Plus"},
        {TokenType::SemiColon, "SemiColon"},
        {TokenType::Slash, "Slash"},
        {TokenType::Star, "Star"},
        {TokenType::Bang, "Bang"},
        {TokenType::BangEqual, "BangEqual"},
        {TokenType::Equal, "Equal"},
        {TokenType::EqualEqual, "EqualEqual"},
        {TokenType::Greater, "Greater"},
        {TokenType::GreaterEqual, "GreaterEqual"},
        {TokenType::Less, "Less"},
        {TokenType::LessEqual, "LessEqual"},
        {TokenType::Identifier, "Identifier"},
        {TokenType::String, "String"},
        {TokenType::Number, "Number"},
        {TokenType::And, "And"},
        {TokenType::Class, "Class"},
        {TokenType::Else, "Else"},
        {TokenType::False, "False"},
        {TokenType::Fun, "Fun"},
        {TokenType::For, "For"},
        {TokenType::If, "If"},
        {TokenType::Nil, "Nil"},
        {TokenType::Or, "Or"},
        {TokenType::Print, "Print"},
        {TokenType::Return, "Return"},
        {TokenType::Super, "Super"},
        {TokenType::This, "This"},
        {TokenType::True, "True"},
        {TokenType::Var, "Var"},
        {TokenType::While, "While"},
        {TokenType::Eof, "Eof"}
    };
    os << type_map[type];
    return os;
  }


  std::ostream& operator<<(std::ostream& os, const Token& token)
  {
    std::string literal_string = "None";

    const Generic& literal = token.literal();

    if (token.type() == TokenType::Number) {
      literal_string = std::to_string(literal.get<double>());
    }
    else if (token.type() == TokenType::String) {
      literal_string = literal.get<std::string>();
    }

    os << token.type() << ' ' << token.lexeme() << ' ' << literal_string;
    return os;
  }
}