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
 * Created by Matt Spraggs on 04/12/17.
 */

#include "logging.hpp"
#include "Resolver.hpp"


namespace loxx
{
  void Resolver::visit_block_stmt(const Block& stmt)
  {
    begin_scope();
    resolve(stmt.statements());
    end_scope();
  }


  void Resolver::visit_expression_stmt(const Expression& stmt)
  {
    resolve(stmt.expression());
  }


  void Resolver::visit_function_stmt(const Function& stmt)
  {
    declare(stmt.name());
    define(stmt.name());

    resolve_function(stmt, FunctionType::Function);
  }


  void Resolver::visit_if_stmt(const If& stmt)
  {
    resolve(stmt.condition());
    resolve(stmt.then_branch());

    try {
      resolve(stmt.else_branch());
    }
    catch (const std::out_of_range& e) {}
  }


  void Resolver::visit_print_stmt(const Print& stmt)
  {
    resolve(stmt.expression());
  }


  void Resolver::visit_return_stmt(const Return& stmt)
  {
    if (current_function_ == FunctionType::None) {
      error(stmt.keyword(), "Cannot return from top-level code.");
    }
    try {
      resolve(stmt.value());
    }
    catch (const std::out_of_range& e) {}
  }


  void Resolver::visit_var_stmt(const Var& stmt)
  {
    declare(stmt.name());

    try {
      resolve(stmt.initialiser());
    }
    catch (const std::out_of_range& e) {}

    define(stmt.name());
  }


  void Resolver::visit_while_stmt(const While& stmt)
  {
    resolve(stmt.condition());
    resolve(stmt.body());
  }


  void Resolver::visit_variable_expr(const Variable& expr)
  {
    if (scopes_.size() != 0 and
        scopes_.top().count(expr.name().lexeme()) != 0 and
        not std::get<0>(scopes_.top()[expr.name().lexeme()])) {
      error(expr.name(), "Cannot read local variable in its own initialiser.");
    }

    resolve_local(expr, expr.name());
  }


  void Resolver::visit_assign_expr(const Assign& expr)
  {
    resolve(expr.value());
    resolve_local(expr, expr.name());
  }


  void Resolver::visit_ternary_expr(const Ternary& expr)
  {
    resolve(expr.first());
    resolve(expr.second());
    resolve(expr.third());
  }


  void Resolver::visit_binary_expr(const Binary& expr)
  {
    resolve(expr.left());
    resolve(expr.right());
  }


  void Resolver::visit_call_expr(const Call& expr)
  {
    resolve(expr.callee());

    for (const auto& arg : expr.arguments()) {
      resolve(*arg);
    }
  }


  void Resolver::visit_grouping_expr(const Grouping& expr)
  {
    resolve(expr.expression());
  }


  void Resolver::visit_logical_expr(const Logical& expr)
  {
    resolve(expr.left());
    resolve(expr.right());
  }


  void Resolver::visit_unary_expr(const Unary& expr)
  {
    resolve(expr.right());
  }


  void Resolver::visit_lambda_expr(const Lambda& expr)
  {
    resolve_lambda(expr);
  }


  void Resolver::resolve(
      const std::vector<std::shared_ptr<Stmt>>& statements)
  {
    for (const auto& stmt : statements) {
      resolve(*stmt);
    }
  }


  void Resolver::resolve(const Stmt& stmt)
  {
    stmt.accept(*this);
  }


  void Resolver::resolve(const Expr& expr)
  {
    expr.accept(*this);
  }


  void Resolver::resolve_function(const Function& function,
                                  const FunctionType type)
  {
    FunctionType enclosing_function = current_function_;
    current_function_ = type;

    begin_scope();

    for (const auto& param : function.parameters()) {
      declare(param);
      define(param);
    }

    resolve(function.body());
    end_scope();
    current_function_ = enclosing_function;
  }


  void Resolver::resolve_lambda(const Lambda& lambda)
  {
    FunctionType enclosing_function = current_function_;
    current_function_ = FunctionType::Lambda;

    begin_scope();

    for (const auto& param : lambda.parameters()) {
      declare(param);
      define(param);
    }

    resolve(lambda.body());
    end_scope();
    current_function_ = enclosing_function;
  }


  void Resolver::begin_scope()
  {
    scopes_.push(StackElem());
  }


  void Resolver::end_scope()
  {
    const auto scope = std::move(scopes_.pop());

    for (const auto& var_state : scope) {
      if (not std::get<1>(var_state.second)) {
        error(std::get<2>(var_state.second),
              "Variable '" + var_state.first + "' is unused.");
      }
    }
  }


  void Resolver::declare(const Token& name)
  {
    if (scopes_.size() == 0) {
      return;
    }
    if (scopes_.top().count(name.lexeme())) {
      error(name, "Variable with this name already declared in this scope.");
    }

    scopes_.top()[name.lexeme()] = std::make_tuple(false, false, name.line());
  }


  void Resolver::define(const Token& name)
  {
    if (scopes_.size() == 0) {
      return;
    }

    std::get<0>(scopes_.top()[name.lexeme()]) = true;
  }


  void Resolver::resolve_local(const Expr& expr, const Token& name)
  {
    for (int i = static_cast<int>(scopes_.size()) - 1; i >= 0; i--) {
      if (scopes_.get(i).count(name.lexeme()) > 0) {
        std::get<1>(scopes_.get(i)[name.lexeme()]) = true;
        interpreter_->resolve(expr, scopes_.size() - 1 - i);
      }
    }
  }
}