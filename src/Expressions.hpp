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

#ifndef LOXX_EXPRESSIONS_HPP_HPP
#define LOXX_EXPRESSIONS_HPP_HPP

#include "Token.hpp"


namespace loxx
{
  class Expr
  {
  public:
    virtual ~Expr() = default;
  };


  class Unary : public Expr
  {
  public:
    Unary(Token op, Expr right)
        : op_(std::move(op)), right_(std::move(right))
    {}

    const Token& op() const { return op_; }
    const Expr& right() const { return right_; }

  private:
    Token op_;
    Expr right_;
  };


  class Binary : public Expr
  {
  public:
    Binary(Expr left, Token op, Expr right)
        : left_(std::move(left)), op_(std::move(op)), right_(std::move(right))
    {}

    const Expr& left() const { return left_; }
    const Token& op() const { return op_; }
    const Expr& right() const { return right_; }

  private:
    Expr left_;
    Token op_;
    Expr right_;
  };


  class StringLiteral : public Expr
  {
  public:
    StringLiteral(std::string value)
        : value_(std::move(value))
    {}

    const std::string& value() const { return value_; }

  private:
    std::string value_;
  };


  class NumberLiteral : public Expr
  {
  public:
    NumberLiteral(double value)
        : value_(std::move(value))
    {}

    const double& value() const { return value_; }

  private:
    double value_;
  };


  class Grouping : public Expr
  {
  public:
    Grouping(Expr expression)
        : expression_(std::move(expression))
    {}

    const Expr& expression() const { return expression_; }

  private:
    Expr expression_;
  };

}

#endif //LOXX_EXPRESSIONS_HPP_HPP