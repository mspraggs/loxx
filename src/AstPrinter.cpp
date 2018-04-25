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
    paranthesise(expr.op.lexeme(), {expr.right.get()});
  }


  void AstPrinter::visit_assign_expr(const Assign& expr)
  {
    const std::string name = "setq " + expr.name.lexeme();
    paranthesise(name, {expr.value.get()});
  }


  void AstPrinter::visit_binary_expr(const Binary& expr)
  {
    paranthesise(expr.op.lexeme(), {expr.left.get(), expr.right.get()});
  }


  void AstPrinter::visit_literal_expr(const Literal& expr)
  {
    if (holds_alternative<double>(expr.value)) {
      stream_ << get<double>(expr.value);
    }
    else if (holds_alternative<std::string>(expr.value)) {
      stream_ << '"' << get<std::string>(expr.value) << '"';
    }
    else if (holds_alternative<bool>(expr.value)) {
      stream_ << (get<bool>(expr.value) ? "true" : "false");
    }
    else {
      stream_ << "nil";
    }
  }


  void AstPrinter::visit_logical_expr(const Logical& expr)
  {
    const std::string name = expr.op.type() == TokenType::Or ? "or" : "and";
    paranthesise(name, {expr.left.get(), expr.right.get()});
  }


  void AstPrinter::visit_grouping_expr(const Grouping& expr)
  {
    paranthesise("group", {expr.expression.get()});
  }


  void AstPrinter::visit_variable_expr(const Variable& expr)
  {
    stream_ << expr.name.lexeme();
  }


  void AstPrinter::visit_call_expr(const Call& expr)
  {
    stream_ << '(';
    expr.callee->accept(*this);

    for (const auto& arg : expr.arguments) {
      stream_ << ' ';
      arg->accept(*this);
    }

    stream_ << ')';
  }


  void AstPrinter::visit_get_expr(const Get& expr)
  {
    paranthesise("get " + expr.name.lexeme(), {expr.object.get()});
  }


  void AstPrinter::visit_set_expr(const Set& expr)
  {
    paranthesise("set " + expr.name.lexeme(),
                 {expr.object.get(), expr.value.get()});
  }


  void AstPrinter::visit_this_expr(const This& expr)
  {
    stream_ << expr.keyword.lexeme();
  }


  void AstPrinter::visit_super_expr(const Super& expr)
  {
    stream_ << expr.keyword.lexeme() << ' '
            << expr.method.lexeme();
  }


  void AstPrinter::visit_print_stmt(const Print& stmt)
  {
    paranthesise("write-line", {stmt.expression.get()});
  }


  void AstPrinter::visit_return_stmt(const Return& stmt)
  {
    paranthesise("return", {stmt.value.get()});
  }


  void AstPrinter::visit_var_stmt(const Var& stmt)
  {
    const std::string name = "defvar " + stmt.name.lexeme();

    if (stmt.initialiser != nullptr) {
      paranthesise(name, {stmt.initialiser.get()});
    }
    else {
      paranthesise(name, {});
    }
  }


  void AstPrinter::visit_while_stmt(const While& stmt)
  {
    stream_ << "(while ";
    stmt.condition->accept(*this);
    stream_ << ' ';
    stmt.body->accept(*this);
    stream_ << ')';
  }


  void AstPrinter::visit_if_stmt(const If& stmt)
  {
    stream_ << "(if ";
    stmt.condition->accept(*this);
    stream_ << ' ';
    stmt.then_branch->accept(*this);
    stream_ << ' ';
    if (stmt.else_branch != nullptr) {
      stmt.else_branch->accept(*this);
    }
    stream_ << ')';
  }


  void AstPrinter::visit_expression_stmt(const Expression& stmt)
  {
    stmt.expression->accept(*this);
  }


  void AstPrinter::visit_function_stmt(const Function& func)
  {
    stream_ << "(defun " << func.name.lexeme() << " ( ";

    for (const auto& arg : func.parameters) {
      stream_ << arg.lexeme() << ' ';
    }
    stream_ << ") (block\n";
    set_indent(indent_level_ + 1);
    print(func.body);
    set_indent(indent_level_ - 1);
    stream_ << indent_ << ')';
  }


  void AstPrinter::visit_block_stmt(const Block& stmt)
  {
    stream_ << "(block\n";
    set_indent(indent_level_ + 1);
    print(stmt.statements);
    set_indent(indent_level_ - 1);
    stream_ << indent_ << ')';
  }


  void AstPrinter::visit_class_stmt(const Class& stmt)
  {
    stream_ << "(declass " << stmt.name.lexeme() << " (\n";
    set_indent(indent_level_ + 1);
    print(stmt.methods);
    set_indent(indent_level_ - 1);
    stream_ << indent_ << ')';
  }


  void AstPrinter::paranthesise(const std::string& name,
                                std::initializer_list<raw_ptr<const Expr>> exprs)
  {
    stream_ << '(' << name;

    for (const auto expr : exprs) {
      if (expr == nullptr) {
        continue;
      }
      stream_ << ' ';
      expr->accept(*this);
    }

    stream_ << ')';
  }


  void AstPrinter::set_indent(const unsigned int indent)
  {
    indent_level_ = indent;
    indent_ = std::string(indent_level_ * 2, ' ');
  }
}
