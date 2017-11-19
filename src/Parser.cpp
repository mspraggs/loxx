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
  std::unique_ptr<Stmt> Parser::declaration()
  {
    try {
      if (match({TokenType::Var})) {
        return var_declaration();
      }

      return statement();
    }
    catch (const ParseError& e) {
      synchronise();

      return std::unique_ptr<Stmt>();
    }
  }


  std::unique_ptr<Stmt> Parser::statement()
  {
    if (match({TokenType::Print})) {
      return print_statement();
    }
    if (match({TokenType::LeftBrace})) {
      return std::make_unique<Block>(block());
    }
    return expression_statement();
  }


  std::unique_ptr<Stmt> Parser::print_statement()
  {
    auto expr = expression();
    consume(TokenType::SemiColon, "Expect ';' after value.");
    return std::make_unique<Print>(std::move(expr));
  }


  std::unique_ptr<Stmt> Parser::var_declaration()
  {
    auto name = consume(TokenType::Identifier, "Expected variable name.");

    auto initialiser =
        match({TokenType::Equal}) ? expression() : std::unique_ptr<Expr>();

    consume(TokenType::SemiColon, "Expected ';' after variable declaration.");
    return std::make_unique<Var>(std::move(name), std::move(initialiser));
  }


  std::unique_ptr<Stmt> Parser::expression_statement()
  {
    auto expr = expression();

    if (check(TokenType::SemiColon) or not in_repl_) {
      consume(TokenType::SemiColon, "Expxect ';' after expression");
    }
    return std::make_unique<Expression>(std::move(expr));
  }


  std::vector<std::unique_ptr<Stmt>> Parser::block()
  {
    std::vector<std::unique_ptr<Stmt>> statements;

    while (not check(TokenType::RightBrace) and not is_at_end()) {
      statements.push_back(declaration());
    }

    consume(TokenType::RightBrace, "Expected '}' after block.");
    return statements;
  }


  std::unique_ptr<Expr> Parser::comma()
  {
    return binary([this] () { return assignment(); },
                  {TokenType::Comma});
  }


  std::unique_ptr<Expr> Parser::assignment()
  {
    auto expr = ternary();

    if (match({TokenType::Equal})) {
      auto equals = previous();
      auto value = assignment();

      if (typeid(*expr) == typeid(Variable)) {
        Token name = static_cast<Variable*>(expr.get())->name();
        return std::make_unique<Assign>(std::move(name), std::move(value));
      }

      error(equals, "Invalid assignment target.");
    }

    return expr;
  }


  std::unique_ptr<Expr> Parser::ternary()
  {
    auto first = equality();

    if (match({TokenType::Question})) {
      Token op = previous();
      auto second = ternary();

      if (match({TokenType::Colon})) {
        auto third = ternary();

        return std::make_unique<Ternary>(std::move(first), op,
                                         std::move(second), std::move(third));
      }

      throw error(previous(), "Ternary expression expected ':'");
    }

    return first;
  }


  std::unique_ptr<Expr> Parser::equality()
  {
    return binary([this] () { return comparison(); },
                  {TokenType::BangEqual, TokenType::EqualEqual});
  }


  std::unique_ptr<Expr> Parser::comparison()
  {
    return binary([this] () { return addition(); },
                  {TokenType::Greater, TokenType::GreaterEqual,
                   TokenType::Less, TokenType::LessEqual});
  }


  std::unique_ptr<Expr> Parser::addition()
  {
    return binary([this] () { return multiplication(); },
                  {TokenType::Minus, TokenType::Plus});
  }


  std::unique_ptr<Expr> Parser::multiplication()
  {
    return binary([this] () { return unary(); },
                  {TokenType::Slash, TokenType::Star});
  }


  std::unique_ptr<Expr> Parser::unary()
  {
    if (match({TokenType::BangEqual, TokenType::EqualEqual,
               TokenType::Greater, TokenType::GreaterEqual,
               TokenType::Less, TokenType::LessEqual, TokenType::Plus,
               TokenType::Slash, TokenType::Star}))
    {
      throw error(previous(), "Binary operator expects two operands");
    }

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

    if (match({TokenType::Identifier})) {
      return std::make_unique<Variable>(previous());
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