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

#ifndef LOXX_ASTPRINTER_HPP
#define LOXX_ASTPRINTER_HPP

#include <sstream>
#include <vector>

#include "Stmt.hpp"


namespace loxx
{
  class AstPrinter : public Expr::Visitor, public Stmt::Visitor
  {
  public:
    AstPrinter() : indent_level_(0) {}

    void visit_unary_expr(const Unary& expr) override;
    void visit_binary_expr(const Binary& expr) override;
    void visit_logical_expr(const Logical& expr) override;
    void visit_assign_expr(const Assign& expr) override;
    void visit_literal_expr(const Literal& expr) override;
    void visit_grouping_expr(const Grouping& expr) override;
    void visit_variable_expr(const Variable& expr) override;
    void visit_call_expr(const Call& expr) override;
    void visit_get_expr(const Get& expr) override;
    void visit_set_expr(const Set& expr) override;
    void visit_this_expr(const This& expr) override;
    void visit_super_expr(const Super& expr) override;

    void visit_if_stmt(const If& stmt) override;
    void visit_print_stmt(const Print& stmt) override;
    void visit_return_stmt(const Return& stmt) override;
    void visit_var_stmt(const Var& stmt) override;
    void visit_while_stmt(const While& stmt) override;
    void visit_expression_stmt(const Expression& stmt) override;
    void visit_function_stmt(const Function& func) override;
    void visit_block_stmt(const Block& stmt) override;
    void visit_class_stmt(const Class& stmt) override;

    template <typename T>
    std::string print(const std::vector<std::unique_ptr<T>>& statements);

  private:
    void paranthesise(const std::string& name,
                      std::initializer_list<raw_ptr<const Expr>> exprs);
    void set_indent(const unsigned int indent);

    unsigned int indent_level_;
    std::string indent_;
    std::stringstream stream_;
  };


  template <typename T>
  std::string AstPrinter::print(
      const std::vector<std::unique_ptr<T>>& statements)
  {
    for (const auto& stmt : statements) {
      stream_ << indent_;
      stmt->accept(*this);
      stream_ << '\n';
    }
    return stream_.str();
  }
}

#endif //LOXX_ASTPRINTER_HPP
