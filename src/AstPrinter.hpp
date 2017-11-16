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
    void visit_unary_expr(const Unary& expr) override;
    void visit_binary_expr(const Binary& expr) override;
    void visit_literal_expr(const Literal& expr) override;
    void visit_grouping_expr(const Grouping& expr) override;
    void visit_variable_expr(const Variable& expr) override;

    void visit_print_stmt(const Print& expr) override;
    void visit_var_stmt(const Var& expr) override;
    void visit_expression_stmt(const Expression& expr) override;

    std::string print(const std::vector<std::unique_ptr<Stmt>>& statements);

  private:
    void paranthesise(const std::string& name,
                      std::initializer_list<const Expr*> exprs);

    std::stringstream stream_;
  };
}

#endif //LOXX_ASTPRINTER_HPP
