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
 * Created by Matt Spraggs on 05/03/2018.
 */

#include <iomanip>
#include <iostream>
#include <sstream>

#include "Compiler.hpp"
#include "logging.hpp"
#include "RuntimeError.hpp"
#include "VirtualMachine.hpp"


namespace loxx
{
  template <>
  std::shared_ptr<ClassObject> VirtualMachine::get_object(const Value& value);


  template <>
  std::shared_ptr<ClosureObject> VirtualMachine::get_object(const Value& value);


  template <>
  std::shared_ptr<InstanceObject> VirtualMachine::get_object(
      const Value& value);


  VirtualMachine::VirtualMachine(const bool debug)
      : debug_(debug), ip_(0)
  {
  }


  void VirtualMachine::execute(const CompilationOutput& compiler_output)
  {
    compiler_output_ = &compiler_output;

    if (debug_) {
      disassemble_bytecode();
    }

    ip_ = 0;
    call_stack_.push(StackFrame(ip_, 0, stack_.data(), nullptr));

    while (ip_ < compiler_output.bytecode.size()) {

      if (debug_) {
        print_stack();
        disassemble_instruction();
      }

      const auto instruction =
          static_cast<Instruction>(compiler_output.bytecode[ip_++]);

      switch (instruction) {

      case Instruction::Add:
      case Instruction::Subtract:
      case Instruction::Multiply:
      case Instruction::Divide:
      case Instruction::Equal:
      case Instruction::NotEqual:
      case Instruction::Greater:
      case Instruction::GreaterEqual:
      case Instruction::Less:
      case Instruction::LessEqual:
        execute_binary_op(instruction);
        break;

      case Instruction::Call:
        execute_call();
        break;

      case Instruction::CloseUpvalue:
        close_upvalues(stack_.top());
        stack_.pop();
        break;

      case Instruction::ConditionalJump: {
        const auto jmp = read_integer<ByteCodeArg>();
        if (is_truthy(stack_.top())) {
          ip_ += jmp;
        }
        break;
      }

      case Instruction::CreateClass: {
        auto name = get<std::string>(constants_[read_integer<UByteCodeArg>()]);
        stack_.push(std::make_shared<ClassObject>(std::move(name)));
        break;
      }

      case Instruction::CreateClosure:
        execute_create_closure();
        break;

      case Instruction::CreateMethod: {
        const auto cls = get_object<ClassObject>(stack_.top(1));
        auto closure = get_object<ClosureObject>(stack_.top());
        const auto& name =
            get<std::string>(constants_[read_integer<UByteCodeArg>()]);

        cls->set_method(name, std::move(closure));
        stack_.pop();
        break;
      }

      case Instruction::CreateSubclass: {
        auto super = get_object<ClassObject>(stack_.top());

        if (not super) {
          throw RuntimeError(get_current_line(), "Superclass must be a class.");
        }

        auto name = get<std::string>(constants_[read_integer<UByteCodeArg>()]);
        stack_.push(std::make_shared<ClassObject>(std::move(name),
                                                  std::move(super)));
        break;
      }

      case Instruction::DefineGlobal: {
        const auto arg = read_integer<UByteCodeArg>();
        const auto varname = get<std::string>(constants_[arg]);
        globals_[varname] = stack_.pop();
        break;
      }

      case Instruction::False:
        stack_.push(Value(InPlace<bool>(), false));
        break;

      case Instruction::GetGlobal: {
        const auto arg = read_integer<UByteCodeArg>();
        const auto varname = get<std::string>(constants_[arg]);

        if (globals_.count(varname) != 1) {
          throw RuntimeError(get_current_line(),
                             "Undefined variable '" + varname + "'.");
        }

        stack_.push(globals_[varname]);
        break;
      }

      case Instruction::GetLocal: {
        const auto arg = read_integer<UByteCodeArg>();
        stack_.push(call_stack_.top().slot(arg));
        break;
      }

      case Instruction::GetProperty: {
        auto instance = get_object<InstanceObject>(stack_.top());
        if (not instance) {
          throw RuntimeError(get_current_line(),
                             "Only instances have properties.");
        }

        const auto& name =
            get<std::string>(constants_[read_integer<UByteCodeArg>()]);

        if (instance->has_field(name)) {
          stack_.pop();
          stack_.push(instance->field(name));
        }
        else if (instance->cls().has_method(name)) {
          auto method = instance->cls().method(name);
          method->bind(std::move(instance));
          stack_.pop();
          stack_.push(method);
        }
        else {
          throw RuntimeError(get_current_line(),
                             "Undefined property '" + name + "'.");
        }
        break;
      }

      case Instruction::GetSuperFunc: {
        const auto cls_value = stack_.pop();
        const auto cls = get_object<ClassObject>(cls_value);
        auto instance = get_object<InstanceObject>(stack_.top());
        const auto& name =
            get<std::string>(constants_[read_integer<UByteCodeArg>()]);

        if (cls->has_method(name)) {
          auto method = cls->method(name);
          method->bind(std::move(instance));
          stack_.pop();
          stack_.push(method);
        }
        else {
          throw RuntimeError(get_current_line(),
                             "Undefined property '" + name + "'.");
        }
        break;
      }

      case Instruction::GetUpvalue: {
        const auto slot = read_integer<UByteCodeArg>();
        stack_.push(call_stack_.top().closure()->upvalue(slot)->value());
        break;
      }

      case Instruction::Jump:
        ip_ += read_integer<ByteCodeArg>();
        break;

      case Instruction::LoadConstant:
        stack_.push(constants_[read_integer<UByteCodeArg>()]);
        break;

      case Instruction::Negate: {
        if (not holds_alternative<double>(stack_.top())) {
          throw RuntimeError(get_current_line(),
                             "Unary operand must be a number.");
        }
        const auto number = get<double>(stack_.pop());
        stack_.push(-number);
        break;
      }

      case Instruction::Nil:
        stack_.push(Value());
        break;

      case Instruction::Not:
        stack_.push(Value(InPlace<bool>(), not is_truthy(stack_.pop())));
        break;

      case Instruction::Pop:
        stack_.pop();
        break;

      case Instruction::Print:
        print_object(stack_.pop());
        break;

      case Instruction::Return: {
        const auto result = stack_.pop();
        close_upvalues(call_stack_.top().slot(0));
        const auto frame = call_stack_.pop();
        while (stack_.size() != frame.prev_stack_size()) {
          stack_.pop();
        }
        stack_.push(result);
        ip_ = frame.prev_ip();
        break;
      }

      case Instruction::SetGlobal: {
        const auto arg = read_integer<UByteCodeArg>();
        const auto varname = get<std::string>(constants_[arg]);

        if (globals_.count(varname) == 0) {
          throw RuntimeError(get_current_line(),
                             "Undefined variable '" + varname + "'.");
        }

        globals_[varname] = stack_.top();
        break;
      }

      case Instruction::SetLocal: {
        const auto arg = read_integer<UByteCodeArg>();
        call_stack_.top().slot(arg) = stack_.top();
        break;
      }

      case Instruction::SetProperty: {
        const auto obj = get_object<InstanceObject>(stack_.top(1));
        if (not obj) {
          throw RuntimeError(get_current_line(),
                             "Only instances have fields.");
        }

        const auto instance = std::static_pointer_cast<InstanceObject>(obj);
        const auto& name =
            get<std::string>(constants_[read_integer<UByteCodeArg>()]);

        instance->set_field(name, stack_.top());
        const auto value = stack_.pop();
        stack_.pop();
        stack_.push(value);
        break;
      }

      case Instruction::SetUpvalue: {
        const auto slot = read_integer<UByteCodeArg>();
        call_stack_.top().closure()->upvalue(slot)->set_value(stack_.top());
        break;
      }

      case Instruction::True:
        stack_.push(Value(InPlace<bool>(), true));
        break;

      default:
        std::cout << "Unknown instruction: "
                  << static_cast<unsigned int>(instruction) << std::endl;
        break;
      }
    }
  }


