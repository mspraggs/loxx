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
 * Created by Matt Spraggs on 03/11/17.
 */

#include "Parser.hpp"
#include "logging.hpp"


namespace loxx
{
  std::unique_ptr<Expr> Parser::equality()
  {
    auto expr = comparison();

    while (match({TokenType::BangEqual, TokenType::EqualEqual})) {
      Token op = previous();
      auto right = comparison();
      expr = std::make_unique<Binary>(std::move(expr), op, std::move(right));
    }

    return expr;
  }


  std::unique_ptr<Expr> Parser::comparison()
  {
    auto expr = addition();

    while (match({TokenType::Greater, TokenType::GreaterEqual,
                  TokenType::Less, TokenType::LessEqual})) {
      Token op = previous();
      auto right = addition();
      expr = std::make_unique<Binary>(std::move(expr), op, std::move(right));
    }

    return expr;
  }


  std::unique_ptr<Expr> Parser::addition()
  {
    auto expr = multiplication();

    while (match({TokenType::Minus, TokenType::Plus})) {
      Token op = previous();
      auto right = multiplication();
      expr = std::make_unique<Binary>(std::move(expr), op, std::move(right));
    }

    return expr;
  }


  std::unique_ptr<Expr> Parser::multiplication()
  {
    auto expr = unary();

    while (match({TokenType::Slash, TokenType::Star})) {
      Token op = previous();
      auto right = unary();
      expr = std::make_unique<Binary>(std::move(expr), op, std::move(right));
    }

    return expr;
  }


  std::unique_ptr<Expr> Parser::unary()
  {
    if (match({TokenType::Bang, TokenType::Minus})) {
      Token op = previous();
      auto right = unary();
      return std::make_unique<Unary>(op, std::move(right));
    }

    return primary();
  }


  std::unique_ptr<Expr> Parser::primary()
  {
    if (match({TokenType::False})) {
      return std::make_unique<Literal>(Generic(false));
    }
    if (match({TokenType::True})) {
      return std::make_unique<Literal>(Generic(true));
    }
    if (match({TokenType::Nil})) {
      return std::make_unique<Literal>(Generic(nullptr));
    }

    if (match({TokenType::Number, TokenType::String})) {
      return std::make_unique<Literal>(previous().literal());
    }

    if (match({TokenType::LeftParen})) {
      auto expr = expression();
      consume(TokenType::RightParen, "Expect ')' after expression.");
      return std::make_unique<Grouping>(std::move(expr));
    }

    throw error(peek(), "Expected expression.");
  }


  bool Parser::match(std::initializer_list<TokenType> types)
  {
    for (const auto type : types) {
      if (check(type)) {
        advance();
        return true;
      }
    }
    return false;
  }


  const Token& Parser::consume(const TokenType type, const std::string& message)
  {
    if (check(type)) {
      return advance();
    }

    throw error(peek(), message);
  }


  bool Parser::check(const TokenType type) const
  {
    if (is_at_end()) {
      return false;
    }
    return peek().type() == type;
  }


  const Token& Parser::advance()
  {
    if (not is_at_end()) {
      current_ += 1;
    }
    return previous();
  }


  bool Parser::is_at_end() const
  {
    return peek().type() == TokenType::Eof;
  }


  const Token& Parser::peek() const
  {
    return tokens_[current_];
  }


  const Token& Parser::previous() const
  {
    return tokens_[current_ - 1];
  }


  Parser::ParseError Parser::error(const Token& token,
                                   const std::string& message)
  {
    loxx::error(token, message);
    return ParseError();
  }


  void Parser::synchronise()
  {
    advance();

    while (not is_at_end()) {
      if (previous().type() == TokenType::SemiColon) {
        return;
      }

      switch (peek().type()) {
        case TokenType::Class:
        case TokenType::Fun:
        case TokenType::Var:
        case TokenType::For:
        case TokenType::If:
        case TokenType::While:
        case TokenType::Print:
        case TokenType::Return:
          return;
        default:
          advance();
          break;
      }
    }
  }
}