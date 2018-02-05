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

#include "Generic.hpp"
#include "Token.hpp"
#include "Expr.hpp"


namespace loxx
{


  struct Break;
  struct Function;
  struct While;
  struct Return;
  struct Var;
  struct Print;
  struct Expression;
  struct Class;
  struct Block;
  struct If;

  struct Stmt
  {
    virtual ~Stmt() = default;

    class Visitor
    {
    public:
      virtual void visit_break_stmt(const Break& stmt) = 0;
      virtual void visit_function_stmt(const Function& stmt) = 0;
      virtual void visit_while_stmt(const While& stmt) = 0;
      virtual void visit_return_stmt(const Return& stmt) = 0;
      virtual void visit_var_stmt(const Var& stmt) = 0;
      virtual void visit_print_stmt(const Print& stmt) = 0;
      virtual void visit_expression_stmt(const Expression& stmt) = 0;
      virtual void visit_class_stmt(const Class& stmt) = 0;
      virtual void visit_block_stmt(const Block& stmt) = 0;
      virtual void visit_if_stmt(const If& stmt) = 0;
    };

    virtual void accept(Visitor&) const {}
  };


  struct Break : public Stmt
  {
    Break()
    {}

    void accept(Visitor& visitor) const override
    { visitor.visit_break_stmt(*this); }

    
  };


  struct Function : public Stmt
  {
    Function(Token name_arg, std::vector<Token> parameters_arg, std::vector<std::shared_ptr<Stmt>> body_arg)
        : name(std::move(name_arg)), parameters(std::move(parameters_arg)), body(std::move(body_arg))
    {}

    void accept(Visitor& visitor) const override
    { visitor.visit_function_stmt(*this); }

    Token name;
    std::vector<Token> parameters;
    std::vector<std::shared_ptr<Stmt>> body;
  };


  struct While : public Stmt
  {
    While(std::shared_ptr<Expr> condition_arg, std::shared_ptr<Stmt> body_arg)
        : condition(std::move(condition_arg)), body(std::move(body_arg))
    {}

    void accept(Visitor& visitor) const override
    { visitor.visit_while_stmt(*this); }

    std::shared_ptr<Expr> condition;
    std::shared_ptr<Stmt> body;
  };


  struct Return : public Stmt
  {
    Return(Token keyword_arg, std::shared_ptr<Expr> value_arg)
        : keyword(std::move(keyword_arg)), value(std::move(value_arg))
    {}

    void accept(Visitor& visitor) const override
    { visitor.visit_return_stmt(*this); }

    Token keyword;
    std::shared_ptr<Expr> value;
  };


  struct Var : public Stmt
  {
    Var(Token name_arg, std::shared_ptr<Expr> initialiser_arg)
        : name(std::move(name_arg)), initialiser(std::move(initialiser_arg))
    {}

    void accept(Visitor& visitor) const override
    { visitor.visit_var_stmt(*this); }

    Token name;
    std::shared_ptr<Expr> initialiser;
  };


  struct Print : public Stmt
  {
    Print(std::shared_ptr<Expr> expression_arg)
        : expression(std::move(expression_arg))
    {}

    void accept(Visitor& visitor) const override
    { visitor.visit_print_stmt(*this); }

    std::shared_ptr<Expr> expression;
  };


  struct Expression : public Stmt
  {
    Expression(std::shared_ptr<Expr> expression_arg)
        : expression(std::move(expression_arg))
    {}

    void accept(Visitor& visitor) const override
    { visitor.visit_expression_stmt(*this); }

    std::shared_ptr<Expr> expression;
  };


  struct Class : public Stmt
  {
    Class(Token name_arg, std::vector<std::shared_ptr<Function>> bound_methods_arg, std::vector<std::shared_ptr<Function>> static_methods_arg)
        : name(std::move(name_arg)), bound_methods(std::move(bound_methods_arg)), static_methods(std::move(static_methods_arg))
    {}

    void accept(Visitor& visitor) const override
    { visitor.visit_class_stmt(*this); }

    Token name;
    std::vector<std::shared_ptr<Function>> bound_methods;
    std::vector<std::shared_ptr<Function>> static_methods;
  };


  struct Block : public Stmt
  {
    Block(std::vector<std::shared_ptr<Stmt>> statements_arg)
        : statements(std::move(statements_arg))
    {}

    void accept(Visitor& visitor) const override
    { visitor.visit_block_stmt(*this); }

    std::vector<std::shared_ptr<Stmt>> statements;
  };


  struct If : public Stmt
  {
    If(std::shared_ptr<Expr> condition_arg, std::shared_ptr<Stmt> then_branch_arg, std::shared_ptr<Stmt> else_branch_arg)
        : condition(std::move(condition_arg)), then_branch(std::move(then_branch_arg)), else_branch(std::move(else_branch_arg))
    {}

    void accept(Visitor& visitor) const override
    { visitor.visit_if_stmt(*this); }

    std::shared_ptr<Expr> condition;
    std::shared_ptr<Stmt> then_branch;
    std::shared_ptr<Stmt> else_branch;
  };

}

#endif //LOXX_STMT_HPP