  UByteCodeArg VirtualMachine::add_named_constant(const std::string& lexeme,
                                                  const Value& value)
  {
    if (constant_map_.count(lexeme) != 0) {
      return constant_map_[lexeme];
    }

    const auto index = static_cast<UByteCodeArg>(constants_.size());

    constants_.push_back(value);
    constant_map_[lexeme] = index;

    return index;
  }


  UByteCodeArg VirtualMachine::add_constant(const Value& value)
  {
    constants_.push_back(value);
    return constants_.size() - 1;
  }


  void VirtualMachine::print_object(Value variant) const
  {
    std::cout << stringify(variant) << std::endl;
  }


  void VirtualMachine::execute_binary_op(const Instruction instruction)
  {
    const auto second = stack_.pop();
    const auto first = stack_.pop();

    switch (instruction) {
    case Instruction::NotEqual:
      stack_.push(Value(InPlace<bool>(), not are_equal(first, second)));
      break;
    case Instruction::Equal:
      stack_.push(Value(InPlace<bool>(), are_equal(first, second)));
      break;
    case Instruction::Less:
      check_number_operands(first, second);
      stack_.push(Value(InPlace<bool>(),
                           get<double>(first) < get<double>(second)));
      break;
    case Instruction::LessEqual:
      check_number_operands(first, second);
      stack_.push(Value(InPlace<bool>(),
                           get<double>(first) <= get<double>(second)));
      break;
    case Instruction::Greater:
      check_number_operands(first, second);
      stack_.push(Value(InPlace<bool>(),
                           get<double>(first) > get<double>(second)));
      break;
    case Instruction::GreaterEqual:
      check_number_operands(first, second);
      stack_.push(Value(InPlace<bool>(),
                           get<double>(first) >= get<double>(second)));
      break;
    case Instruction::Multiply:
      check_number_operands(first, second);
      stack_.push(get<double>(first) * get<double>(second));
      break;
    case Instruction::Divide:
      check_number_operands(first, second);
      stack_.push(get<double>(first) / get<double>(second));
      break;
    case Instruction::Subtract:
      check_number_operands(first, second);
      stack_.push(get<double>(first) - get<double>(second));
      break;
    case Instruction::Add:
      if (holds_alternative<std::string>(first) and
          holds_alternative<std::string>(second))
      {
        stack_.push(get<std::string>(first) + get<std::string>(second));
      }
      else if (holds_alternative<double>(first) and
               holds_alternative<double>(second))
      {
        stack_.push(get<double>(first) + get<double>(second));
      }
      else {
        throw RuntimeError(
            get_current_line(),
            "Binary operands must be two numbers or two strings.");
      }
      break;
    default:
      break;
    }
  }


