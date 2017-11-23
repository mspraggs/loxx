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

  class Var;
  class While;
  class Print;
  class Expression;
  class Block;
  class If;

  class Stmt
  {
  public:
    virtual ~Stmt() = default;

    class Visitor
    {
    public:
      virtual void visit_var_stmt(const Var& stmt) = 0;
      virtual void visit_while_stmt(const While& stmt) = 0;
      virtual void visit_print_stmt(const Print& stmt) = 0;
      virtual void visit_expression_stmt(const Expression& stmt) = 0;
      virtual void visit_block_stmt(const Block& stmt) = 0;
      virtual void visit_if_stmt(const If& stmt) = 0;
    };

    virtual void accept(Visitor& visitor) const {}
  };


  class Var : public Stmt
  {
  public:
    Var(Token name, std::unique_ptr<Expr> initialiser)
        : name_(std::move(name)), initialiser_(std::move(initialiser))
    {}

    void accept(Visitor& visitor) const override
    { visitor.visit_var_stmt(*this); }

    const Token& name() const { return name_; }
    const Expr& initialiser() const { if (initialiser_ == nullptr) throw std::out_of_range("Member initialiser_ contains nullptr!"); return *initialiser_; }

  private:
    Token name_;
    std::unique_ptr<Expr> initialiser_;
  };


  class While : public Stmt
  {
  public:
    While(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> body)
        : condition_(std::move(condition)), body_(std::move(body))
    {}

    void accept(Visitor& visitor) const override
    { visitor.visit_while_stmt(*this); }

    const Expr& condition() const { if (condition_ == nullptr) throw std::out_of_range("Member condition_ contains nullptr!"); return *condition_; }
    const Stmt& body() const { if (body_ == nullptr) throw std::out_of_range("Member body_ contains nullptr!"); return *body_; }

  private:
    std::unique_ptr<Expr> condition_;
    std::unique_ptr<Stmt> body_;
  };


  class Print : public Stmt
  {
  public:
    Print(std::unique_ptr<Expr> expression)
        : expression_(std::move(expression))
    {}

    void accept(Visitor& visitor) const override
    { visitor.visit_print_stmt(*this); }

    const Expr& expression() const { if (expression_ == nullptr) throw std::out_of_range("Member expression_ contains nullptr!"); return *expression_; }

  private:
    std::unique_ptr<Expr> expression_;
  };


  class Expression : public Stmt
  {
  public:
    Expression(std::unique_ptr<Expr> expression)
        : expression_(std::move(expression))
    {}

    void accept(Visitor& visitor) const override
    { visitor.visit_expression_stmt(*this); }

    const Expr& expression() const { if (expression_ == nullptr) throw std::out_of_range("Member expression_ contains nullptr!"); return *expression_; }

  private:
    std::unique_ptr<Expr> expression_;
  };


  class Block : public Stmt
  {
  public:
    Block(std::vector<std::unique_ptr<Stmt>> statements)
        : statements_(std::move(statements))
    {}

    void accept(Visitor& visitor) const override
    { visitor.visit_block_stmt(*this); }

    const std::vector<std::unique_ptr<Stmt>>& statements() const { return statements_; }

  private:
    std::vector<std::unique_ptr<Stmt>> statements_;
  };


  class If : public Stmt
  {
  public:
    If(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> then_branch, std::unique_ptr<Stmt> else_branch)
        : condition_(std::move(condition)), then_branch_(std::move(then_branch)), else_branch_(std::move(else_branch))
    {}

    void accept(Visitor& visitor) const override
    { visitor.visit_if_stmt(*this); }

    const Expr& condition() const { if (condition_ == nullptr) throw std::out_of_range("Member condition_ contains nullptr!"); return *condition_; }
    const Stmt& then_branch() const { if (then_branch_ == nullptr) throw std::out_of_range("Member then_branch_ contains nullptr!"); return *then_branch_; }
    const Stmt& else_branch() const { if (else_branch_ == nullptr) throw std::out_of_range("Member else_branch_ contains nullptr!"); return *else_branch_; }

  private:
    std::unique_ptr<Expr> condition_;
    std::unique_ptr<Stmt> then_branch_;
    std::unique_ptr<Stmt> else_branch_;
  };

}

#endif //LOXX_STMT_HPP