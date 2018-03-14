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

#ifndef LOXX_COMPILER_HPP
#define LOXX_COMPILER_HPP

#include <vector>

#include "Expr.hpp"
#include "Instruction.hpp"
#include "Scope.hpp"
#include "Stack.hpp"
#include "Stmt.hpp"


namespace loxx
{
  class VirtualMachine;


  struct CompilationOutput
  {
    std::vector<std::uint8_t> bytecode;
    std::vector<std::tuple<std::int8_t, std::uint8_t>> line_num_table;
  };


  class Compiler : public Expr::Visitor, public Stmt::Visitor
  {
  public:
    explicit Compiler(VirtualMachine& vm)
        : last_line_num_(0), last_instr_num_(0), vm_(&vm), scope_(new Scope)
    {}

    void compile(const std::vector<std::unique_ptr<Stmt>>& statements);

    void visit_assign_expr(const Assign& expr) override;
    void visit_binary_expr(const Binary& expr) override;
    void visit_call_expr(const Call& expr) override;
    void visit_get_expr(const Get& expr) override;
    void visit_grouping_expr(const Grouping& expr) override;
    void visit_literal_expr(const Literal& expr) override;
    void visit_logical_expr(const Logical& expr) override;
    void visit_set_expr(const Set& expr) override;
    void visit_super_expr(const Super& expr) override;
    void visit_this_expr(const This& expr) override;
    void visit_unary_expr(const Unary& expr) override;
    void visit_variable_expr(const Variable& expr) override;

    void visit_block_stmt(const Block& stmt) override;
    void visit_class_stmt(const Class& stmt) override;
    void visit_expression_stmt(const Expression& stmt) override;
    void visit_function_stmt(const Function& stmt) override;
    void visit_if_stmt(const If& stmt) override;
    void visit_print_stmt(const Print& stmt) override;
    void visit_return_stmt(const Return& stmt) override;
    void visit_var_stmt(const Var& stmt) override;
    void visit_while_stmt(const While& stmt) override;

    const CompilationOutput& output() const { return output_; }

  private:
    void compile(const Stmt& stmt);
    void update_line_num_table(const Token& token);
    void add_instruction(const Instruction instruction);
    template <typename T>
    void add_integer(const T integer);
    template <typename T>
    void rewrite_integer(const std::size_t pos, const T integer);

    unsigned int last_line_num_;
    std::size_t last_instr_num_;
    VirtualMachine* vm_;
    CompilationOutput output_;
    std::unique_ptr<Scope> scope_;
    Stack<ByteCodeArg> call_positions;
  };


  template<typename T>
  void Compiler::add_integer(const T integer)
  {
    for (unsigned int i = 0; i < sizeof(T); ++i) {
      output_.bytecode.push_back(
          static_cast<std::uint8_t>((integer >> 8 * i) & 0xff));
    }
  }


  template<typename T>
  void Compiler::rewrite_integer(const std::size_t pos, const T integer)
  {
    for (unsigned int i = 0; i < sizeof(T); ++i) {
      output_.bytecode[pos + i] =
          static_cast<std::uint8_t>((integer >> 8 * i) & 0xff);
    }
  }
}

#endif //LOXX_COMPILER_HPP
