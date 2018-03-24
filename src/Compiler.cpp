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
#include "logging.hpp"
#include "VirtualMachine.hpp"


namespace loxx
{
  void Compiler::compile(const std::vector<std::unique_ptr<Stmt>>& statements)
  {
    for (const auto& stmt : statements) {
      try {
        compile(*stmt);
      }
      catch (const CompileError& e) {
        error(e.name(), e.what());
      }
    }
  }


  void Compiler::visit_block_stmt(const Block& stmt)
  {
    scope_depth_++;
    compile(stmt.statements);
    scope_depth_--;

    while (not locals_.empty() and std::get<1>(locals_.back()) > scope_depth_) {
      add_instruction(Instruction::Pop);
      locals_.pop_back();
    }
  }


  void Compiler::visit_class_stmt(const Class& stmt)
  {

  }


  void Compiler::visit_expression_stmt(const Expression& stmt)
  {
    stmt.expression->accept(*this);
  }


  void Compiler::visit_function_stmt(const Function& stmt)
  {

  }


  void Compiler::visit_if_stmt(const If& stmt)
  {
    stmt.condition->accept(*this);

    add_instruction(Instruction::ConditionalJump);
    const auto first_jump_pos = output_.bytecode.size();
    add_integer<ByteCodeArg>(0);

    if (stmt.else_branch != nullptr) {
      stmt.else_branch->accept(*this);
    }

    const auto first_jump_size =
        static_cast<ByteCodeArg>(
            output_.bytecode.size() - first_jump_pos + 1);
    rewrite_integer(first_jump_pos, first_jump_size);

    add_instruction(Instruction::Jump);
    const auto second_jump_pos = output_.bytecode.size();
    add_integer<ByteCodeArg>(0);

    stmt.then_branch->accept(*this);

    const auto second_jump_size =
        static_cast<ByteCodeArg>(
            output_.bytecode.size() - second_jump_pos - sizeof(ByteCodeArg));
    rewrite_integer(second_jump_pos, second_jump_size);
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
    const auto is_global = scope_depth_ == 0;

    const UByteCodeArg arg =
        is_global ?
        vm_->make_constant(stmt.name.lexeme(), stmt.name.lexeme()) : 0;

    if (not is_global) {
      // Check to see if a variable with this name has been declared in this
      // scope already
      for (long int i = locals_.size() - 1; i >= 0; --i) {
        const auto& local = locals_[i];

        if (std::get<0>(local) and std::get<1>(local) < scope_depth_) {
          break;
        }
        if (std::get<2>(local) == stmt.name.lexeme()) {
          throw CompileError(
              stmt.name,
              "Variable with this name already declared in this scope.");
        }
      }
      locals_.emplace_back(false, 0, stmt.name.lexeme());
    }

    if (stmt.initialiser != nullptr) {
      stmt.initialiser->accept(*this);
    }
    else {
      add_instruction(Instruction::Nil);
    }

    if (is_global) {
      add_instruction(Instruction::DefineGlobal);
      add_integer(arg);
    }
    else {
      std::get<0>(locals_.back()) = true;
      std::get<1>(locals_.back()) = scope_depth_;
    }
    update_line_num_table(stmt.name);
  }


  void Compiler::visit_while_stmt(const While& stmt)
  {
    const auto first_label_pos = output_.bytecode.size();

    stmt.condition->accept(*this);

    add_instruction(Instruction::ConditionalJump);
    add_integer<ByteCodeArg>(sizeof(ByteCodeArg) + 1);

    add_instruction(Instruction::Jump);
    const auto second_jump_pos = output_.bytecode.size();
    add_integer<ByteCodeArg>(0);

    compile(*stmt.body);

    add_instruction(Instruction::Jump);
    add_integer<ByteCodeArg>(first_label_pos - output_.bytecode.size());

    rewrite_integer<ByteCodeArg>(
        second_jump_pos, output_.bytecode.size() - second_jump_pos);
  }


  void Compiler::visit_assign_expr(const Assign& expr)
  {
    handle_variable_reference(expr, true);
  }


