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

#ifndef LOXX_INTERPRETER_HPP
#define LOXX_INTERPRETER_HPP

#include <stack>
#include <vector>

#include "Environment.hpp"
#include "Expr.hpp"
#include "Stack.hpp"
#include "Stmt.hpp"


namespace loxx
{
  class Interpreter : public Expr::Visitor, public Stmt::Visitor
  {
  public:
    Interpreter(const bool in_repl = false);

    void interpret(const std::vector<std::shared_ptr<Stmt>>& statements);

    void visit_expression_stmt(const Expression& stmt) override;
    void visit_function_stmt(const Function& stmt) override;
    void visit_if_stmt(const If& stmt) override;
    void visit_print_stmt(const Print& stmt) override;
    void visit_return_stmt(const Return& stmt) override;
    void visit_var_stmt(const Var& stmt) override;
    void visit_while_stmt(const While& stmt) override;
    void visit_block_stmt(const Block& stmt) override;

    void visit_assign_expr(const Assign& expr) override;
    void visit_unary_expr(const Unary& expr) override;
    void visit_binary_expr(const Binary& expr) override;
    void visit_call_expr(const Call& expr) override;
    void visit_literal_expr(const Literal& expr) override;
    void visit_logical_expr(const Logical& expr) override;
    void visit_grouping_expr(const Grouping& expr) override;
    void visit_variable_expr(const Variable& expr) override;

    void execute_block(const std::vector<std::shared_ptr<Stmt>>& statements,
                       std::shared_ptr<Environment> environment);

    std::shared_ptr<Environment> get_global_env() const { return globals_; }

    class Returner : public std::runtime_error
    {
    public:
      explicit Returner(Generic value)
          : std::runtime_error(""), value_(std::move(value))
      {}

      const Generic& value() const { return value_; }

    private:
      Generic value_;
    };

  private:
    void evaluate(const Expr& expr);
    void execute(const Stmt& stmt);
    bool is_truthy(const Generic& value);
    bool is_equal(const Generic& left, const Generic& right);
    void check_number_operand(const Token& op, const Generic& value) const;
    void check_number_operands(const Token& op,
                               const Generic& left, const Generic& right) const;
    std::string stringify(const Generic& generic) const;

    bool in_repl_, print_result_;
    Stack<Generic> stack_;
    std::shared_ptr<Environment> environment_, globals_;
  };
}

#endif //LOXX_INTERPRETER_HPP
