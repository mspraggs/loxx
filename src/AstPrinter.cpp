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
  void AstPrinter::visit_unary_expr(const Unary& expr)
  {
    paranthesise(expr.op().lexeme(), {&expr.right()});
  }


  void AstPrinter::visit_assign_expr(const Assign& expr)
  {
    const std::string name = "setq " + expr.name().lexeme();
    paranthesise(name, {&expr.value()});
  }


  void AstPrinter::visit_binary_expr(const Binary& expr)
  {
    paranthesise(expr.op().lexeme(), {&expr.left(), &expr.right()});
  }


  void AstPrinter::visit_literal_expr(const Literal& expr)
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


  void AstPrinter::visit_grouping_expr(const Grouping& expr)
  {
    paranthesise("group", {&expr.expression()});
  }


  void AstPrinter::visit_variable_expr(const Variable& expr)
  {
    paranthesise(expr.name().lexeme(), {});
  }


  void AstPrinter::visit_print_stmt(const Print& expr)
  {
    paranthesise("write-line", {&expr.expression()});
  }


  void AstPrinter::visit_var_stmt(const Var& expr)
  {
    const std::string name = "defvar " + expr.name().lexeme();

    try {
      paranthesise(name, {&expr.initialiser()});
    }
    catch (const std::out_of_range& e) {
      paranthesise(name, {});
    }
  }


  void AstPrinter::visit_expression_stmt(const Expression& expr)
  {
    expr.expression().accept(*this);
  }


  std::string AstPrinter::print(
      const std::vector<std::unique_ptr<Stmt>>& statements)
  {
    for (const auto& stmt : statements) {
      stmt->accept(*this);
      stream_ << '\n';
    }
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
