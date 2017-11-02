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

  class Unary;
  class Binary;
  class Literal;
  class Grouping;

  class Visitor
  {
  public:
    virtual void visitUnaryExpr(const Unary& expr) = 0;
    virtual void visitBinaryExpr(const Binary& expr) = 0;
    virtual void visitLiteralExpr(const Literal& expr) = 0;
    virtual void visitGroupingExpr(const Grouping& expr) = 0;
  };

  class Expr
  {
  public:
    virtual ~Expr() = default;

    virtual void accept(Visitor& visitor) const = 0;
  };


  class Unary : public Expr
  {
  public:
    Unary(Token op, std::unique_ptr<Expr> right)
        : op_(std::move(op)), right_(std::move(right))
    {}

    void accept(Visitor& visitor) const override
    { visitor.visitUnaryExpr(*this); }

    const Token& op() const { return op_; }
    const Expr& right() const { return *right_; }

  private:
    Token op_;
    std::unique_ptr<Expr> right_;
  };


  class Binary : public Expr
  {
  public:
    Binary(std::unique_ptr<Expr> left, Token op, std::unique_ptr<Expr> right)
        : left_(std::move(left)), op_(std::move(op)), right_(std::move(right))
    {}

    void accept(Visitor& visitor) const override
    { visitor.visitBinaryExpr(*this); }

    const Expr& left() const { return *left_; }
    const Token& op() const { return op_; }
    const Expr& right() const { return *right_; }

  private:
    std::unique_ptr<Expr> left_;
    Token op_;
    std::unique_ptr<Expr> right_;
  };


  class Literal : public Expr
  {
  public:
    Literal(Generic value)
        : value_(std::move(value))
    {}

    void accept(Visitor& visitor) const override
    { visitor.visitLiteralExpr(*this); }

    const Generic& value() const { return value_; }

  private:
    Generic value_;
  };


  class Grouping : public Expr
  {
  public:
    Grouping(std::unique_ptr<Expr> expression)
        : expression_(std::move(expression))
    {}

    void accept(Visitor& visitor) const override
    { visitor.visitGroupingExpr(*this); }

    const Expr& expression() const { return *expression_; }

  private:
    std::unique_ptr<Expr> expression_;
  };

}

#endif //LOXX_EXPRESSIONS_HPP_HPP