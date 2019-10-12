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
      if (match({TokenType::CLASS})) {
        return class_declaration();
      }
      if (match({TokenType::FUN})) {
        return function("function");
      }
      if (match({TokenType::VAR})) {
        return var_declaration();
      }

      return statement();
    }
    catch (const ParseError& e) {
      synchronise();

      return std::unique_ptr<Stmt>();
    }
  }


  std::unique_ptr<Stmt> Parser::class_declaration()
  {
    auto name = consume(TokenType::IDENTIFIER, "Expected class name.");

    auto superclass = std::unique_ptr<Expr>();
    if (match({TokenType::LESS})) {
      consume(TokenType::IDENTIFIER, "Expected superclass name.");
      superclass = std::make_unique<Variable>(previous());
    }

    consume(TokenType::LEFT_BRACE, "Expected '{' before class body.");

    std::vector<std::unique_ptr<Function>> methods;
    while (not check(TokenType::RIGHT_BRACE) and not is_at_end()) {
      std::unique_ptr<Stmt> ptr = function("method");
      methods.emplace_back(static_cast<Function*>(ptr.release()));
    }

    consume(TokenType::RIGHT_BRACE, "Expected '}' after class body.");

    return std::make_unique<Class>(std::move(name), std::move(superclass),
                                   std::move(methods));
  }


  std::unique_ptr<Stmt> Parser::statement()
  {
    if (match({TokenType::IF})) {
      return if_statement();
    }
    if (match({TokenType::PRINT})) {
      return print_statement();
    }
    if (match({TokenType::RETURN})) {
      return return_statement();
    }
    if (match({TokenType::LEFT_BRACE})) {
      return std::make_unique<Block>(block());
    }
    if (match({TokenType::WHILE})) {
      return while_statement();
    }
    if (match({TokenType::FOR})) {
      return for_statement();
    }
    return expression_statement();
  }


  std::unique_ptr<Stmt> Parser::if_statement()
  {
    consume(TokenType::LEFT_PAREN, "Expected '(' after 'if'.");
    auto condition = expression();
    consume(TokenType::RIGHT_PAREN, "Expected ')' after condition.");

    auto then_branch = statement();
    auto else_branch =
        match({TokenType::ELSE}) ? statement() : std::unique_ptr<Stmt>();

    return std::make_unique<If>(std::move(condition), std::move(then_branch),
                                std::move(else_branch));
  }


  std::unique_ptr<Stmt> Parser::print_statement()
  {
    auto expr = expression();
    consume(TokenType::SEMI_COLON, "Expect ';' after value.");
    return std::make_unique<Print>(std::move(expr));
  }


  std::unique_ptr<Stmt> Parser::return_statement()
  {
    auto keyword = previous();
    auto value = not check(TokenType::SEMI_COLON) ?
                 expression() : std::unique_ptr<Expr>();

    consume(TokenType::SEMI_COLON, "Expected ';' after return value.");
    return std::make_unique<Return>(std::move(keyword), std::move(value));
  }


  std::unique_ptr<Stmt> Parser::var_declaration()
  {
    auto name = consume(TokenType::IDENTIFIER, "Expected variable name.");

    auto initialiser =
        match({TokenType::EQUAL}) ? expression() : std::unique_ptr<Expr>();

    consume(TokenType::SEMI_COLON, "Expected ';' after variable declaration.");
    return std::make_unique<Var>(std::move(name), std::move(initialiser));
  }


  std::unique_ptr<Stmt> Parser::while_statement()
  {
    consume(TokenType::LEFT_PAREN, "Expected '(' after 'while'.");
    auto condition = expression();
    consume(TokenType::RIGHT_PAREN, "Expected ')' after condition.");
    auto body = statement();

    return std::make_unique<While>(std::move(condition), std::move(body));
  }


  std::unique_ptr<Stmt> Parser::for_statement()
  {
    consume(TokenType::LEFT_PAREN, "Expected '(' after 'for'.");

    std::unique_ptr<Stmt> initialiser;
    if (match({TokenType::SEMI_COLON})) {
    }
    else if (match({TokenType::VAR})) {
      initialiser = var_declaration();
    }
    else {
      initialiser = expression_statement();
    }

    std::unique_ptr<Expr> condition;
    if (not check(TokenType::SEMI_COLON)) {
      condition = expression();
    }
    consume(TokenType::SEMI_COLON, "Expected ';' after for-loop condition.");

    std::unique_ptr<Expr> increment;
    if (not check(TokenType::RIGHT_PAREN)) {
      increment = expression();
    }
    consume(TokenType::RIGHT_PAREN, "Expected ')' after for-loop clauses.");

    auto body = statement();

    using StmtList = std::vector<std::unique_ptr<Stmt>>;

    if (increment != nullptr) {
      StmtList stmts(2);
      stmts[0] = std::move(body);
      stmts[1] = std::make_unique<Expression>(std::move(increment));
      body = std::make_unique<Block>(std::move(stmts));
    }

    if (condition == nullptr) {
      condition = std::make_unique<Literal>(Value(InPlace<bool>(), true),
                                            "true");
    }
    body = std::make_unique<While>(std::move(condition), std::move(body));

    if (initialiser != nullptr) {
      StmtList stmts(2);
      stmts[0] = std::move(initialiser);
      stmts[1] = std::move(body);
      body = std::make_unique<Block>(std::move(stmts));
    }

    return body;
  }


  std::unique_ptr<Stmt> Parser::expression_statement()
  {
    auto expr = expression();
    consume(TokenType::SEMI_COLON, "Expected ';' after expression.");
    return std::make_unique<Expression>(std::move(expr));
  }


  std::vector<std::unique_ptr<Stmt>> Parser::block()
  {
    std::vector<std::unique_ptr<Stmt>> statements;

    while (not check(TokenType::RIGHT_BRACE) and not is_at_end()) {
      statements.push_back(declaration());
    }

    consume(TokenType::RIGHT_BRACE, "Expected '}' after block.");
    return statements;
  }


  std::unique_ptr<Stmt> Parser::function(const std::string& kind)
  {
    auto name = consume(TokenType::IDENTIFIER, "Expected " + kind + " name.");
    consume(TokenType::LEFT_PAREN, "Expected '(' after " + kind + " name.");

    std::vector<Token> parameters;

    if (not check(TokenType::RIGHT_PAREN)) {
      do {
        if (parameters.size() >= 8) {
          error(peek(), "Cannot have more than eight function parameters.");
        }
        parameters.push_back(consume(TokenType::IDENTIFIER,
                                     "Expected parameter name."));
      } while (match({TokenType::COMMA}));
    }
    consume(TokenType::RIGHT_PAREN, "Expected ')' after parameters.");

    consume(TokenType::LEFT_BRACE, "Expected '{' before " + kind + " body.");
    auto body = block();
    return std::make_unique<Function>(std::move(name), std::move(parameters),
                                      std::move(body));
  }


  std::unique_ptr<Expr> Parser::assignment()
  {
    auto expr = logical_or();

    if (match({TokenType::EQUAL})) {
      auto equals = previous();
      auto value = assignment();

      if (typeid(*expr) == typeid(Variable)) {
        Token name = static_cast<Variable*>(expr.get())->name;
        return std::make_unique<Assign>(std::move(name), std::move(value));
      }
      if (typeid(*expr) == typeid(Get)) {
        auto get = static_cast<Get*>(expr.get());
        return std::make_unique<Set>(std::move(get->object), get->name,
                                     std::move(value));
      }

      error(equals, "Invalid assignment target.");
    }

    return expr;
  }


  std::unique_ptr<Expr> Parser::logical_or()
  {
    auto expr = logical_and();

    while (match({TokenType::OR})) {
      auto op = previous();
      auto right = logical_and();
      expr = std::make_unique<Logical>(std::move(expr), std::move(op),
                                       std::move(right));
    }

    return expr;
  }


  std::unique_ptr<Expr> Parser::logical_and()
  {
    auto expr = equality();

    while (match({TokenType::AND})) {
      auto op = previous();
      auto right = equality();
      expr = std::make_unique<Logical>(std::move(expr), std::move(op),
                                       std::move(right));
    }

    return expr;
  }


  std::unique_ptr<Expr> Parser::equality()
  {
    return binary([this] () { return comparison(); },
                  {TokenType::BANG_EQUAL, TokenType::EQUAL_EQUAL});
  }


  std::unique_ptr<Expr> Parser::comparison()
  {
    return binary([this] () { return addition(); },
                  {TokenType::GREATER, TokenType::GREATER_EQUAL,
                   TokenType::LESS, TokenType::LESS_EQUAL});
  }


  std::unique_ptr<Expr> Parser::addition()
  {
    return binary([this] () { return multiplication(); },
                  {TokenType::MINUS, TokenType::PLUS});
  }


  std::unique_ptr<Expr> Parser::multiplication()
  {
    return binary([this] () { return unary(); },
                  {TokenType::SLASH, TokenType::STAR});
  }


  std::unique_ptr<Expr> Parser::unary()
  {
    if (match({TokenType::BANG, TokenType::MINUS})) {
      Token op = previous();
      auto right = unary();
      return std::make_unique<Unary>(op, std::move(right));
    }

    return call();
  }


  std::unique_ptr<Expr> Parser::finish_call(std::unique_ptr<Expr> callee)
  {
    std::vector<std::unique_ptr<Expr>> arguments;

    if (not check(TokenType::RIGHT_PAREN)) {
      do {
        if (arguments.size() >= 8) {
          error(peek(), "Cannot have more than eight function arguments.");
        }
        arguments.push_back(expression());
      } while (match({TokenType::COMMA}));
    }

    auto paren = consume(TokenType::RIGHT_PAREN, "Expected ')' after arguments.");

    return std::make_unique<Call>(std::move(callee), std::move(paren),
                                  std::move(arguments));
  }


  std::unique_ptr<Expr> Parser::call()
  {
    auto expr = primary();

    while (true) {
      if (match({TokenType::LEFT_PAREN})) {
        expr = finish_call(std::move(expr));
      }
      else if (match({TokenType::DOT})) {
        auto name = consume(TokenType::IDENTIFIER,
                            "Expected property name after '.'.");
        expr = std::make_unique<Get>(std::move(expr), std::move(name));
      }
      else {
        break;
      }
    }

    return expr;
  }


  std::unique_ptr<Expr> Parser::primary()
  {
    if (match({TokenType::FALSE})) {
      return std::make_unique<Literal>(Value(InPlace<bool>(), false),
                                       previous().lexeme());
    }
    if (match({TokenType::TRUE})) {
      return std::make_unique<Literal>(Value(InPlace<bool>(), true),
                                       previous().lexeme());
    }
    if (match({TokenType::NIL})) {
      return std::make_unique<Literal>(Value(), previous().lexeme());
    }

    if (match({TokenType::NUMBER, TokenType::STRING})) {
      return std::make_unique<Literal>(previous().literal(),
                                       previous().lexeme());
    }

    if (match({TokenType::LEFT_PAREN})) {
      auto expr = expression();
      consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
      return std::make_unique<Grouping>(std::move(expr));
    }

    if (match({TokenType::SUPER})) {
      auto keyword = previous();
      consume(TokenType::DOT, "Expected '.' after 'super'.");
      auto method = consume(TokenType::IDENTIFIER,
                            "Expected superclass method name.");
      return std::make_unique<Super>(std::move(keyword), std::move(method));
    }

    if (match({TokenType::THIS})) {
      return std::make_unique<This>(previous());
    }

    if (match({TokenType::IDENTIFIER})) {
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
    return peek().type() == TokenType::END_OF_FILE;
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
      if (previous().type() == TokenType::SEMI_COLON) {
        return;
      }

      switch (peek().type()) {
        case TokenType::CLASS:
        case TokenType::FUN:
        case TokenType::VAR:
        case TokenType::FOR:
        case TokenType::IF:
        case TokenType::WHILE:
        case TokenType::PRINT:
        case TokenType::RETURN:
          return;
        default:
          advance();
          break;
      }
    }
  }
}
