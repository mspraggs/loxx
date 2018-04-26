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
#include "ObjectTracker.hpp"
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
    begin_scope();
    compile(stmt.statements);
    end_scope();
  }


  void Compiler::visit_class_stmt(const Class& stmt)
  {
    const auto class_type_old = class_type_;
    class_type_ = stmt.superclass ? ClassType::Subclass : ClassType::Superclass;

    // If this class derives from an existing class, we create an additional
    // scope containing a reference to the superclass
    if (stmt.superclass) {
      begin_scope();
      locals_.push({});
      upvalues_.push({});

      const auto super_token =
          Token(TokenType::Super, "super", stmt.name.line());
      const auto local = declare_variable(super_token);
      compile(*stmt.superclass);
    }

    // Add an instruction to make the class
    const auto op = stmt.superclass ?
                    Instruction::CreateSubclass : Instruction::CreateClass;
    const auto name_constant = make_string_constant(stmt.name.lexeme());
    add_instruction(op);
    add_integer<UByteCodeArg>(name_constant);
    update_line_num_table(stmt.name);

    // Compile the class's methods
    for (const auto& method : stmt.methods) {
      const auto method_constant = make_string_constant(method->name.lexeme());
      const auto type =
          method->name.lexeme() == "init" ?
          FunctionType::Initialiser :
          FunctionType::Method;
      compile_function(*method, type);

      add_instruction(Instruction::CreateMethod);
      add_integer<UByteCodeArg>(method_constant);
      update_line_num_table(method->name);
    }

    // Close the scope we opened above, if applicable
    if (stmt.superclass) {
      end_scope();
      locals_.pop();
      upvalues_.pop();
    }

    define_variable(name_constant, stmt.name);

    class_type_ = class_type_old;
  }


  void Compiler::visit_expression_stmt(const Expression& stmt)
  {
    compile(*stmt.expression);
    add_instruction(Instruction::Pop);
  }


  void Compiler::visit_function_stmt(const Function& stmt)
  {
    const auto arg = declare_variable(stmt.name);
    compile_function(stmt, FunctionType::Function);
    define_variable(arg, stmt.name);
  }


  void Compiler::visit_if_stmt(const If& stmt)
  {
    compile(*stmt.condition);

    add_instruction(Instruction::ConditionalJump);
    const auto first_jump_pos = output_.bytecode.size();
    add_integer<ByteCodeArg>(0);

    add_instruction(Instruction::Pop);

    if (stmt.else_branch != nullptr) {
      compile(*stmt.else_branch);
    }

    const auto first_jump_size =
        static_cast<ByteCodeArg>(
            output_.bytecode.size() - first_jump_pos + 1);
    rewrite_integer(first_jump_pos, first_jump_size);

    add_instruction(Instruction::Jump);
    const auto second_jump_pos = output_.bytecode.size();
    add_integer<ByteCodeArg>(0);

    add_instruction(Instruction::Pop);

    compile(*stmt.then_branch);

    const auto second_jump_size =
        static_cast<ByteCodeArg>(
            output_.bytecode.size() - second_jump_pos - sizeof(ByteCodeArg));
    rewrite_integer(second_jump_pos, second_jump_size);
  }


  void Compiler::visit_print_stmt(const Print& stmt)
  {
    compile(*stmt.expression);

    add_instruction(Instruction::Print);
  }


  void Compiler::visit_return_stmt(const Return& stmt)
  {
    if (current_function_type_ == FunctionType::None) {
      error(stmt.keyword, "Cannot return from top-level code.");
    }
    if (current_function_type_ == FunctionType::Initialiser and
        stmt.value != nullptr) {
      error(stmt.keyword, "Cannot return a value from an initialiser.");
    }

    if (current_function_type_ == FunctionType::Initialiser) {
      compile_this_return();
      return;
    }
    else if (stmt.value != nullptr) {
      compile(*stmt.value);
    }
    else {
      add_instruction(Instruction::Nil);
    }
    add_instruction(Instruction::Return);
    update_line_num_table(stmt.keyword);
  }


  void Compiler::visit_var_stmt(const Var& stmt)
  {
    const auto arg = declare_variable(stmt.name);

    if (stmt.initialiser != nullptr) {
      compile(*stmt.initialiser);
    }
    else {
      add_instruction(Instruction::Nil);
    }

    define_variable(arg, stmt.name);
  }


  void Compiler::visit_while_stmt(const While& stmt)
  {
    // While loops are modelled around this structure:
    //
    // begin: <- first_label_pos
    // if (expr) goto body <- Instruction::ConditionalJump -
    // goto end <- Instruction::Jump - second_jump_pos
    // body:
    // ...
    // goto begin <- Instruction::Jump
    // end:
    // ...

    const auto first_label_pos = output_.bytecode.size();

    compile(*stmt.condition);

    add_instruction(Instruction::ConditionalJump);
    // We want to jump over the jump that takes us out of the while loop, which
    // also involves jumping over a pop instruction (hence + 2 for two
    // instructions).
    add_integer<ByteCodeArg>(sizeof(ByteCodeArg) + 2);
    add_instruction(Instruction::Pop);

    add_instruction(Instruction::Jump);
    const auto second_jump_pos = output_.bytecode.size();
    add_integer<ByteCodeArg>(0);

    // Pop the condition value (used by ConditionalJump) off the stack.
    add_instruction(Instruction::Pop);
    // Compile the body of the while loop.
    compile(*stmt.body);

    // N.B. Size of bytecode argument needs to be subtracted because the VM
    // automatically increments the instruction pointer when reading arguments.

    // Jump back to the start of the loop to check the condition again.
    add_instruction(Instruction::Jump);
    add_integer<ByteCodeArg>(
        first_label_pos - output_.bytecode.size() - sizeof(ByteCodeArg));

    // Back-patch the jump over the body of the while loop.
    rewrite_integer<ByteCodeArg>(
        second_jump_pos,
        output_.bytecode.size() - second_jump_pos - sizeof(ByteCodeArg));
  }


  void Compiler::visit_assign_expr(const Assign& expr)
  {
    handle_variable_reference(expr, true);
  }


  void Compiler::visit_binary_expr(const Binary& expr)
  {
    compile(*expr.left);
    compile(*expr.right);

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

    default:
      break;
    }
    update_line_num_table(expr.op);
  }


  void Compiler::visit_call_expr(const Call& expr)
  {
    compile(*expr.callee);

    for (const auto& argument : expr.arguments) {
      compile(*argument);
    }

    add_instruction(Instruction::Call);
    add_integer<UByteCodeArg>(expr.arguments.size());
    update_line_num_table(expr.paren);
  }


  void Compiler::visit_get_expr(const Get& expr)
  {
    compile(*expr.object);

    const auto name_constant = make_string_constant(expr.name.lexeme());
    add_instruction(Instruction::GetProperty);
    add_integer<UByteCodeArg>(name_constant);
    update_line_num_table(expr.name);
  }


  void Compiler::visit_grouping_expr(const Grouping& expr)
  {
    compile(*expr.expression);
  }


  void Compiler::visit_literal_expr(const Literal& expr)
  {
    if (holds_alternative<bool>(expr.value)) {
      add_instruction(
          get<bool>(expr.value) ? Instruction::True : Instruction::False);
      return;
    }
    else if (expr.value.index() == Value::npos) {
      add_instruction(Instruction::Nil);
      return;
    }

    const auto index = vm_->add_named_constant(expr.lexeme, expr.value);

    add_instruction(Instruction::LoadConstant);
    add_integer(index);
  }


  void Compiler::visit_logical_expr(const Logical& expr)
  {
    compile(*expr.left);

    if (expr.op.type() == TokenType::Or) {
      add_instruction(Instruction::ConditionalJump);
      const auto jump_pos = output_.bytecode.size();
      add_integer<ByteCodeArg>(0);
      update_line_num_table(expr.op);

      const auto skip_start = output_.bytecode.size();
      add_instruction(Instruction::Pop);
      compile(*expr.right);

      rewrite_integer<ByteCodeArg>(
          jump_pos, output_.bytecode.size() - skip_start);
    }
    else if (expr.op.type() == TokenType::And) {
      add_instruction(Instruction::ConditionalJump);
      add_integer<ByteCodeArg>(sizeof(ByteCodeArg) + 1);
      update_line_num_table(expr.op);
      add_instruction(Instruction::Jump);
      const auto jump_pos = output_.bytecode.size();
      add_integer<ByteCodeArg>(0);

      const auto skip_start = output_.bytecode.size();
      compile(*expr.right);
      rewrite_integer<ByteCodeArg>(
          jump_pos, output_.bytecode.size() - skip_start);
    }
  }


  void Compiler::visit_set_expr(const Set& expr)
  {
    compile(*expr.object);
    compile(*expr.value);

    const auto name_constant = make_string_constant(expr.name.lexeme());
    add_instruction(Instruction::SetProperty);
    add_integer<UByteCodeArg>(name_constant);
    update_line_num_table(expr.name);
  }


  void Compiler::visit_super_expr(const Super& expr)
  {
    if (class_type_ == ClassType::Superclass) {
      error(expr.keyword,
            "Cannot use 'super' in a class without a superclass.");
    }
    else if (class_type_ == ClassType::None) {
      error(expr.keyword, "Cannot use 'super' outside of a class.");
    }

    const auto this_token = Token(TokenType::This, "this", expr.keyword.line());
    handle_variable_reference(this_token, false);
    handle_variable_reference(expr.keyword, false);

    const auto func = make_string_constant(expr.method.lexeme());
    add_instruction(Instruction::GetSuperFunc);
    add_integer<UByteCodeArg>(func);
    update_line_num_table(expr.keyword);
  }


  void Compiler::visit_this_expr(const This& expr)
  {
    if (class_type_ == ClassType::None) {
      error(expr.keyword, "Cannot use 'this' outside of a class.");
    }
    handle_variable_reference(expr, false);
  }


  void Compiler::visit_unary_expr(const Unary& expr)
  {
    compile(*expr.right);

    if (expr.op.type() == TokenType::Bang) {
      add_instruction(Instruction::Not);
    }
    else if (expr.op.type() == TokenType::Minus) {
      add_instruction(Instruction::Negate);
    }
    update_line_num_table(expr.op);
  }


  void Compiler::visit_variable_expr(const Variable& expr)
  {
    handle_variable_reference(expr, false);
  }


  void Compiler::compile(const Expr& expr)
  {
    expr.accept(*this);
  }


  void Compiler::compile(const Stmt& stmt)
  {
    stmt.accept(*this);
  }


  void Compiler::compile_function(const Function& stmt, const FunctionType type)
  {
    const auto old_func_type = current_function_type_;
    current_function_type_ = type;

    // Because all code is compiled into one contiguous blob of bytecode,
    // there's the risk that we end up walking straight into bytecode that
    // belongs to functions. To avoid this we prepend a jump instruction before
    // the function's bytecode so that we can skip over the latter.
    add_instruction(Instruction::Jump);
    const auto jump_pos = output_.bytecode.size();
    add_integer<ByteCodeArg>(0);

    const auto bytecode_pos = output_.bytecode.size();

    begin_scope();
    locals_.push({});
    upvalues_.push({});

    // Declare/define "this"
    if (type == FunctionType::Method or type == FunctionType::Initialiser) {
      const auto this_token = Token(TokenType::This, "this", stmt.name.line());
      const auto param_index = declare_variable(this_token);
      define_variable(param_index, this_token);
    }

    // Declare/define function parameters
    for (const auto& param : stmt.parameters) {
      const auto param_index = declare_variable(param);
      define_variable(param_index, param);
    }

    compile(stmt.body);

    // Return "this" if in constructor
    if (current_function_type_ == FunctionType::Initialiser) {
      compile_this_return();
    }

    end_scope();
    locals_.pop();
    const auto upvalues = upvalues_.pop();

    // Return "nil" if we haven't returned already.
    add_instruction(Instruction::Nil);
    add_instruction(Instruction::Return);

    // Back-patch the jump over the function definition
    const auto jump_size =
        static_cast<ByteCodeArg>(output_.bytecode.size() - bytecode_pos);
    rewrite_integer(jump_pos, jump_size);

    // Add the new function object as a constant
    auto func = make_object<FuncObject>(
        stmt.name.lexeme(), bytecode_pos,
        static_cast<unsigned int>(stmt.parameters.size()),
        upvalues.size());
    const auto index = vm_->add_constant(Value(InPlace<ObjectPtr>(), func));

    add_instruction(Instruction::CreateClosure);
    add_integer<UByteCodeArg>(index);
    update_line_num_table(stmt.name);

    for (const auto& upvalue : upvalues) {
      add_integer<UByteCodeArg>(upvalue.is_local ? 1 : 0);
      add_integer<UByteCodeArg>(upvalue.index);
    }

    current_function_type_ = old_func_type;
  }


  void Compiler::compile_this_return()
  {
    const auto this_token = Token(TokenType::This, "this", last_line_num_);
    handle_variable_reference(this_token, false);
    add_instruction(Instruction::Return);
    update_line_num_table(this_token);
  }


  Optional<UByteCodeArg> Compiler::declare_variable(const Token& name)
  {
    const Optional<UByteCodeArg> arg =
        scope_depth_ == 0 ?
        vm_->add_string_constant(name.lexeme()) :
        Optional<UByteCodeArg>();

    if (not arg) {
      // Check to see if a variable with this name has been declared in this
      // scope already
      for (long int i = locals_.top().size() - 1; i >= 0; --i) {
        const auto& local = locals_.top()[i];

        if (local.defined and local.depth < scope_depth_) {
          break;
        }
        if (local.name == name.lexeme()) {
          throw CompileError(
              name,
              "Variable with this name already declared in this scope.");
        }
      }
      locals_.top().push_back({false, false, 0, name.lexeme()});
    }

    return arg;
  }


  void Compiler::define_variable(const Optional<UByteCodeArg>& arg,
                                 const Token& name)
  {
    if (arg) {
      add_instruction(Instruction::DefineGlobal);
      add_integer(*arg);
      update_line_num_table(name);
    }
    else {
      locals_.top().back().defined = true;
      locals_.top().back().depth = scope_depth_;
    }
  }


  void Compiler::handle_variable_reference(const Token& token, const bool write)
  {
    auto arg = resolve_local(token, false);

    Instruction op;
    if (arg) {
      op = write ? Instruction::SetLocal : Instruction::GetLocal;
    }
    else {
      arg = resolve_upvalue(locals_.size() - 1, token);

      if (arg) {
        op = write ? Instruction::SetUpvalue : Instruction::GetUpvalue;
      }
      else {
        op = write ? Instruction::SetGlobal : Instruction::GetGlobal;
      }
    }

    if (not arg) {
      arg = make_string_constant(token.lexeme());
    }

    add_instruction(op);
    add_integer(*arg);
    update_line_num_table(token);
  }


  Optional<UByteCodeArg> Compiler::resolve_local(
      const Token& name, const bool in_function,
      const std::size_t call_depth) const
  {
    const auto& locals =
        call_depth > locals_.size() ? locals_.top() : locals_.get(call_depth);

    for (long int i = locals.size() - 1; i >= 0; --i) {
      if (locals[i].name == name.lexeme()) {
        if (not in_function and not locals[i].defined) {
          throw CompileError(
              name, "Cannot read local variable in its own initialiser.");
        }
        return static_cast<UByteCodeArg>(i);
      }
    }
    return Optional<UByteCodeArg>();
  }


  Optional<UByteCodeArg> Compiler::resolve_upvalue(
      const std::size_t call_depth, const Token& name)
  {
    if (call_depth == 0) {
      // There is only one scope, so we're not going to find an upvalue
      return Optional<UByteCodeArg>();
    }

    auto local = resolve_local(name, true, call_depth - 1);

    if (local) {
      locals_.get(call_depth - 1)[*local].is_upvalue = true;
      return add_upvalue(call_depth, *local, true);
    }

    local = resolve_upvalue(call_depth - 1, name);
    if (local) {
      return add_upvalue(call_depth, *local, false);
    }

    return Optional<UByteCodeArg>();
  }


  UByteCodeArg Compiler::add_upvalue(
      const std::size_t call_depth, const UByteCodeArg index,
      const bool is_local)
  {
    auto& upvalues = upvalues_.get(call_depth);

    for (UByteCodeArg i = 0; i < upvalues.size(); ++i) {
      if (upvalues[i].index == index and upvalues[i].is_local == is_local) {
        return i;
      }
    }

    upvalues.push_back(Upvalue{is_local, index});
    return static_cast<UByteCodeArg>(upvalues.size() - 1);
  }


  void Compiler::begin_scope()
  {
    scope_depth_++;
  }


  void Compiler::end_scope()
  {
    scope_depth_--;

    while (not locals_.top().empty() and
           locals_.top().back().depth > scope_depth_) {
      if (locals_.top().back().is_upvalue) {
        add_instruction(Instruction::CloseUpvalue);
      }
      else {
        add_instruction(Instruction::Pop);
      }
      locals_.top().pop_back();
    }
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

    if (num_rows > 0) {
      const auto line_num_delta =
          static_cast<std::int8_t>(line_num_diff / num_rows);
      const auto instr_num_delta =
          static_cast<std::int8_t>(instr_num_diff / num_rows);

      for (unsigned int i = 0; i < num_rows; ++i) {
        output_.line_num_table.emplace_back(line_num_delta, instr_num_delta);
      }

      line_num_diff -= num_rows * line_num_delta;
      instr_num_diff -= num_rows * instr_num_delta;
    }

    output_.line_num_table.emplace_back(line_num_diff, instr_num_diff);
    last_instr_num_ = output_.bytecode.size();
    last_line_num_ = token.line();
  }


  void Compiler::add_instruction(const Instruction instruction)
  {
    output_.bytecode.push_back(static_cast<std::uint8_t>(instruction));
  }


  UByteCodeArg Compiler::make_string_constant(const std::string& str) const
  {
    return vm_->add_string_constant(str);
  }


  namespace detail
  {
    template <>
    void handle_value(const Assign& var_expr, Compiler& compiler)
    {
      var_expr.value->accept(compiler);
    }


    template <>
    const Token& get_token(const Assign& var_expr)
    {
      return var_expr.name;
    }


    template <>
    const Token& get_token(const Variable& var_expr)
    {
      return var_expr.name;
    }


    template <>
    const Token& get_token(const This& var_expr)
    {
      return var_expr.keyword;
    }
  }
}
