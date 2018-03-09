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
  void Compiler::compile(const std::vector<std::unique_ptr<Stmt>>& statements)
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

    add_instruction(Instruction::Print);
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
      add_instruction(Instruction::Add);
      update_line_num_table(expr.op);
      break;

    case TokenType::Minus:
      add_instruction(Instruction::Subtract);
      update_line_num_table(expr.op);
      break;

    case TokenType::Star:
      add_instruction(Instruction::Multiply);
      update_line_num_table(expr.op);
      break;

    case TokenType::Slash:
      add_instruction(Instruction::Divide);
      update_line_num_table(expr.op);
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
    vm_->constants().push_back(expr.value);
    const auto index = vm_->constants().size() - 1;

    add_instruction(Instruction::LoadConstant);

    for (unsigned int i = 0; i < sizeof(std::size_t); ++i) {
      output_.bytecode.push_back(
          static_cast<std::uint8_t>((index >> 8 * i) & 0xff));
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


  void Compiler::update_line_num_table(const Token& token)
  {
    int line_num_diff = token.line() - last_line_num_;
    unsigned int line_num_diff_abs = std::abs(line_num_diff);
    std::size_t instr_num_diff = output_.bytecode.size() - last_instr_num_;

    const unsigned int num_rows =
        std::max(line_num_diff_abs / 128, instr_num_diff / 256);

    std::int8_t line_num_delta = line_num_diff / num_rows;
    std::int8_t instr_num_delta = instr_num_diff / num_rows;

    for (unsigned int i = 0; i < num_rows; ++i) {
      output_.line_num_table.emplace_back(line_num_delta, instr_num_delta);
    }

    line_num_diff -= num_rows * line_num_delta;
    instr_num_diff -= num_rows * instr_num_delta;

    if (line_num_diff > 0 or instr_num_diff > 0) {
      output_.line_num_table.emplace_back(line_num_diff, instr_num_diff);
    }
  }


  void Compiler::add_instruction(const Instruction instruction)
  {
    output_.bytecode.push_back(static_cast<std::uint8_t>(instruction));
  }
}
