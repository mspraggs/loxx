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

#include <functional>
#include <limits>
#include <utility>
#include <vector>

#include "Expr.hpp"
#include "Instruction.hpp"
#include "Optional.hpp"
#include "Stack.hpp"
#include "Stmt.hpp"


namespace loxx
{
  namespace detail
  {
    constexpr auto size_t_max = std::numeric_limits<std::size_t>::max();
  }


  class VirtualMachine;


  struct CodeObject
  {
    std::vector<std::uint8_t> bytecode;
    std::vector<std::tuple<std::int8_t, std::uint8_t>> line_num_table;
  };


  class Compiler : public Expr::Visitor, public Stmt::Visitor
  {
  public:
    explicit Compiler(VirtualMachine& vm, CodeObject& compiler_output)
        : class_type_(ClassType::None),
          current_function_type_(FunctionType::None), last_line_num_(0),
          scope_depth_(0), last_instr_num_(0), vm_(&vm),
          current_(compiler_output)
    {
      locals_.push(std::vector<Local>());
      upvalues_.push(std::vector<Upvalue>());
    }

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

    const CodeObject& output() const { return current_; }

  private:
    enum class ClassType {
      Superclass,
      Subclass,
      None
    };

    enum class FunctionType {
      Function,
      Initialiser,
      Method,
      None
    };

    class CompileError : public std::runtime_error
    {
    public:
      CompileError(Token name, const std::string& msg)
          : std::runtime_error(msg), name_(std::move(name))
      {}

      const Token& name() const { return name_; }

    private:
      Token name_;
    };

    struct Local
    {
      bool defined;
      bool is_upvalue;
      std::size_t depth;
      std::string name;
    };

    struct Upvalue
    {
      bool is_local;
      UByteCodeArg index;
    };

    void compile(const Expr& expr);
    void compile(const Stmt& stmt);
    void compile_function(const Function& stmt, const FunctionType type);
    void compile_this_return();
    Optional<UByteCodeArg> declare_variable(const Token& name);
    void define_variable(const Optional <UByteCodeArg>& arg, const Token& name);
    template <typename T>
    void handle_variable_reference(const T& expr, const bool write);
    void handle_variable_reference(const Token& token, const bool write);
    Optional<UByteCodeArg> resolve_local(
        const Token& name, const bool in_function,
        const std::size_t call_depth = detail::size_t_max) const;
    Optional<UByteCodeArg> resolve_upvalue(
        const std::size_t call_depth, const Token& name);
    UByteCodeArg add_upvalue(
        const std::size_t call_depth, const UByteCodeArg index,
        const bool is_local);
    void begin_scope();
    void end_scope();
    void update_line_num_table(const Token& token);
    void add_instruction(const Instruction instruction);
    template <typename T>
    void add_integer(const T integer);
    template <typename T>
    void rewrite_integer(const std::size_t pos, const T integer);
    inline UByteCodeArg make_string_constant(const std::string& str) const;

    ClassType class_type_;
    FunctionType current_function_type_;
    unsigned int last_line_num_;
    unsigned int scope_depth_;
    std::size_t last_instr_num_;
    raw_ptr<VirtualMachine> vm_;
    std::reference_wrapper<CodeObject> current_;
    Stack<std::vector<Local>> locals_;
    Stack<std::vector<Upvalue>> upvalues_;
  };


  template <typename T>
  T read_integer_at_pos(const raw_ptr<const std::uint8_t> pos)
  {
    const T integer = *reinterpret_cast<raw_ptr<const T>>(pos);
    return integer;
  }


  namespace detail
  {
    template <typename T>
    void handle_value(const T&, Compiler&)
    {
    }


    template <typename T>
    const Token& get_token(const T& var_expr);
  }


  template <typename T>
  void Compiler::handle_variable_reference(const T& expr, const bool write)
  {
    detail::handle_value(expr, *this);
    const auto& token = detail::get_token(expr);
    handle_variable_reference(token, write);
  }


  template<typename T>
  void Compiler::add_integer(const T integer)
  {
    auto& bytecode = current_.get().bytecode;
    const auto integer_ptr = reinterpret_cast<const std::uint8_t*>(&integer);
    bytecode.insert(bytecode.end(), integer_ptr, integer_ptr + sizeof(T));
  }


  template<typename T>
  void Compiler::rewrite_integer(const std::size_t pos, const T integer)
  {
    *reinterpret_cast<T*>(&current_.get().bytecode[pos]) = integer;
  }
}

#endif //LOXX_COMPILER_HPP