  void Compiler::visit_binary_expr(const Binary& expr)
  {
    expr.left->accept(*this);
    expr.right->accept(*this);

    switch (expr.op.type()) {

    case TokenType::Plus:
      add_instruction(Instruction::Add);
      break;

    case TokenType::Minus:
      add_instruction(Instruction::Subtract);
      break;

    case TokenType::Star:
      add_instruction(Instruction::Multiply);
      break;

    case TokenType::Slash:
      add_instruction(Instruction::Divide);
      break;

    case TokenType::Less:
      add_instruction(Instruction::Less);
      break;

    case TokenType::LessEqual:
      add_instruction(Instruction::LessEqual);
      break;

    case TokenType::Greater:
      add_instruction(Instruction::Greater);
      break;

    case TokenType::GreaterEqual:
      add_instruction(Instruction::GreaterEqual);
      break;

    case TokenType::EqualEqual:
      add_instruction(Instruction::Equal);
      break;

    case TokenType::BangEqual:
      add_instruction(Instruction::NotEqual);
      break;
    }
    update_line_num_table(expr.op);
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
    if (holds_alternative<bool>(expr.value)) {
      add_instruction(
          get<bool>(expr.value) ? Instruction::True : Instruction::False);
      return;
    }

    const auto index = vm_->make_constant(expr.lexeme, expr.value);

    add_instruction(Instruction::LoadConstant);
    add_integer(index);
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
    handle_variable_reference(expr, false);
  }


  void Compiler::compile(const Stmt& stmt)
  {
    stmt.accept(*this);
  }


  std::tuple<bool, UByteCodeArg> Compiler::resolve_local(
      const Token& name, const bool in_function) const
  {
    for (long int i = locals_.size() - 1; i >= 0; --i) {
      if (std::get<2>(locals_[i]) == name.lexeme()) {
        if (not in_function and not std::get<0>(locals_[i])) {
          throw CompileError(
              name, "Cannot read local variable in its own initialiser.");
        }
        return std::make_tuple(true, static_cast<UByteCodeArg>(i));
      }
    }
    return std::make_tuple(false, static_cast<UByteCodeArg>(0));
  }


  void Compiler::update_line_num_table(const Token& token)
  {
    // Okay, so I totally ripped off CPython's strategy for encoding line
    // numbers here, but what I can I say? It's a good strategy!

    // The broad idea here is that for each instruction we encode the difference
    // between the last line number number and the current line number, and the
    // last instruction and the current instruction. If the last line number is
    // different to the previous one, the difference in instruction and line is
    // encoded as one or more pairs of bytes.

    int line_num_diff = token.line() - last_line_num_;
    auto line_num_diff_abs = static_cast<unsigned int>(std::abs(line_num_diff));
    std::size_t instr_num_diff = output_.bytecode.size() - last_instr_num_;

    const auto num_rows =
        std::max(line_num_diff_abs / 128,
                 static_cast<unsigned int>(instr_num_diff / 256));

    const auto line_num_delta =
        num_rows == 0 ? 1 : static_cast<std::int8_t>(line_num_diff / num_rows);
    const auto instr_num_delta =
        num_rows == 0 ? 1 : static_cast<std::int8_t>(instr_num_diff / num_rows);

    for (unsigned int i = 0; i < num_rows; ++i) {
      output_.line_num_table.emplace_back(line_num_delta, instr_num_delta);
    }

    line_num_diff -= num_rows * line_num_delta;
    instr_num_diff -= num_rows * instr_num_delta;

    if (line_num_diff != 0) {
      output_.line_num_table.emplace_back(line_num_diff, instr_num_diff);
    }

    last_line_num_ = token.line();
  }


  void Compiler::add_instruction(const Instruction instruction)
  {
    output_.bytecode.push_back(static_cast<std::uint8_t>(instruction));
  }


  UByteCodeArg Compiler::get_constant(const std::string& str) const
  {
    return vm_->get_constant(str);
  }
}
