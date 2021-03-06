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

#ifndef LOXX_STMT_HPP
#define LOXX_STMT_HPP

#include <memory>
#include <vector>

#include "globals.hpp"
#include "Token.hpp"
#include "Expr.hpp"


namespace loxx
{

  struct Block;
  struct Class;
  struct Expression;
  struct Function;
  struct If;
  struct Print;
  struct Return;
  struct Var;
  struct While;

  struct Stmt
  {
    virtual ~Stmt() = default;

    class Visitor
    {
    public:
      virtual void visit_block_stmt(const Block& stmt) = 0;
      virtual void visit_class_stmt(const Class& stmt) = 0;
      virtual void visit_expression_stmt(const Expression& stmt) = 0;
      virtual void visit_function_stmt(const Function& stmt) = 0;
      virtual void visit_if_stmt(const If& stmt) = 0;
      virtual void visit_print_stmt(const Print& stmt) = 0;
      virtual void visit_return_stmt(const Return& stmt) = 0;
      virtual void visit_var_stmt(const Var& stmt) = 0;
      virtual void visit_while_stmt(const While& stmt) = 0;
    };

    virtual void accept(Visitor&) const {}
  };


  struct Block : public Stmt
  {
    Block(std::vector<std::unique_ptr<Stmt>> statements_arg)
        : statements(std::move(statements_arg))
    {}

    void accept(Visitor& visitor) const override
    { visitor.visit_block_stmt(*this); }

    std::vector<std::unique_ptr<Stmt>> statements;
  };


  struct Class : public Stmt
  {
    Class(Token name_arg, std::unique_ptr<Expr> superclass_arg, std::vector<std::unique_ptr<Function>> methods_arg)
        : name(std::move(name_arg)), superclass(std::move(superclass_arg)), methods(std::move(methods_arg))
    {}

    void accept(Visitor& visitor) const override
    { visitor.visit_class_stmt(*this); }

    Token name;
    std::unique_ptr<Expr> superclass;
    std::vector<std::unique_ptr<Function>> methods;
  };


  struct Expression : public Stmt
  {
    Expression(std::unique_ptr<Expr> expression_arg)
        : expression(std::move(expression_arg))
    {}

    void accept(Visitor& visitor) const override
    { visitor.visit_expression_stmt(*this); }

    std::unique_ptr<Expr> expression;
  };


  struct Function : public Stmt
  {
    Function(Token name_arg, std::vector<Token> parameters_arg, std::vector<std::unique_ptr<Stmt>> body_arg)
        : name(std::move(name_arg)), parameters(std::move(parameters_arg)), body(std::move(body_arg))
    {}

    void accept(Visitor& visitor) const override
    { visitor.visit_function_stmt(*this); }

    Token name;
    std::vector<Token> parameters;
    std::vector<std::unique_ptr<Stmt>> body;
  };


  struct If : public Stmt
  {
    If(std::unique_ptr<Expr> condition_arg, std::unique_ptr<Stmt> then_branch_arg, std::unique_ptr<Stmt> else_branch_arg)
        : condition(std::move(condition_arg)), then_branch(std::move(then_branch_arg)), else_branch(std::move(else_branch_arg))
    {}

    void accept(Visitor& visitor) const override
    { visitor.visit_if_stmt(*this); }

    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> then_branch;
    std::unique_ptr<Stmt> else_branch;
  };


  struct Print : public Stmt
  {
    Print(std::unique_ptr<Expr> expression_arg)
        : expression(std::move(expression_arg))
    {}

    void accept(Visitor& visitor) const override
    { visitor.visit_print_stmt(*this); }

    std::unique_ptr<Expr> expression;
  };


  struct Return : public Stmt
  {
    Return(Token keyword_arg, std::unique_ptr<Expr> value_arg)
        : keyword(std::move(keyword_arg)), value(std::move(value_arg))
    {}

    void accept(Visitor& visitor) const override
    { visitor.visit_return_stmt(*this); }

    Token keyword;
    std::unique_ptr<Expr> value;
  };


  struct Var : public Stmt
  {
    Var(Token name_arg, std::unique_ptr<Expr> initialiser_arg)
        : name(std::move(name_arg)), initialiser(std::move(initialiser_arg))
    {}

    void accept(Visitor& visitor) const override
    { visitor.visit_var_stmt(*this); }

    Token name;
    std::unique_ptr<Expr> initialiser;
  };


  struct While : public Stmt
  {
    While(std::unique_ptr<Expr> condition_arg, std::unique_ptr<Stmt> body_arg)
        : condition(std::move(condition_arg)), body(std::move(body_arg))
    {}

    void accept(Visitor& visitor) const override
    { visitor.visit_while_stmt(*this); }

    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> body;
  };

}

#endif //LOXX_STMT_HPP