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
 * Created by Matt Spraggs on 05/11/17.
 */

#include <iostream>
#include <sstream>

#include "Interpreter.hpp"
#include "logging.hpp"


namespace loxx
{
  Interpreter::Interpreter() : stack_(4096)
  {
  }


  void Interpreter::interpret(
      const std::vector<std::unique_ptr<Stmt>>& statements)
  {
    try {
      for (const auto& stmt : statements) {
        execute(*stmt);
      }
    }
    catch (const RuntimeError& e) {
      runtime_error(e);
    }
  }


  void Interpreter::visit_expression_stmt(const Expression& stmt)
  {
    evaluate(stmt.expression());
    stack_.pop();
  }


  void Interpreter::visit_print_stmt(const Print& stmt)
  {
    evaluate(stmt.expression());
    std::cout << stringify(stack_.pop()) << std::endl;
  }


  void Interpreter::visit_var_stmt(const Var& stmt)
  {
    auto value = [this, &stmt] () {
      try {
        evaluate(stmt.initialiser());
        return stack_.pop();
      }
      catch (const std::out_of_range& e) {
        return Generic(nullptr);
      }
    }();

    environment_.define(stmt.name().lexeme(), std::move(value));
  }


  void Interpreter::visit_assign_expr(const Assign& expr)
  {
    evaluate(expr.value());

    environment_.assign(expr.name(), stack_.top());
  }


  void Interpreter::visit_unary_expr(const Unary& expr)
  {
    evaluate(expr.right());

    const auto value = stack_.pop();

    switch (expr.op().type()) {
    case TokenType::Bang:
      stack_.push(Generic(not is_truthy(value)));
      break;
    case TokenType::Minus:
      check_number_operand(expr.op(), value);
      stack_.push(Generic(-value.get<double>()));
      break;
    default:
      break;
    }
  }


  void Interpreter::visit_binary_expr(const Binary& expr)
  {
    evaluate(expr.left());
    const auto left = stack_.pop();

    evaluate(expr.right());
    const auto right = stack_.pop();

    switch (expr.op().type()) {
    case TokenType::BangEqual:
      check_number_operands(expr.op(), left, right);
      stack_.push(Generic(not is_equal(left, right)));
      break;
    case TokenType::EqualEqual:
      check_number_operands(expr.op(), left, right);
      stack_.push(Generic(is_equal(left, right)));
      break;
    case TokenType::Greater:
      check_number_operands(expr.op(), left, right);
      stack_.push(Generic(left.get<double>() > right.get<double>()));
      break;
    case TokenType::GreaterEqual:
      check_number_operands(expr.op(), left, right);
      stack_.push(Generic(left.get<double>() >= right.get<double>()));
      break;
    case TokenType::Less:
      check_number_operands(expr.op(), left, right);
      stack_.push(Generic(left.get<double>() < right.get<double>()));
      break;
    case TokenType::LessEqual:
      check_number_operands(expr.op(), left, right);
      stack_.push(Generic(left.get<double>() <= right.get<double>()));
      break;
    case TokenType::Minus:
      check_number_operands(expr.op(), left, right);
      stack_.push(Generic(left.get<double>() - right.get<double>()));
      break;
    case TokenType::Plus:
      if (left.has_type<double>() and right.has_type<double>()) {
        stack_.push(Generic(left.get<double>() + right.get<double>()));
      }
      else if (left.has_type<std::string>() and right.has_type<std::string>()) {
        stack_.push(Generic(left.get<std::string>() +
                                     right.get<std::string>()));
      }
      else {
        throw RuntimeError(
            expr.op(), "Binary operands must be two numbers or two strings.");
      }
      break;
    case TokenType::Slash:
      check_number_operands(expr.op(), left, right);
      stack_.push(Generic(left.get<double>() / right.get<double>()));
      break;
    case TokenType::Star:
      check_number_operands(expr.op(), left, right);
      stack_.push(Generic(left.get<double>() * right.get<double>()));
      break;
    default:
      break;
    }
  }


  void Interpreter::visit_literal_expr(const Literal& expr)
  {
    stack_.push(expr.value());
  }


  void Interpreter::visit_grouping_expr(const Grouping& expr)
  {
    evaluate(expr.expression());
  }


  void Interpreter::visit_variable_expr(const Variable& expr)
  {
    stack_.push(environment_.get(expr.name()));
  }


  void Interpreter::evaluate(const Expr& expr)
  {
    expr.accept(*this);
  }


  void Interpreter::execute(const Stmt& stmt)
  {
    stmt.accept(*this);
  }


  bool Interpreter::is_truthy(const Generic& value)
  {
    if (value.has_type<std::nullptr_t>()) {
      return false;
    }
    if (value.has_type<bool>()) {
      return value.get<bool>();
    }
    return true;
  }


  bool Interpreter::is_equal(const Generic& left, const Generic& right)
  {
    if (left.has_type<std::nullptr_t>() and right.has_type<std::nullptr_t>()) {
      return true;
    }
    if (left.has_type<std::nullptr_t>()) {
      return false;
    }

    return left == right;
  }


  void Interpreter::check_number_operand(const Token& op,
                                         const Generic& value) const
  {
    if (value.has_type<double>()) {
      return;
    }
    throw RuntimeError(op, "Unary operand must be a number.");
  }


  void Interpreter::check_number_operands(const Token& op,
                                          const Generic& left,
                                          const Generic& right) const
  {
    if (left.has_type<double>() and right.has_type<double>()) {
      return;
    }
    throw RuntimeError(op, "Binary operands must both be numbers.");
  }

  std::string Interpreter::stringify(const Generic& generic) const
  {
    if (generic.has_type<std::nullptr_t>()) {
      return "nil";
    }
    if (generic.has_type<double>()) {
      std::stringstream ss;
      ss << generic.get<double>();
      return ss.str();
    }
    if (generic.has_type<bool>()) {
      return generic.get<bool>() ? "true" : "false";
    }
    if (generic.has_type<std::string>()) {
      return generic.get<std::string>();
    }
    return "";
  }
}