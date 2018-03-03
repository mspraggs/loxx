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
    resolve(stmt.statements);
    end_scope();
  }


  void Resolver::visit_class_stmt(const Class& stmt)
  {
    declare(stmt.name);
    define(stmt.name);
    const auto enclosing_class = current_class_;
    current_class_ = ClassType::Class;

    if (stmt.superclass != nullptr) {
      current_class_ = ClassType::SubClass;
      resolve(*stmt.superclass);

      begin_scope();
      scopes_.top()["super"] = true;
    }

    begin_scope();
    scopes_.top()["this"] = true;

    for (const auto& method : stmt.methods) {
      auto declaration = FunctionType::Method;

      if (method->name.lexeme() == "init") {
        declaration = FunctionType::Initialiser;
      }

      resolve_function(*method, declaration);
    }

    if (stmt.superclass != nullptr) {
      end_scope();
    }

    current_class_ = enclosing_class;
    end_scope();
  }


  void Resolver::visit_expression_stmt(const Expression& stmt)
  {
    resolve(*stmt.expression);
  }


  void Resolver::visit_function_stmt(const Function& stmt)
  {
    declare(stmt.name);
    define(stmt.name);

    resolve_function(stmt, FunctionType::Function);
  }


  void Resolver::visit_if_stmt(const If& stmt)
  {
    resolve(*stmt.condition);
    resolve(*stmt.then_branch);

    if (stmt.else_branch != nullptr) {
      resolve(*stmt.else_branch);
    }
  }


  void Resolver::visit_print_stmt(const Print& stmt)
  {
    resolve(*stmt.expression);
  }


  void Resolver::visit_return_stmt(const Return& stmt)
  {
    if (current_function_ == FunctionType::None) {
      error(stmt.keyword, "Cannot return from top-level code.");
    }

    if (current_function_ == FunctionType::Initialiser) {
      error(stmt.keyword, "Cannot return a value from an initialiser.");
    }

    if (stmt.value != nullptr) {
      resolve(*stmt.value);
    }
  }


  void Resolver::visit_var_stmt(const Var& stmt)
  {
    declare(stmt.name);

    if (stmt.initialiser != nullptr) {
      resolve(*stmt.initialiser);
    }

    define(stmt.name);
  }


  void Resolver::visit_while_stmt(const While& stmt)
  {
    resolve(*stmt.condition);
    resolve(*stmt.body);
  }


  void Resolver::visit_variable_expr(const Variable& expr)
  {
    if (scopes_.size() != 0 and
        scopes_.top().count(expr.name.lexeme()) != 0 and
        not scopes_.top()[expr.name.lexeme()]) {
      error(expr.name, "Cannot read local variable in its own initialiser.");
    }

    resolve_local(expr, expr.name);
  }


  void Resolver::visit_assign_expr(const Assign& expr)
  {
    resolve(*expr.value);
    resolve_local(expr, expr.name);
  }


  void Resolver::visit_binary_expr(const Binary& expr)
  {
    resolve(*expr.left);
    resolve(*expr.right);
  }


  void Resolver::visit_call_expr(const Call& expr)
  {
    resolve(*expr.callee);

    for (const auto& arg : expr.arguments) {
      resolve(*arg);
    }
  }


  void Resolver::visit_get_expr(const Get& expr)
  {
    resolve(*expr.object);
  }


  void Resolver::visit_grouping_expr(const Grouping& expr)
  {
    resolve(*expr.expression);
  }


  void Resolver::visit_logical_expr(const Logical& expr)
  {
    resolve(*expr.left);
    resolve(*expr.right);
  }


  void Resolver::visit_set_expr(const Set& expr)
  {
    resolve(*expr.value);
    resolve(*expr.object);
  }


  void Resolver::visit_super_expr(const Super& expr)
  {
    if (current_class_ == ClassType::None) {
      error(expr.keyword, "Cannot use 'super' outside of a class.");
    }
    else if (current_class_ != ClassType::SubClass) {
      error(expr.keyword, "Cannot use 'super' in a class without a superclass.");
    }
    resolve_local(expr, expr.keyword);
  }


  void Resolver::visit_this_expr(const This& expr)
  {
    if (current_class_ == ClassType::None) {
      error(expr.keyword, "Cannot use 'this' outside of a class.");
    }
    resolve_local(expr, expr.keyword);
  }


  void Resolver::visit_unary_expr(const Unary& expr)
  {
    resolve(*expr.right);
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

    for (const auto& param : function.parameters) {
      declare(param);
      define(param);
    }

    resolve(function.body);
    end_scope();
    current_function_ = enclosing_function;
  }


  void Resolver::begin_scope()
  {
    scopes_.push(std::unordered_map<std::string, bool>());
  }


  void Resolver::end_scope()
  {
    scopes_.pop();
  }


  void Resolver::declare(const Token& name)
  {
    if (scopes_.size() == 0) {
      return;
    }
    if (scopes_.top().count(name.lexeme())) {
      error(name, "Variable with this name already declared in this scope.");
    }

    scopes_.top()[name.lexeme()] = false;
  }


  void Resolver::define(const Token& name)
  {
    if (scopes_.size() == 0) {
      return;
    }

    scopes_.top()[name.lexeme()] = true;
  }


  void Resolver::resolve_local(const Expr& expr, const Token& name)
  {
    for (int i = static_cast<int>(scopes_.size()) - 1; i >= 0; i--) {
      if (scopes_.get(i).count(name.lexeme()) > 0) {
        interpreter_->resolve(expr, scopes_.size() - 1 - i);
      }
    }
  }
}
