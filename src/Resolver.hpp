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

#ifndef LOXX_RESOLVER_HPP
#define LOXX_RESOLVER_HPP

#include <string>
#include <unordered_map>

#include "Expr.hpp"
#include "Interpreter.hpp"
#include "Stack.hpp"
#include "Stmt.hpp"


namespace loxx
{
  class Resolver : public Expr::Visitor, public Stmt::Visitor
  {
  public:
    explicit Resolver(Interpreter& interpreter)
        : interpreter_(&interpreter), current_function_(FunctionType::None),
          current_class_(ClassType::None)
    {}

    void visit_block_stmt(const Block& stmt) override;
    void visit_class_stmt(const Class& stmt) override;
    void visit_expression_stmt(const Expression& stmt) override;
    void visit_function_stmt(const Function& stmt) override;
    void visit_if_stmt(const If& stmt) override;
    void visit_return_stmt(const Return& stmt) override;
    void visit_print_stmt(const Print& stmt) override;
    void visit_var_stmt(const Var& stmt) override;
    void visit_while_stmt(const While& stmt) override;

    void visit_variable_expr(const Variable& expr) override;
    void visit_assign_expr(const Assign& expr) override;
    void visit_binary_expr(const Binary& expr) override;
    void visit_call_expr(const Call& expr) override;
    void visit_get_expr(const Get& expr) override;
    void visit_grouping_expr(const Grouping& expr) override;
    void visit_literal_expr(const Literal& expr) override {}
    void visit_logical_expr(const Logical& expr) override;
    void visit_set_expr(const Set& expr) override;
    void visit_super_expr(const Super& expr) override;
    void visit_this_expr(const This& expr) override;
    void visit_unary_expr(const Unary& expr) override;

    void resolve(const std::vector<std::shared_ptr<Stmt>>& statements);

  private:
    enum class FunctionType { None, Function, Initialiser, Method };
    enum class ClassType { None, Class };

    void resolve(const Stmt& stmt);
    void resolve(const Expr& expr);
    void resolve_function(const Function& function, const FunctionType type);
    void begin_scope();
    void end_scope();
    void declare(const Token& name);
    void define(const Token& name);
    void resolve_local(const Expr& expr, const Token& name);

    Interpreter* interpreter_;
    Stack<std::unordered_map<std::string, bool>> scopes_;
    FunctionType current_function_;
    ClassType current_class_;
  };
}

#endif //LOXX_RESOLVER_HPP
