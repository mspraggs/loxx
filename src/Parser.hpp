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

#ifndef LOXX_PARSER_HPP
#define LOXX_PARSER_HPP

#include <vector>

#include "Expressions.hpp"
#include "Token.hpp"


namespace loxx
{
  class Parser
  {
    class ParseError;

  public:
    explicit Parser(std::vector<Token> tokens)
        : current_(0), tokens_(std::move(tokens))
    {}

    std::unique_ptr<Expr> parse() {
      try {
        return expression();
      }
      catch (const ParseError& e) {
        return std::make_unique<Expr>();
      }
    }

  private:
    class ParseError : public std::runtime_error
    {
    public:
      ParseError() : std::runtime_error("") {}
    };

    std::unique_ptr<Expr> expression() { return comma(); }
    std::unique_ptr<Expr> comma();
    std::unique_ptr<Expr> ternary();
    std::unique_ptr<Expr> equality();
    std::unique_ptr<Expr> comparison();
    std::unique_ptr<Expr> addition();
    std::unique_ptr<Expr> multiplication();
    std::unique_ptr<Expr> unary();
    std::unique_ptr<Expr> primary();

    template <typename Fn>
    std::unique_ptr<Expr> binary(
        Fn fn, const std::initializer_list<TokenType>& tokens);

    bool match(std::initializer_list<TokenType> types);
    const Token& consume(const TokenType type, const std::string& message);
    bool check(const TokenType type) const;
    const Token& advance();
    bool is_at_end() const;
    const Token& peek() const;
    const Token& previous() const;
    ParseError error(const Token& token, const std::string& message);
    void synchronise();

    unsigned int current_;
    std::vector<Token> tokens_;
  };


  template<typename Fn>
  std::unique_ptr<Expr> Parser::binary(
      Fn fn, const std::initializer_list<TokenType>& tokens)
  {
    auto expr = fn();

    while (match(tokens)) {
      Token op = previous();
      auto right = fn();
      expr = std::make_unique<Binary>(std::move(expr), op, std::move(right));
    }

    return expr;
  }
}

#endif //LOXX_PARSER_HPP
