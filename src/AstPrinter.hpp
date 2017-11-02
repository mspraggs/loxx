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

#ifndef LOXX_ASTPRINTER_HPP
#define LOXX_ASTPRINTER_HPP

#include <sstream>

#include "Expressions.hpp"


namespace loxx
{
  class AstPrinter : public Visitor
  {
  public:
    void visitUnaryExpr(const Unary& expr) override;

    void visitBinaryExpr(const Binary& expr) override;

    void visitLiteralExpr(const Literal& expr) override;

    void visitGroupingExpr(const Grouping& expr) override;

    std::string print(const Expr& expr);

  private:
    template <typename... Exprs>
    void paranthesise(const std::string& name, const Exprs&... exprs);

    template <typename... Exprs>
    void iterate_exprs(const Expr& expr0, const Exprs&... exprs);

    void iterate_exprs(const Expr& expr);

    std::stringstream stream_;
  };


  template<typename... Exprs>
  void AstPrinter::paranthesise(const std::string& name, const Exprs& ... exprs)
  {
    stream_ << '(' << name;
    iterate_exprs(exprs...);
    stream_ << ')';
  }


  template<typename... Exprs>
  void AstPrinter::iterate_exprs(const Expr& expr0, const Exprs&... exprs)
  {
    iterate_exprs(expr0);
    iterate_exprs(exprs...);
  }


  void AstPrinter::iterate_exprs(const Expr& expr)
  {
    stream_ << ' ';
    expr.accept(*this);
  }
}

#endif //LOXX_ASTPRINTER_HPP
