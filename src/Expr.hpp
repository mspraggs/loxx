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

#include "globals.hpp"
#include "Token.hpp"


namespace loxx
{

  struct Assign;
  struct Binary;
  struct Call;
  struct Get;
  struct Grouping;
  struct Literal;
  struct Logical;
  struct Set;
  struct Super;
  struct This;
  struct Unary;
  struct Variable;

  struct Expr
  {
    virtual ~Expr() = default;

    class Visitor
    {
    public:
      virtual void visit_assign_expr(const Assign& expr) = 0;
      virtual void visit_binary_expr(const Binary& expr) = 0;
      virtual void visit_call_expr(const Call& expr) = 0;
      virtual void visit_get_expr(const Get& expr) = 0;
      virtual void visit_grouping_expr(const Grouping& expr) = 0;
      virtual void visit_literal_expr(const Literal& expr) = 0;
      virtual void visit_logical_expr(const Logical& expr) = 0;
      virtual void visit_set_expr(const Set& expr) = 0;
      virtual void visit_super_expr(const Super& expr) = 0;
      virtual void visit_this_expr(const This& expr) = 0;
      virtual void visit_unary_expr(const Unary& expr) = 0;
      virtual void visit_variable_expr(const Variable& expr) = 0;
    };

    virtual void accept(Visitor&) const {}
  };


  struct Assign : public Expr
  {
    Assign(Token name_arg, std::unique_ptr<Expr> value_arg)
        : name(std::move(name_arg)), value(std::move(value_arg))
    {}

    void accept(Visitor& visitor) const override
    { visitor.visit_assign_expr(*this); }

    Token name;
    std::unique_ptr<Expr> value;
  };


  struct Binary : public Expr
  {
    Binary(std::unique_ptr<Expr> left_arg, Token op_arg, std::unique_ptr<Expr> right_arg)
        : left(std::move(left_arg)), op(std::move(op_arg)), right(std::move(right_arg))
    {}

    void accept(Visitor& visitor) const override
    { visitor.visit_binary_expr(*this); }

    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;
  };


  struct Call : public Expr
  {
    Call(std::unique_ptr<Expr> callee_arg, Token paren_arg, std::vector<std::unique_ptr<Expr>> arguments_arg)
        : callee(std::move(callee_arg)), paren(std::move(paren_arg)), arguments(std::move(arguments_arg))
    {}

    void accept(Visitor& visitor) const override
    { visitor.visit_call_expr(*this); }

    std::unique_ptr<Expr> callee;
    Token paren;
    std::vector<std::unique_ptr<Expr>> arguments;
  };


  struct Get : public Expr
  {
    Get(std::unique_ptr<Expr> object_arg, Token name_arg)
        : object(std::move(object_arg)), name(std::move(name_arg))
    {}

    void accept(Visitor& visitor) const override
    { visitor.visit_get_expr(*this); }

    std::unique_ptr<Expr> object;
    Token name;
  };


  struct Grouping : public Expr
  {
    Grouping(std::unique_ptr<Expr> expression_arg)
        : expression(std::move(expression_arg))
    {}

    void accept(Visitor& visitor) const override
    { visitor.visit_grouping_expr(*this); }

    std::unique_ptr<Expr> expression;
  };


  struct Literal : public Expr
  {
    Literal(StackVar value_arg, std::string lexeme_arg)
        : value(std::move(value_arg)), lexeme(std::move(lexeme_arg))
    {}

    void accept(Visitor& visitor) const override
    { visitor.visit_literal_expr(*this); }

    StackVar value;
    std::string lexeme;
  };


  struct Logical : public Expr
  {
    Logical(std::unique_ptr<Expr> left_arg, Token op_arg, std::unique_ptr<Expr> right_arg)
        : left(std::move(left_arg)), op(std::move(op_arg)), right(std::move(right_arg))
    {}

    void accept(Visitor& visitor) const override
    { visitor.visit_logical_expr(*this); }

    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;
  };


  struct Set : public Expr
  {
    Set(std::unique_ptr<Expr> object_arg, Token name_arg, std::unique_ptr<Expr> value_arg)
        : object(std::move(object_arg)), name(std::move(name_arg)), value(std::move(value_arg))
    {}

    void accept(Visitor& visitor) const override
    { visitor.visit_set_expr(*this); }

    std::unique_ptr<Expr> object;
    Token name;
    std::unique_ptr<Expr> value;
  };


  struct Super : public Expr
  {
    Super(Token keyword_arg, Token method_arg)
        : keyword(std::move(keyword_arg)), method(std::move(method_arg))
    {}

    void accept(Visitor& visitor) const override
    { visitor.visit_super_expr(*this); }

    Token keyword;
    Token method;
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


  struct Unary : public Expr
  {
    Unary(Token op_arg, std::unique_ptr<Expr> right_arg)
        : op(std::move(op_arg)), right(std::move(right_arg))
    {}

    void accept(Visitor& visitor) const override
    { visitor.visit_unary_expr(*this); }

    Token op;
    std::unique_ptr<Expr> right;
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

}

#endif //LOXX_EXPR_HPP