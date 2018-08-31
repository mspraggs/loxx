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
    func_->begin_scope();
    compile(stmt.statements);
    func_->end_scope();
  }


  void Compiler::visit_class_stmt(const Class& stmt)
  {
    const auto class_type_old = class_type_;
    class_type_ = stmt.superclass ? ClassType::Subclass : ClassType::Superclass;

    // If this class derives from an existing class, we create an additional
    // scope containing a reference to the superclass, which is then captured
    // as an upvalue if it's used anywhere via the super keyword
    if (stmt.superclass) {
      func_->begin_scope();

      func_->add_local(func_->make_token(TokenType::Super, "super"));
      compile(*stmt.superclass);
    }

    // Add an instruction to make the class
    const auto op = stmt.superclass ?
                    Instruction::CreateSubclass : Instruction::CreateClass;
    const auto name_constant = make_string_constant(stmt.name.lexeme());
    func_->add_instruction(op);
    func_->add_integer<UByteCodeArg>(name_constant);
    func_->update_line_num_table(stmt.name);

    // Compile the class's methods
    for (const auto& method : stmt.methods) {
      const auto method_constant = make_string_constant(method->name.lexeme());
      const auto type =
          method->name.lexeme() == "init" ?
          FunctionType::Initialiser :
          FunctionType::Method;
      compile_function(*method, type);

      func_->add_instruction(Instruction::CreateMethod);
      func_->add_integer<UByteCodeArg>(method_constant);
      func_->update_line_num_table(method->name);
    }

    // Close the scope we opened above, if applicable
    if (stmt.superclass) {
      func_->end_scope();
    }

    define_variable(name_constant, stmt.name);

    class_type_ = class_type_old;
  }


  void Compiler::visit_expression_stmt(const Expression& stmt)
  {
    compile(*stmt.expression);
    func_->add_instruction(Instruction::Pop);
  }


  void Compiler::visit_function_stmt(const Function& stmt)
  {
    const auto arg = declare_variable(stmt.name);
    compile_function(stmt, FunctionType::Function);
    define_variable(arg, stmt.name);
  }


  void Compiler::visit_if_stmt(const If& stmt)
  {
    // If statements are implemented like this:
    // if (not expr) goto else <- Instruction::ConditionalJump
    // ...
    // goto end
    // --- else is compiled only if it exists in stmt
    // else:
    // ...
    // --- /else compilation
    // end:
    // ...
    compile(*stmt.condition);

    func_->add_instruction(Instruction::ConditionalJump);
    const auto first_jump_pos = func_->current_bytecode_size();
    func_->add_integer<UByteCodeArg>(0);

    func_->add_instruction(Instruction::Pop);

    compile(*stmt.then_branch);

    const auto first_jump_size = static_cast<ByteCodeArg>(
        func_->current_bytecode_size() - first_jump_pos + 1);
    func_->rewrite_integer(first_jump_pos, first_jump_size);

    func_->add_instruction(Instruction::Jump);
    const auto second_jump_pos = func_->current_bytecode_size();
    func_->add_integer<UByteCodeArg>(0);

    func_->add_instruction(Instruction::Pop);

    if (stmt.else_branch != nullptr) {
      compile(*stmt.else_branch);
    }

    const auto second_jump_size = static_cast<ByteCodeArg>(
            func_->current_bytecode_size() - second_jump_pos -
            sizeof(ByteCodeArg));
    func_->rewrite_integer(second_jump_pos, second_jump_size);
  }


  void Compiler::visit_print_stmt(const Print& stmt)
  {
    compile(*stmt.expression);

    func_->add_instruction(Instruction::Print);
  }


  void Compiler::visit_return_stmt(const Return& stmt)
  {
    if (func_->type() == FunctionType::None) {
      error(stmt.keyword, "Cannot return from top-level code.");
    }
    if (func_->type() == FunctionType::Initialiser and stmt.value != nullptr) {
      error(stmt.keyword, "Cannot return a value from an initialiser.");
    }

    if (func_->type() == FunctionType::Initialiser) {
      compile_this_return();
      return;
    }
    else if (stmt.value != nullptr) {
      compile(*stmt.value);
    }
    else {
      func_->add_instruction(Instruction::Nil);
    }
    func_->add_instruction(Instruction::Return);
    func_->update_line_num_table(stmt.keyword);
  }


  void Compiler::visit_var_stmt(const Var& stmt)
  {
    const auto arg = declare_variable(stmt.name);

    if (stmt.initialiser != nullptr) {
      compile(*stmt.initialiser);
    }
    else {
      func_->add_instruction(Instruction::Nil);
    }

    define_variable(arg, stmt.name);
  }


  void Compiler::visit_while_stmt(const While& stmt)
  {
    // While loops are modelled around this structure:
    //
    // begin: <- first_label_pos
    // if (not expr) goto end <- Instruction::ConditionalJump - first_jump_pos
    // body:
    // ...
    // goto begin <- Instruction::Jump
    // end:
    // ...

    const auto first_label_pos = func_->current_bytecode_size();

    compile(*stmt.condition);

    func_->add_instruction(Instruction::ConditionalJump);
    const auto first_jump_pos = func_->current_bytecode_size();
    // We want to jump over the jump that takes us out of the while loop, which
    // also involves jumping over a pop instruction (hence + 2 for two
    // instructions).
    func_->add_integer<ByteCodeArg>(0);
    func_->add_instruction(Instruction::Pop);

    // Compile the body of the while loop.
    compile(*stmt.body);

    // N.B. Size of bytecode argument needs to be subtracted because the VM
    // automatically increments the instruction pointer when reading arguments.

    // Jump back to the start of the loop to check the condition again.
    func_->add_instruction(Instruction::Jump);
    func_->add_integer<ByteCodeArg>(
        first_label_pos - func_->current_bytecode_size() - sizeof(ByteCodeArg));

    // Back-patch the jump over the body of the while loop.
    func_->rewrite_integer<ByteCodeArg>(
        first_jump_pos,
        func_->current_bytecode_size() - first_jump_pos - sizeof(ByteCodeArg));
    func_->add_instruction(Instruction::Pop);
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

    case TokenType::Plus: {
      func_->add_instruction(Instruction::Add);
    }
      break;

    case TokenType::Minus: {
      func_->add_instruction(Instruction::Subtract);
    }
      break;

    case TokenType::Star: {
      func_->add_instruction(Instruction::Multiply);
    }
      break;

    case TokenType::Slash: {
      func_->add_instruction(Instruction::Divide);
    }
      break;

    case TokenType::Less: {
      func_->add_instruction(Instruction::Less);
    }
      break;

    case TokenType::LessEqual: {
      func_->add_instruction(Instruction::Greater);
      func_->add_instruction(Instruction::Not);
    }
      break;

    case TokenType::Greater: {
      func_->add_instruction(Instruction::Greater);
    }
      break;

    case TokenType::GreaterEqual: {
      func_->add_instruction(Instruction::Less);
      func_->add_instruction(Instruction::Not);
    }
      break;

    case TokenType::EqualEqual: {
      func_->add_instruction(Instruction::Equal);
    }
      break;

    case TokenType::BangEqual: {
      func_->add_instruction(Instruction::Equal);
      func_->add_instruction(Instruction::Not);
    }
      break;

    default:
      break;
    }
    func_->update_line_num_table(expr.op);
  }


  void Compiler::visit_call_expr(const Call& expr)
  {
    const auto callee_is_property = typeid(*expr.callee) == typeid(Get);

    if (callee_is_property) {
      const auto get = static_cast<const Get*>(expr.callee.get());
      compile(*get->object);
    }
    else {
      compile(*expr.callee);
    }

    for (const auto& argument : expr.arguments) {
      compile(*argument);
    }

    if (callee_is_property) {
      const auto get = static_cast<const Get*>(expr.callee.get());
      func_->add_instruction(Instruction::Invoke);
      func_->update_line_num_table(expr.paren);
      func_->add_integer<UByteCodeArg>(
          func_->add_string_constant(get->name.lexeme()));
      func_->add_integer<UByteCodeArg>(expr.arguments.size());
    }
    else {
      func_->add_instruction(Instruction::Call);
      func_->update_line_num_table(expr.paren);
      func_->add_integer<UByteCodeArg>(expr.arguments.size());
    }
  }


  void Compiler::visit_get_expr(const Get& expr)
  {
    compile(*expr.object);

    const auto name_constant = make_string_constant(expr.name.lexeme());
    func_->add_instruction(Instruction::GetProperty);
    func_->add_integer<UByteCodeArg>(name_constant);
    func_->update_line_num_table(expr.name);
  }


  void Compiler::visit_grouping_expr(const Grouping& expr)
  {
    compile(*expr.expression);
  }


  void Compiler::visit_literal_expr(const Literal& expr)
  {
    if (holds_alternative<bool>(expr.value)) {
      const Instruction instruction =
          get<bool>(expr.value) ? Instruction::True : Instruction::False;
      func_->add_instruction(instruction);
      return;
    }
    else if (expr.value.index() == Value::npos) {
      func_->add_instruction(Instruction::Nil);
      return;
    }

    const auto index = func_->add_named_constant(expr.lexeme, expr.value);

    func_->add_instruction(Instruction::LoadConstant);
    func_->add_integer<UByteCodeArg>(index);
  }


  void Compiler::visit_logical_expr(const Logical& expr)
  {
    compile(*expr.left);

    if (expr.op.type() == TokenType::Or) {
      func_->add_instruction(Instruction::ConditionalJump);
      func_->update_line_num_table(expr.op);
      func_->add_integer<ByteCodeArg>(sizeof(ByteCodeArg) + 1);
      func_->add_instruction(Instruction::Jump);
      const auto jump_pos = func_->current_bytecode_size();
      func_->add_integer<ByteCodeArg>(0);

      const auto skip_start = func_->current_bytecode_size();
      compile(*expr.right);
      func_->rewrite_integer<ByteCodeArg>(
          jump_pos, func_->current_bytecode_size() - skip_start);
    }
    else if (expr.op.type() == TokenType::And) {
      func_->add_instruction(Instruction::ConditionalJump);
      func_->update_line_num_table(expr.op);
      const auto jump_pos = func_->current_bytecode_size();
      func_->add_integer<UByteCodeArg>(0);

      const auto skip_start = func_->current_bytecode_size();
      func_->add_instruction(Instruction::Pop);
      compile(*expr.right);

      func_->rewrite_integer<ByteCodeArg>(
          jump_pos, func_->current_bytecode_size() - skip_start);
    }
  }


  void Compiler::visit_set_expr(const Set& expr)
  {
    compile(*expr.object);
    compile(*expr.value);

    const auto name_constant = make_string_constant(expr.name.lexeme());
    func_->add_instruction(Instruction::SetProperty);
    func_->add_integer<UByteCodeArg>(name_constant);
    func_->update_line_num_table(expr.name);
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
    func_->add_instruction(Instruction::GetSuperFunc);
    func_->add_integer<UByteCodeArg>(func);
    func_->update_line_num_table(expr.keyword);
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
      func_->add_instruction(Instruction::Not);
    }
    else if (expr.op.type() == TokenType::Minus) {
      func_->add_instruction(Instruction::Negate);
    }
    func_->update_line_num_table(expr.op);
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
    func_ = std::make_unique<FunctionScope>(type, std::move(func_));
    func_->begin_scope();

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
    if (func_->type() == FunctionType::Initialiser) {
      compile_this_return();
    }

    func_->end_scope();
    // Return "nil" if we haven't returned already.
    func_->add_instruction(Instruction::Nil);
    func_->add_instruction(Instruction::Return);

    const auto upvalues = func_->release_upvalues();
    auto code_object = func_->release_code_object();
    func_ = func_->release_enclosing();

    if (debug_) {
      print_bytecode(stmt.name.lexeme(), *code_object);
    }

    // Add the new function object as a constant
    auto func = make_object<FuncObject>(
        stmt.name.lexeme(), std::move(code_object),
        static_cast<unsigned int>(stmt.parameters.size()),
        upvalues.size());
    const auto index = func_->add_constant(Value(InPlace<ObjectPtr>(), func));

    func_->add_instruction(Instruction::CreateClosure);
    func_->add_integer<UByteCodeArg>(index);
    func_->update_line_num_table(stmt.name);

    for (const auto& upvalue : upvalues) {
      func_->add_integer<UByteCodeArg>(upvalue.is_local ? 1 : 0);
      func_->add_integer<UByteCodeArg>(upvalue.index);
    }
  }


  void Compiler::compile_this_return()
  {
    const auto this_token = func_->make_token(TokenType::This, "this");
    handle_variable_reference(this_token, false);
    func_->add_instruction(Instruction::Return);
    func_->update_line_num_table(this_token);
  }


  Optional<UByteCodeArg> Compiler::declare_variable(const Token& name)
  {
    const Optional<UByteCodeArg> arg =
        func_->scope_depth() == 0 ?
        func_->add_string_constant(name.lexeme()) :
        Optional<UByteCodeArg>();

    if (not arg) {
      func_->declare_local(name);
    }

    return arg;
  }


  void Compiler::define_variable(const Optional<UByteCodeArg>& arg,
                                 const Token& name)
  {
    if (arg) {
      func_->add_instruction(Instruction::DefineGlobal);
      func_->add_integer<UByteCodeArg>(*arg);
      func_->update_line_num_table(name);
    }
    else {
      func_->define_local();
    }
  }


  void Compiler::handle_variable_reference(const Token& token, const bool write)
  {
    auto arg = func_->resolve_local(token, false);

    Instruction op;
    if (arg) {
      op = write ? Instruction::SetLocal : Instruction::GetLocal;
    }
    else {
      arg = func_->resolve_upvalue(token);

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

    func_->add_instruction(op);
    func_->add_integer<UByteCodeArg>(*arg);
    func_->update_line_num_table(token);
  }


  UByteCodeArg Compiler::make_string_constant(const std::string& str) const
  {
    return func_->add_string_constant(str);
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