  void VirtualMachine::execute_call()
  {
    const auto num_args = read_integer<UByteCodeArg>();
    const auto obj_pos = stack_.size() - num_args - 1;
    auto obj = get_object(stack_.get(obj_pos),
                          {ObjectType::Class, ObjectType::Closure});

    if (not obj) {
      throw RuntimeError(get_current_line(),
                         "Can only call functions and classes.");
    }

    call_object(std::move(obj), obj_pos, num_args);
  }


  void VirtualMachine::execute_create_closure()
  {
    const auto& func_value = constants_[read_integer<UByteCodeArg>()];
    const auto& func_obj = get<std::shared_ptr<Object>>(func_value);
    auto func = std::static_pointer_cast<FuncObject>(func_obj);

    auto closure = std::make_shared<ClosureObject>(std::move(func));

    for (unsigned int i = 0; i < closure->num_upvalues(); ++i) {
      const auto is_local = read_integer<UByteCodeArg>() != 0;
      const auto index = read_integer<UByteCodeArg>();

      if (is_local) {
        closure->set_upvalue(
            i, capture_upvalue(call_stack_.top().slot(index)));
      }
      else {
        closure->set_upvalue(
            i, call_stack_.top().closure()->upvalue(index));
      }
    }

    stack_.push(closure);
  }


  std::shared_ptr<UpvalueObject> VirtualMachine::capture_upvalue(Value& local)
  {
    if (open_upvalues_.empty()) {
      open_upvalues_.push_back(std::make_shared<UpvalueObject>(local));
      return open_upvalues_.back();
    }

    auto upvalue = open_upvalues_.end();

    for (auto it = open_upvalues_.begin(); it != open_upvalues_.end(); ++it) {
      if (&(*it)->value() == &local) {
        return *it;
      }
      else if (&(*it)->value() < &local) {
        upvalue = it;
        break;
      }
    }

    const auto new_upvalue =
        open_upvalues_.insert(upvalue, std::make_shared<UpvalueObject>(local));

    return *new_upvalue;
  }


  void VirtualMachine::close_upvalues(Value& last)
  {
    auto it = open_upvalues_.begin();

    while (it != open_upvalues_.end() and &(*it)->value() >= &last) {
      (*it)->close();

      ++it;
      open_upvalues_.pop_front();
    }
  }


