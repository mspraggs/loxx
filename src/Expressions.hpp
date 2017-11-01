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

#include "Generic.hpp"
#include "Token.hpp"


namespace loxx
{
  class Visitor
  {
  public:
    virtual Generic visitUnaryExpr(const Unary& expr) = 0;
    virtual Generic visitBinaryExpr(const Binary& expr) = 0;
    virtual Generic visitLiteralExpr(const Literal& expr) = 0;
    virtual Generic visitGroupingExpr(const Grouping& expr) = 0;
  };

  class Expr
  {
  public:
    virtual ~Expr() = default;

    virtual Generic accept(Visitor& visitor) const = 0;
  };


  class Unary : public Expr
  {
  public:
    Unary(Token op, Expr right)
        : op_(std::move(op)), right_(std::move(right))
    {}

    Generic accept(Visitor& visitor) const override
    { return visitor.visitUnaryExpr(*this); }

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

    Generic accept(Visitor& visitor) const override
    { return visitor.visitBinaryExpr(*this); }

    const Expr& left() const { return left_; }
    const Token& op() const { return op_; }
    const Expr& right() const { return right_; }

  private:
    Expr left_;
    Token op_;
    Expr right_;
  };


  class Literal : public Expr
  {
  public:
    Literal(Generic value)
        : value_(std::move(value))
    {}

    Generic accept(Visitor& visitor) const override
    { return visitor.visitLiteralExpr(*this); }

    const Generic& value() const { return value_; }

  private:
    Generic value_;
  };


  class Grouping : public Expr
  {
  public:
    Grouping(Expr expression)
        : expression_(std::move(expression))
    {}

    Generic accept(Visitor& visitor) const override
    { return visitor.visitGroupingExpr(*this); }

    const Expr& expression() const { return expression_; }

  private:
    Expr expression_;
  };

}

#endif //LOXX_EXPRESSIONS_HPP_HPP