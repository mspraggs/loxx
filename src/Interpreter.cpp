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

#include <chrono>
#include <iostream>
#include <sstream>

#include "Callable.hpp"
#include "ClassDef.hpp"
#include "ClassInstance.hpp"
#include "Interpreter.hpp"
#include "logging.hpp"
#include "NativeCallable.hpp"


namespace loxx
{
  Interpreter::Interpreter(const bool in_repl)
      : in_repl_(in_repl), print_result_(false), stack_(4096),
        environment_(new Environment), globals_(environment_)
  {
    using Fn = Generic (*) (const Interpreter&, const std::vector<Generic>&);
    Fn fn = [] (const Interpreter&, const std::vector<Generic>&) {
      using namespace std::chrono;
      const auto millis =
          system_clock::now().time_since_epoch() / milliseconds(1);

      return Generic(static_cast<double>(millis) / 1000.0);
    };

    globals_->define(
        "clock",
        Generic(std::shared_ptr<Callable>(
            new NativeCallable<Fn>(std::move(fn), 0)))
    );
  }


  void Interpreter::interpret(
      const std::vector<std::shared_ptr<Stmt>>& statements)
  {
    try {
      bool single_expr = statements.size() == 1 and
                         typeid(*statements[0]) == typeid(Expression);
      print_result_ = single_expr and in_repl_;

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
    evaluate(*stmt.expression);
    const auto result = stack_.pop();

    if (print_result_) {
      std::cout << "= " << stringify(result) << '\n';
    }
  }


  void Interpreter::visit_function_stmt(const Function& stmt)
  {
    std::shared_ptr<Callable> func =
        std::make_shared<FuncCallable>(stmt, environment_, false);
    environment_->define(stmt.name.lexeme(), func);
  }


  void Interpreter::visit_if_stmt(const If& stmt)
  {
    evaluate(*stmt.condition);
    if (is_truthy(stack_.pop())) {
      execute(*stmt.then_branch);
    }
    else if (stmt.else_branch != nullptr) {
      execute(*stmt.else_branch);
    }
  }


  void Interpreter::visit_print_stmt(const Print& stmt)
  {
    evaluate(*stmt.expression);
    std::cout << stringify(stack_.pop()) << std::endl;
  }


  void Interpreter::visit_return_stmt(const Return& stmt)
  {
    auto value = [this, &stmt] () {
      try {
        evaluate(*stmt.value);
        return stack_.pop();
      }
      catch (const std::out_of_range& e) {
        return Generic(nullptr);
      }
    }();

    throw Returner(std::move(value));
  }


  void Interpreter::visit_var_stmt(const Var& stmt)
  {
    auto value = [this, &stmt] () {
      if (stmt.initialiser != nullptr) {
        evaluate(*stmt.initialiser);
        return stack_.pop();
      }
      return Generic(nullptr);
    }();

    environment_->define(stmt.name.lexeme(), std::move(value));
  }


  void Interpreter::visit_while_stmt(const While& stmt)
  {
    evaluate(*stmt.condition);

    while (is_truthy(stack_.pop())) {
      execute(*stmt.body);
      evaluate(*stmt.condition);
    }
  }


  void Interpreter::visit_block_stmt(const Block& stmt)
  {
    try {
      execute_block(stmt.statements,
                    std::make_shared<Environment>(environment_));
    }
    catch (const RuntimeError& e) {
    }
  }


  void Interpreter::visit_class_stmt(const Class& stmt)
  {
    environment_->define(stmt.name.lexeme(), nullptr);

    std::shared_ptr<ClassDef> superclass;
    if (stmt.superclass != nullptr) {
      evaluate(*stmt.superclass);
      auto superclass_obj = stack_.pop();
      
      if (superclass_obj.has_type<std::shared_ptr<Callable>>()) {
        auto superclass_ptr =
          generic_cast<std::shared_ptr<Callable>>(superclass_obj);
        superclass = std::dynamic_pointer_cast<ClassDef>(superclass_ptr);
      }
      
      if (superclass == nullptr) {
        throw RuntimeError(stmt.name, "Superclass must be a class");
      }

      environment_ = std::make_shared<Environment>(std::move(environment_));
      environment_->define("superclass", superclass);
    }

    std::unordered_map<std::string, Generic> methods;
    for (const auto& method : stmt.methods) {
      const bool is_initialiser = method->name.lexeme() == "init";
      std::shared_ptr<Callable> callable =
          std::make_shared<FuncCallable>(*method, environment_, is_initialiser);
      methods.emplace(method->name.lexeme(), callable);
    }
    
    std::shared_ptr<Callable> cls =
        std::make_shared<ClassDef>(stmt.name.lexeme(), std::move(superclass),
                                   std::move(methods));

    if (superclass != nullptr) {
      environment_ = environment_->release_enclosing();
    }

    environment_->assign(stmt.name, cls);
  }


  void Interpreter::visit_assign_expr(const Assign& expr)
  {
    evaluate(*expr.value);

    const bool have_distance = locals_.count(&expr) != 0;

    if (have_distance) {
      const std::size_t distance = locals_[&expr];
      environment_->assign_at(distance, expr.name, stack_.top());
    }
    else {
      globals_->assign(expr.name, stack_.top());
    }
  }


  void Interpreter::visit_unary_expr(const Unary& expr)
  {
    evaluate(*expr.right);

    const auto value = stack_.pop();

    switch (expr.op.type()) {
    case TokenType::Bang:
      stack_.push(not is_truthy(value));
      break;
    case TokenType::Minus:
      check_number_operand(expr.op, value);
      stack_.push(-generic_cast<double>(value));
      break;
    default:
      break;
    }
  }


  void Interpreter::visit_binary_expr(const Binary& expr)
  {
    evaluate(*expr.left);
    const auto left = stack_.pop();

    evaluate(*expr.right);
    const auto right = stack_.pop();

    switch (expr.op.type()) {
    case TokenType::BangEqual:
      stack_.push(not is_equal(left, right));
      break;
    case TokenType::EqualEqual:
      stack_.push(is_equal(left, right));
      break;
    case TokenType::Greater:
      check_number_operands(expr.op, left, right);
      stack_.push(generic_cast<double>(left) > generic_cast<double>(right));
      break;
    case TokenType::GreaterEqual:
      check_number_operands(expr.op, left, right);
      stack_.push(generic_cast<double>(left) >= generic_cast<double>(right));
      break;
    case TokenType::Less:
      check_number_operands(expr.op, left, right);
      stack_.push(generic_cast<double>(left) < generic_cast<double>(right));
      break;
    case TokenType::LessEqual:
      check_number_operands(expr.op, left, right);
      stack_.push(generic_cast<double>(left) <= generic_cast<double>(right));
      break;
    case TokenType::Minus:
      check_number_operands(expr.op, left, right);
      stack_.push(generic_cast<double>(left) - generic_cast<double>(right));
      break;
    case TokenType::Plus:
      if (left.has_type<double>() and right.has_type<double>()) {
        stack_.push(generic_cast<double>(left) + generic_cast<double>(right));
      }
      else if (left.has_type<std::string>() and right.has_type<std::string>()) {
        stack_.push(generic_cast<std::string>(left) +
                    generic_cast<std::string>(right));
      }
      else {
        throw RuntimeError(
            expr.op, "Binary operands must be two numbers or two strings.");
      }
      break;
    case TokenType::Slash:
      check_number_operands(expr.op, left, right);
      stack_.push(generic_cast<double>(left) / generic_cast<double>(right));
      break;
    case TokenType::Star:
      check_number_operands(expr.op, left, right);
      stack_.push(generic_cast<double>(left) * generic_cast<double>(right));
      break;
    default:
      break;
    }
  }


  void Interpreter::visit_call_expr(const Call& expr)
  {
    evaluate(*expr.callee);
    auto callee = stack_.pop();

    std::vector<Generic> arguments;
    for (const auto& argument : expr.arguments) {
      evaluate(*argument);
      arguments.push_back(stack_.pop());
    }

    if (not callee.has_type<std::shared_ptr<Callable>>()) {
      throw RuntimeError(expr.paren, "Can only call functions and classes.");
    }

    auto& function = generic_cast<std::shared_ptr<Callable>>(callee);
    if (arguments.size() != function->arity()) {
      std::stringstream ss;
      ss << "Expected " << function->arity() << " arguments but got "
         << arguments.size() << '.';
      throw RuntimeError(expr.paren, ss.str());
    }
    stack_.push(function->call(*this, arguments));
  }


  void Interpreter::visit_get_expr(const Get& expr)
  {
    evaluate(*expr.object);
    const auto object = stack_.pop();
    if (object.has_type<std::shared_ptr<ClassInstance>>()) {
      stack_.push(generic_cast<std::shared_ptr<ClassInstance>>(object)->get(expr.name));
      return;
    }

    throw RuntimeError(expr.name, "Only instances have properties.");
  }


  void Interpreter::visit_literal_expr(const Literal& expr)
  {
    stack_.push(expr.value);
  }


  void Interpreter::visit_logical_expr(const Logical& expr)
  {
    evaluate(*expr.left);
    auto left = stack_.pop();

    if (expr.op.type() == TokenType::Or) {
      if (is_truthy(left)) {
        stack_.push(std::move(left));
        return;
      }
    }
    else {
      if (not is_truthy(left)) {
        stack_.push(std::move(left));
        return;
      }
    }

    evaluate(*expr.right);
  }


  void Interpreter::visit_set_expr(const Set& expr)
  {
    evaluate(*expr.object);
    auto object = stack_.pop();

    if (not object.has_type<std::shared_ptr<ClassInstance>>()) {
      throw RuntimeError(expr.name, "Only instances have fields.");
    }

    evaluate(*expr.value);
    generic_cast<std::shared_ptr<ClassInstance>>(object)->set(expr.name, stack_.top());
  }


  void Interpreter::visit_super_expr(const Super& expr)
  {
    const int distance = locals_[&expr];
    const auto super_obj = generic_cast<std::shared_ptr<Callable>>(
        environment_->get_at(distance, "super"));
    const auto superclass = std::static_pointer_cast<ClassDef>(super_obj);

    auto object = generic_cast<std::shared_ptr<ClassInstance>>(
        environment_->get_at(distance - 1, "this"));

    auto method = superclass->find_method(std::move(object),
                                          expr.method.lexeme());

    if (method.has_type<decltype(nullptr)>()) {
      throw RuntimeError(expr.method,
                         "Undefined property '" + expr.method.lexeme() + "'.");
    }

    stack_.push(std::move(method));
  }


  void Interpreter::visit_this_expr(const This& expr)
  {
    stack_.push(lookup_variable(expr.keyword, expr));
  }


  void Interpreter::visit_grouping_expr(const Grouping& expr)
  {
    evaluate(*expr.expression);
  }


  void Interpreter::visit_variable_expr(const Variable& expr)
  {
    stack_.push(lookup_variable(expr.name, expr));
  }


  void Interpreter::evaluate(const Expr& expr)
  {
    expr.accept(*this);
  }


  void Interpreter::execute(const Stmt& stmt)
  {
    stmt.accept(*this);
  }


  void Interpreter::resolve(const Expr& expr, const std::size_t depth)
  {
    locals_[&expr] = depth;
  }


  void Interpreter::execute_block(
      const std::vector<std::shared_ptr<Stmt>>& statements,
      std::shared_ptr<Environment> environment)
  {
    auto previous = environment_;
    environment_ = std::move(environment);

    std::exception_ptr exception;

    try {
      for (const auto& stmt : statements) {
        execute(*stmt);
      }
    }
    catch (const std::exception& e) {
      exception = std::current_exception();
    }

    environment_ = previous;

    if (exception) {
      std::rethrow_exception(exception);
    }
  }


  bool Interpreter::is_truthy(const Generic& value)
  {
    if (value.has_type<std::nullptr_t>()) {
      return false;
    }
    if (value.has_type<bool>()) {
      return generic_cast<bool>(value);
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
      ss << generic_cast<double>(generic);
      return ss.str();
    }
    if (generic.has_type<bool>()) {
      return generic_cast<bool>(generic) ? "true" : "false";
    }
    if (generic.has_type<std::string>()) {
      return generic_cast<std::string>(generic);
    }
    if (generic.has_type<std::shared_ptr<Callable>>()) {
      // TODO: Find a more elegant way to achieve ths...
      const auto ptr = std::dynamic_pointer_cast<ClassDef>(
          generic_cast<std::shared_ptr<Callable>>(generic));
      if (ptr != nullptr) {
        return ptr->name();
      }
    }
    if (generic.has_type<std::shared_ptr<ClassInstance>>()) {
      return generic_cast<std::shared_ptr<ClassInstance>>(generic)->cls().name() +
          " instance";
    }
    return "";
  }


  Generic Interpreter::lookup_variable(const Token& name,
                                       const Expr& expr) const
  {
    const bool have_distance = locals_.count(&expr) != 0;

    if (have_distance) {
      return environment_->get_at(locals_.at(&expr), name.lexeme());
    }
    else {
      return globals_->get(name);
    }
  }
}
