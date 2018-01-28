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

#include "Stmt.hpp"
#include "Token.hpp"


namespace loxx
{
  class Parser
  {
    class ParseError;

  public:
    explicit Parser(std::vector<Token> tokens, const bool in_repl = false)
        : in_repl_(in_repl), parsing_loop_(false), current_(0),
          tokens_(std::move(tokens))
    {}

    std::vector<std::shared_ptr<Stmt>> parse() {
      std::vector<std::shared_ptr<Stmt>> statements;

      while (not is_at_end()) {
        statements.emplace_back(declaration());
      }

      return statements;
    }

  private:
    class ParseError : public std::runtime_error
    {
    public:
      ParseError() : std::runtime_error("") {}
    };

    template <typename T>
    class VariableScoper
    {
    public:
      VariableScoper(T& variable, T new_value);
      ~VariableScoper();

    private:
      T* ptr_;
      T old_value_;
    };

    std::unique_ptr<Stmt> declaration();
    std::unique_ptr<Stmt> class_declaration();
    std::unique_ptr<Stmt> statement();
    std::unique_ptr<Stmt> if_statement();
    std::unique_ptr<Stmt> print_statement();
    std::unique_ptr<Stmt> return_statement();
    std::unique_ptr<Stmt> var_declaration();
    std::unique_ptr<Stmt> while_statement();
    std::unique_ptr<Stmt> for_statement();
    std::unique_ptr<Stmt> break_statement();
    std::unique_ptr<Stmt> expression_statement();
    std::unique_ptr<Stmt> function(const std::string& kind);
    std::vector<std::shared_ptr<Stmt>> block();

    std::unique_ptr<Expr> expression() { return comma(); }
    std::unique_ptr<Expr> comma();
    std::unique_ptr<Expr> ternary();
    std::unique_ptr<Expr> assignment();
    std::unique_ptr<Expr> logical_or();
    std::unique_ptr<Expr> logical_and();
    std::unique_ptr<Expr> equality();
    std::unique_ptr<Expr> comparison();
    std::unique_ptr<Expr> addition();
    std::unique_ptr<Expr> multiplication();
    std::unique_ptr<Expr> unary();
    std::unique_ptr<Expr> finish_call(std::unique_ptr<Expr> callee);
    std::unique_ptr<Expr> call();
    std::unique_ptr<Expr> primary();
    std::unique_ptr<Expr> lambda();

    template <typename Fn>
    std::unique_ptr<Expr> binary(
        Fn fn, const std::initializer_list<TokenType>& tokens);

    std::vector<Token> parse_parameters();

    bool match(std::initializer_list<TokenType> types);
    const Token& consume(const TokenType type, const std::string& message);
    bool check(const TokenType type) const;
    const Token& advance();
    bool is_at_end() const;
    const Token& peek() const;
    const Token& previous() const;
    ParseError error(const Token& token, const std::string& message);
    void synchronise();

    bool in_repl_, parsing_loop_;
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


  template <typename T>
  Parser::VariableScoper<T>::VariableScoper(T& variable, T new_value)
      : ptr_(&variable), old_value_(variable)
  {
    *ptr_ = new_value;
  }


  template <typename T>
  Parser::VariableScoper<T>::~VariableScoper()
  {
    *ptr_ = old_value_;
  }
}

#endif //LOXX_PARSER_HPP
