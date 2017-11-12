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

#include "Generic.hpp"
#include "Token.hpp"
#include "Expr.hpp"


namespace loxx
{

  class Print;
  class Expression;

  class Stmt
  {
  public:
    virtual ~Stmt() = default;

    class Visitor
    {
    public:
      virtual void visit_print_stmt(const Print& expr) = 0;
      virtual void visit_expression_stmt(const Expression& expr) = 0;
    };

    virtual void accept(Visitor& visitor) const {}
  };


  class Print : public Stmt
  {
  public:
    Print(std::unique_ptr<Expr> expression)
        : expression_(std::move(expression))
    {}

    void accept(Visitor& visitor) const override
    { visitor.visit_print_stmt(*this); }

    const Expr& expression() const { return *expression_; }

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

    const Expr& expression() const { return *expression_; }

  private:
    std::unique_ptr<Expr> expression_;
  };

}

#endif //LOXX_STMT_HPP