  void VirtualMachine::call_object(
      std::shared_ptr<Object> obj, const std::size_t obj_pos,
      const UByteCodeArg num_args)
  {
    switch (obj->type()) {
    case ObjectType::Class: {
      const auto cls = std::static_pointer_cast<ClassObject>(obj);
      auto instance = std::make_shared<InstanceObject>(cls);
      stack_.get(obj_pos) = instance;

      if (cls->has_method("init")) {
        auto method = cls->method("init");
        method->bind(std::move(instance));
        call_object(std::move(method), obj_pos, num_args);
        break;
      }

      if (num_args != 0) {
        std::stringstream ss;
        ss << "Expected 0 arguments but got " << num_args << '.';
        throw RuntimeError(get_current_line(), ss.str());
      }
      break;
    }

    case ObjectType::Closure: {
      const auto closure = std::static_pointer_cast<ClosureObject>(obj);

      if (closure->function().arity() != num_args) {
        std::stringstream ss;
        ss << "Expected " << closure->function().arity()
           << " arguments but got " << num_args << '.';
        throw RuntimeError(get_current_line(), ss.str());
      }

      if (closure->instance()) {
        stack_.get(obj_pos) = closure->instance();
      }
      auto& base_slot = stack_.get(obj_pos + (closure->instance() ? 0 : 1));

      call_stack_.push(StackFrame(ip_, stack_.size() - num_args - 1,
                                  base_slot, closure));
      ip_ = closure->function().bytecode_offset();
      break;
    }

    default:
      break;
    }
  }


  std::string VirtualMachine::stringify(const Value& object) const
  {
    if (object.index() == Value::npos) {
      return "nil";
    }
    if (holds_alternative<double>(object)) {
      std::stringstream ss;
      ss << get<double>(object);
      return ss.str();
    }
    else if (holds_alternative<bool>(object)) {
      return get<bool>(object) ? "true" : "false";
    }
    else if (holds_alternative<std::string>(object)) {
      return get<std::string>(object);
    }
    else if (holds_alternative<std::shared_ptr<Object>>(object)) {
      const auto ptr = get<std::shared_ptr<Object>>(object);

      std::stringstream ss;

      switch (ptr->type()) {
      case ObjectType::Function:
        ss << "<fn " << std::static_pointer_cast<FuncObject>(ptr)->lexeme()
           << '>';
        break;
      case ObjectType::Class: {
        const auto cls = std::static_pointer_cast<ClassObject>(ptr);
        ss << "<class " << cls->lexeme() << '>';
        break;
      }
      case ObjectType::Closure: {
        const auto closure = std::static_pointer_cast<ClosureObject>(ptr);
        ss << "<fn " << closure->function().lexeme() << '>';
        break;
      }
      case ObjectType::Instance: {
        const auto instance = std::static_pointer_cast<InstanceObject>(ptr);
        ss << instance->cls().lexeme() << " instance";
      }
      default:
        break;
      }
      return ss.str();
    }

    return "";
  }


  void VirtualMachine::print_stack() const
  {
    std::cout << "          ";

    for (unsigned int i = 0; i < stack_.size(); ++i) {
      std::cout << "[ ";
      std::cout << stringify(stack_.get(i));
      std::cout << " ] ";
    }
    std::cout << std::endl;
  }


  void VirtualMachine::disassemble_bytecode()
  {
    ip_ = 0;
    while (ip_ < compiler_output_->bytecode.size()) {
      ip_ = disassemble_instruction();
    }
  }


