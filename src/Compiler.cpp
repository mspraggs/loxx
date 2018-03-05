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
 * Created by Matt Spraggs on 05/03/18.
 */

#include "Compiler.hpp"


namespace loxx
{
  void Compiler::compile(const std::vector<std::shared_ptr<Stmt>>& statements)
  {
    for (const auto& stmt : statements) {
      compile(*stmt);
    }
  }


  void Compiler::visit_block_stmt(const Block& stmt)
  {

  }


  void Compiler::visit_class_stmt(const Class& stmt)
  {

  }


  void Compiler::visit_expression_stmt(const Expression& stmt)
  {

  }


  void Compiler::visit_function_stmt(const Function& stmt)
  {

  }


  void Compiler::visit_if_stmt(const If& stmt)
  {

  }


  void Compiler::visit_print_stmt(const Print& stmt)
  {
    stmt.expression->accept(*this);

    byte_code_.push_back(static_cast<std::uint8_t>(Instruction::Print));
  }


  void Compiler::visit_return_stmt(const Return& stmt)
  {

  }


  void Compiler::visit_var_stmt(const Var& stmt)
  {

  }


  void Compiler::visit_while_stmt(const While& stmt)
  {

  }


  void Compiler::visit_assign_expr(const Assign& expr)
  {

  }


  void Compiler::visit_binary_expr(const Binary& expr)
  {
    expr.left->accept(*this);
    expr.right->accept(*this);

    switch (expr.op.type()) {

    case TokenType::Plus:
      byte_code_.push_back(static_cast<std::uint8_t>(Instruction::Add));
      break;

    case TokenType::Minus:
      byte_code_.push_back(static_cast<std::uint8_t>(Instruction::Subtract));
      break;

    case TokenType::Star:
      byte_code_.push_back(static_cast<std::uint8_t>(Instruction::Multiply));
      break;

    case TokenType::Slash:
      byte_code_.push_back(static_cast<std::uint8_t>(Instruction::Divide));
      break;
    }
  }


  void Compiler::visit_call_expr(const Call& expr)
  {

  }


  void Compiler::visit_get_expr(const Get& expr)
  {

  }


  void Compiler::visit_grouping_expr(const Grouping& expr)
  {
    expr.expression->accept(*this);
  }


  void Compiler::visit_literal_expr(const Literal& expr)
  {
    // TODO: Generalise for other types
    auto value = generic_cast<double>(expr.value);

    constants_.emplace_back(std::move(value));
    const auto index = constants_.size() - 1;

    byte_code_.push_back(static_cast<std::uint8_t>(Instruction::LoadConstant));

    for (unsigned int i = 0; i < sizeof(std::size_t); ++i) {
      byte_code_.push_back(static_cast<std::uint8_t>((index >> 8 * i) & 0xff));
    }
  }


  void Compiler::visit_logical_expr(const Logical& expr)
  {

  }


  void Compiler::visit_set_expr(const Set& expr)
  {

  }


  void Compiler::visit_super_expr(const Super& expr)
  {

  }


  void Compiler::visit_this_expr(const This& expr)
  {

  }


  void Compiler::visit_unary_expr(const Unary& expr)
  {

  }


  void Compiler::visit_variable_expr(const Variable& expr)
  {

  }


  void Compiler::compile(const Stmt& stmt)
  {
    stmt.accept(*this);
  }
}