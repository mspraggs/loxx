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

#include <memory>
#include <vector>

#include "Generic.hpp"
#include "Token.hpp"


namespace loxx
{

  class Stmt;

  struct Binary;
  struct This;
  struct Literal;
  struct Call;
  struct Ternary;
  struct Assign;
  struct Lambda;
  struct Set;
  struct Get;
  struct Logical;
  struct Unary;
  struct Variable;
  struct Grouping;

  struct Expr
  {
    virtual ~Expr() = default;

    class Visitor
    {
    public:
      virtual void visit_binary_expr(const Binary& expr) = 0;
      virtual void visit_this_expr(const This& expr) = 0;
      virtual void visit_literal_expr(const Literal& expr) = 0;
      virtual void visit_call_expr(const Call& expr) = 0;
      virtual void visit_ternary_expr(const Ternary& expr) = 0;
      virtual void visit_assign_expr(const Assign& expr) = 0;
      virtual void visit_lambda_expr(const Lambda& expr) = 0;
      virtual void visit_set_expr(const Set& expr) = 0;
      virtual void visit_get_expr(const Get& expr) = 0;
      virtual void visit_logical_expr(const Logical& expr) = 0;
      virtual void visit_unary_expr(const Unary& expr) = 0;
      virtual void visit_variable_expr(const Variable& expr) = 0;
      virtual void visit_grouping_expr(const Grouping& expr) = 0;
    };

    virtual void accept(Visitor&) const {}
  };


  struct Binary : public Expr
  {
    Binary(std::shared_ptr<Expr> left_arg, Token op_arg, std::shared_ptr<Expr> right_arg)
        : left(std::move(left_arg)), op(std::move(op_arg)), right(std::move(right_arg))
    {}

    void accept(Visitor& visitor) const override
    { visitor.visit_binary_expr(*this); }

    std::shared_ptr<Expr> left;
    Token op;
    std::shared_ptr<Expr> right;
  };


  struct This : public Expr
  {
    This(Token keyword_arg)
        : keyword(std::move(keyword_arg))
    {}

    void accept(Visitor& visitor) const override
    { visitor.visit_this_expr(*this); }

    Token keyword;
  };


  struct Literal : public Expr
  {
    Literal(Generic value_arg)
        : value(std::move(value_arg))
    {}

    void accept(Visitor& visitor) const override
    { visitor.visit_literal_expr(*this); }

    Generic value;
  };


  struct Call : public Expr
  {
    Call(std::shared_ptr<Expr> callee_arg, Token paren_arg, std::vector<std::shared_ptr<Expr>> arguments_arg)
        : callee(std::move(callee_arg)), paren(std::move(paren_arg)), arguments(std::move(arguments_arg))
    {}

    void accept(Visitor& visitor) const override
    { visitor.visit_call_expr(*this); }

    std::shared_ptr<Expr> callee;
    Token paren;
    std::vector<std::shared_ptr<Expr>> arguments;
  };


  struct Ternary : public Expr
  {
    Ternary(std::shared_ptr<Expr> first_arg, Token op_arg, std::shared_ptr<Expr> second_arg, std::shared_ptr<Expr> third_arg)
        : first(std::move(first_arg)), op(std::move(op_arg)), second(std::move(second_arg)), third(std::move(third_arg))
    {}

    void accept(Visitor& visitor) const override
    { visitor.visit_ternary_expr(*this); }

    std::shared_ptr<Expr> first;
    Token op;
    std::shared_ptr<Expr> second;
    std::shared_ptr<Expr> third;
  };


  struct Assign : public Expr
  {
    Assign(Token name_arg, std::shared_ptr<Expr> value_arg)
        : name(std::move(name_arg)), value(std::move(value_arg))
    {}

    void accept(Visitor& visitor) const override
    { visitor.visit_assign_expr(*this); }

    Token name;
    std::shared_ptr<Expr> value;
  };


  struct Lambda : public Expr
  {
    Lambda(Token start_arg, std::vector<Token> parameters_arg, std::vector<std::shared_ptr<Stmt>> body_arg)
        : start(std::move(start_arg)), parameters(std::move(parameters_arg)), body(std::move(body_arg))
    {}

    void accept(Visitor& visitor) const override
    { visitor.visit_lambda_expr(*this); }

    Token start;
    std::vector<Token> parameters;
    std::vector<std::shared_ptr<Stmt>> body;
  };


  struct Set : public Expr
  {
    Set(std::shared_ptr<Expr> object_arg, Token name_arg, std::shared_ptr<Expr> value_arg)
        : object(std::move(object_arg)), name(std::move(name_arg)), value(std::move(value_arg))
    {}

    void accept(Visitor& visitor) const override
    { visitor.visit_set_expr(*this); }

    std::shared_ptr<Expr> object;
    Token name;
    std::shared_ptr<Expr> value;
  };


  struct Get : public Expr
  {
    Get(std::shared_ptr<Expr> object_arg, Token name_arg)
        : object(std::move(object_arg)), name(std::move(name_arg))
    {}

    void accept(Visitor& visitor) const override
    { visitor.visit_get_expr(*this); }

    std::shared_ptr<Expr> object;
    Token name;
  };


  struct Logical : public Expr
  {
    Logical(std::shared_ptr<Expr> left_arg, Token op_arg, std::shared_ptr<Expr> right_arg)
        : left(std::move(left_arg)), op(std::move(op_arg)), right(std::move(right_arg))
    {}

    void accept(Visitor& visitor) const override
    { visitor.visit_logical_expr(*this); }

    std::shared_ptr<Expr> left;
    Token op;
    std::shared_ptr<Expr> right;
  };


  struct Unary : public Expr
  {
    Unary(Token op_arg, std::shared_ptr<Expr> right_arg)
        : op(std::move(op_arg)), right(std::move(right_arg))
    {}

    void accept(Visitor& visitor) const override
    { visitor.visit_unary_expr(*this); }

    Token op;
    std::shared_ptr<Expr> right;
  };


  struct Variable : public Expr
  {
    Variable(Token name_arg)
        : name(std::move(name_arg))
    {}

    void accept(Visitor& visitor) const override
    { visitor.visit_variable_expr(*this); }

    Token name;
  };


  struct Grouping : public Expr
  {
    Grouping(std::shared_ptr<Expr> expression_arg)
        : expression(std::move(expression_arg))
    {}

    void accept(Visitor& visitor) const override
    { visitor.visit_grouping_expr(*this); }

    std::shared_ptr<Expr> expression;
  };

}

#endif //LOXX_EXPR_HPP