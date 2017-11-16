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

#ifndef LOXX_EXPR_HPP
#define LOXX_EXPR_HPP

#include "Generic.hpp"
#include "Token.hpp"


namespace loxx
{

  class Unary;
  class Binary;
  class Literal;
  class Variable;
  class Grouping;

  class Expr
  {
  public:
    virtual ~Expr() = default;

    class Visitor
    {
    public:
      virtual void visit_unary_expr(const Unary& expr) = 0;
      virtual void visit_binary_expr(const Binary& expr) = 0;
      virtual void visit_literal_expr(const Literal& expr) = 0;
      virtual void visit_variable_expr(const Variable& expr) = 0;
      virtual void visit_grouping_expr(const Grouping& expr) = 0;
    };

    virtual void accept(Visitor& visitor) const {}
  };


  class Unary : public Expr
  {
  public:
    Unary(Token op, std::unique_ptr<Expr> right)
        : op_(std::move(op)), right_(std::move(right))
    {}

    void accept(Visitor& visitor) const override
    { visitor.visit_unary_expr(*this); }

    const Token& op() const { return op_; }
    const Expr& right() const { if (right_ == nullptr) throw std::out_of_range("Member right_ contains nullptr!"); return *right_; }

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
    { visitor.visit_binary_expr(*this); }

    const Expr& left() const { if (left_ == nullptr) throw std::out_of_range("Member left_ contains nullptr!"); return *left_; }
    const Token& op() const { return op_; }
    const Expr& right() const { if (right_ == nullptr) throw std::out_of_range("Member right_ contains nullptr!"); return *right_; }

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
    { visitor.visit_literal_expr(*this); }

    const Generic& value() const { return value_; }

  private:
    Generic value_;
  };


  class Variable : public Expr
  {
  public:
    Variable(Token name)
        : name_(std::move(name))
    {}

    void accept(Visitor& visitor) const override
    { visitor.visit_variable_expr(*this); }

    const Token& name() const { return name_; }

  private:
    Token name_;
  };


  class Grouping : public Expr
  {
  public:
    Grouping(std::unique_ptr<Expr> expression)
        : expression_(std::move(expression))
    {}

    void accept(Visitor& visitor) const override
    { visitor.visit_grouping_expr(*this); }

    const Expr& expression() const { if (expression_ == nullptr) throw std::out_of_range("Member expression_ contains nullptr!"); return *expression_; }

  private:
    std::unique_ptr<Expr> expression_;
  };

}

#endif //LOXX_EXPR_HPP