  size_t VirtualMachine::disassemble_instruction() const
  {
    const auto instruction =
        static_cast<Instruction>(compiler_output_->bytecode[ip_]);

    static unsigned int last_line_num = 0;
    const unsigned int current_line_num = get_current_line();

    std::stringstream line_num_ss;
    line_num_ss << std::right << std::setw(5) << std::setfill(' ');
    if (last_line_num < current_line_num) {
      line_num_ss << current_line_num;
    }
    else {
      line_num_ss << '|';
    }
    last_line_num = current_line_num;

    std::cout << std::setw(4) << std::setfill('0') << std::right << ip_;
    std::cout << line_num_ss.str() << ' ';
    std::cout << std::setw(20) << std::setfill(' ') << std::left << instruction;

    std::size_t ret = ip_ + 1;

    switch (instruction) {

    case Instruction::Add:
    case Instruction::CloseUpvalue:
    case Instruction::Divide:
    case Instruction::Equal:
    case Instruction::False:
    case Instruction::Greater:
    case Instruction::GreaterEqual:
    case Instruction::Less:
    case Instruction::LessEqual:
    case Instruction::Multiply:
    case Instruction::Negate:
    case Instruction::Nil:
    case Instruction::Not:
    case Instruction::NotEqual:
    case Instruction::Pop:
    case Instruction::Print:
    case Instruction::Push:
    case Instruction::Return:
    case Instruction::SetBase:
    case Instruction::Subtract:
    case Instruction::True:
      break;

    case Instruction::ConditionalJump:
    case Instruction::Jump: {
      const auto param = read_integer_at_pos<ByteCodeArg>(ret);
      std::cout << ip_ << " -> " << ip_ + param + sizeof(ByteCodeArg) + 1;
      ret += sizeof(ByteCodeArg);
      break;
    }

    case Instruction::CreateClosure: {
      const auto& func_value =
          constants_[read_integer_at_pos<UByteCodeArg>(ret)];
      const auto& func_obj = get<std::shared_ptr<Object>>(func_value);
      auto func = std::static_pointer_cast<FuncObject>(func_obj);

      ret += sizeof(UByteCodeArg);

      std::cout << func->lexeme() << ' ';

      for (unsigned int i = 0; i < func->num_upvalues(); ++i) {
        const auto is_local = read_integer_at_pos<UByteCodeArg>(ret) != 0;
        ret += sizeof(UByteCodeArg);
        const auto index = read_integer_at_pos<UByteCodeArg>(ret);
        ret += sizeof(UByteCodeArg);

        std::cout << '(' << (is_local ? "local" : "upvalue") << ", "
                  << index << ')';

        if (i < func->num_upvalues() - 1) {
          std::cout << ", ";
        }
      }

      break;
    }

    case Instruction::Call: {
      const auto num_args = read_integer_at_pos<UByteCodeArg>(ret);
      ret += sizeof(UByteCodeArg);
      std::cout << num_args;
      break;
    }

    case Instruction::CreateClass:
    case Instruction::CreateMethod:
    case Instruction::CreateSubclass:
    case Instruction::DefineGlobal:
    case Instruction::GetGlobal:
    case Instruction::GetLocal:
    case Instruction::GetProperty:
    case Instruction::GetSuperFunc:
    case Instruction::GetUpvalue:
    case Instruction::SetGlobal:
    case Instruction::SetLocal:
    case Instruction::SetProperty:
    case Instruction::SetUpvalue:
    case Instruction::LoadConstant: {
      const auto param = read_integer_at_pos<UByteCodeArg>(ret);
      std::cout << param << " '" << stringify(constants_[param]) << "'";
      ret += sizeof(UByteCodeArg);
      break;
    }
    }

    std::cout << '\n';

    return ret;
  }


  std::shared_ptr<Object> VirtualMachine::get_object(
      const Value& value, const std::vector<ObjectType>& valid_types)
  {
    if (holds_alternative<std::shared_ptr<Object>>(value)) {
      const auto obj_ptr = get<std::shared_ptr<Object>>(value);

      for (auto type : valid_types) {
        if (obj_ptr->type() == type) {
          return obj_ptr;
        }
      }
    }
    return std::shared_ptr<Object>();
  }


  template <>
  std::shared_ptr<ClassObject> VirtualMachine::get_object(const Value& value)
  {
    return std::static_pointer_cast<ClassObject>(
        get_object_impl(value, ObjectType::Class));
  }


  template <>
  std::shared_ptr<ClosureObject> VirtualMachine::get_object(const Value& value)
  {
    return std::static_pointer_cast<ClosureObject>(
        get_object_impl(value, ObjectType::Closure));
  }


  template <>
  std::shared_ptr<InstanceObject> VirtualMachine::get_object(const Value& value)
  {
    return std::static_pointer_cast<InstanceObject>(
        get_object_impl(value, ObjectType::Instance));
  }


  std::shared_ptr<Object> VirtualMachine::get_object_impl(
      const loxx::Value& value, loxx::ObjectType type)
  {
    if (holds_alternative<std::shared_ptr<Object>>(value)) {
      const auto obj_ptr = get<std::shared_ptr<Object>>(value);

      if (obj_ptr->type() == type) {
        return obj_ptr;
      }
    }

    return {};
  }


  void VirtualMachine::check_number_operands(
      const Value& first, const Value& second) const
  {
    if (not holds_alternative<double>(first) or
        not holds_alternative<double>(second)) {
      throw RuntimeError(get_current_line(),
                         "Binary operands must both be numbers.");
    }
  }


  bool VirtualMachine::are_equal(const Value& first,
                                 const Value& second) const
  {
    if (first.index()  == Value::npos and
        second.index() == Value::npos) {
      return true;
    }
    if (first.index() == Value::npos) {
      return false;
    }

    return first == second;
  }


  bool VirtualMachine::is_truthy(const Value& value) const
  {
    if (value.index() == Value::npos) {
      return false;
    }
    if (holds_alternative<bool>(value)) {
      return get<bool>(value);
    }
    return true;
  }


  unsigned int VirtualMachine::get_current_line() const
  {
    std::size_t instruction_counter = 0;
    unsigned int line = 0;

    for (const auto& table_row : compiler_output_->line_num_table) {
      instruction_counter += std::get<1>(table_row);
      line += std::get<0>(table_row);

      if (instruction_counter >= ip_) {
        break;
      }
    }

    return line;
  }
}
