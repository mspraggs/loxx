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

#include <iostream>
#include "AstPrinter.hpp"


namespace loxx
{
  void AstPrinter::visitUnaryExpr(const Unary& expr)
  {
    paranthesise(expr.op().lexeme(), {&expr.right()});
  }


  void AstPrinter::visitBinaryExpr(const Binary& expr)
  {
    paranthesise(expr.op().lexeme(), {&expr.left(), &expr.right()});
  }


  void AstPrinter::visitTernaryExpr(const Ternary& expr)
  {
    paranthesise(expr.op().lexeme(),
                 {&expr.first(), &expr.second(), &expr.third()});
  }


  void AstPrinter::visitLiteralExpr(const Literal& expr)
  {
    if (expr.value().has_type<double>()) {
      stream_ << expr.value().get<double>();
    }
    else if (expr.value().has_type<std::string>()) {
      stream_ << expr.value().get<std::string>();
    }
    else if (expr.value().has_type<bool>()) {
      stream_ << (expr.value().get<bool>() ? "true" : "false");
    }
    else {
      stream_ << "nil";
    }
  }


  void AstPrinter::visitGroupingExpr(const Grouping& expr)
  {
    paranthesise("group", {&expr.expression()});
  }


  std::string AstPrinter::print(const Expr& expr)
  {
    expr.accept(*this);
    return stream_.str();
  }


  void AstPrinter::paranthesise(const std::string& name,
                                std::initializer_list<const Expr*> exprs)
  {
    stream_ << '(' << name;

    for (const auto expr : exprs) {
      stream_ << ' ';
      expr->accept(*this);
    }

    stream_ << ')';
  }
}
