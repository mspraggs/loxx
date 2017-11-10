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
  void Interpreter::interpret(const Expr& expr)
  {
    try {
      evaluate(expr);
      if (stack_.size() > 0) {
        std::cout << stringify(pop_top()) << std::endl;
      }
    }
    catch (const RuntimeError& e) {
      runtime_error(e);
    }
  }


  void Interpreter::visit_unary_expr(const Unary& expr)
  {
    evaluate(expr.right());

    const auto value = pop_top();

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
    const auto left = pop_top();

    evaluate(expr.right());
    const auto right = pop_top();

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
        stack_.push(Generic(left.get<std::string>() + right.get<std::string>()));
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


  void Interpreter::evaluate(const Expr& expr)
  {
    expr.accept(*this);
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


  Generic Interpreter::pop_top()
  {
    const auto ret = stack_.top();
    stack_.pop();
    return ret;
  }
}