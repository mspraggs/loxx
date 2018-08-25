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

#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "Compiler.hpp"
#include "logging.hpp"
#include "ObjectTracker.hpp"
#include "RuntimeError.hpp"
#include "VirtualMachine.hpp"


namespace loxx
{
  VirtualMachine::VirtualMachine(const bool debug)
      : debug_(debug), ip_(0), init_lexeme_(new StringObject("init"))
  {
    NativeObject::Fn fn =
        [] (const Value*, const unsigned int)
        {
          using namespace std::chrono;
          const auto millis =
              system_clock::now().time_since_epoch() / milliseconds(1);

          return Value(static_cast<double>(millis) / 1000.0);
        };

    auto str = make_object<StringObject>("clock");
    globals_[str] =
        Value(InPlace<ObjectPtr>(), make_object<NativeObject>(fn, 0));

    ObjectTracker::instance().set_roots(
        ObjectTracker::Roots{&stack_, &open_upvalues_, &globals_});
  }


  void VirtualMachine::execute(const CodeObject& code_object)
  {
    code_object_ = &code_object;
    ip_ = 0;
    call_stack_.emplace(ip_, code_object_, 0, stack_.data(), nullptr);

    while (ip_ < code_object_->bytecode.size()) {

      if (debug_) {
        print_stack();
        print_instruction(*code_object_, ip_);
      }

      const auto instruction =
          static_cast<Instruction>(code_object_->bytecode[ip_++]);

      switch (instruction) {

      case Instruction::Add: {
        const auto second = stack_.pop();
        const auto first = stack_.pop();

        const auto first_str = get_object<StringObject>(first);
        const auto second_str = get_object<StringObject>(second);

        if (first_str and second_str) {
          const auto combined_str = make_object<StringObject>(
              first_str->as_std_string() + second_str->as_std_string());
          stack_.emplace(InPlace<ObjectPtr>(), combined_str);
        }
        else if (holds_alternative<double>(first) and
                 holds_alternative<double>(second)) {
          stack_.emplace(
              unsafe_get<double>(first) + unsafe_get<double>(second));
        }
        else {
          throw make_runtime_error(
              "Binary operands must be two numbers or two strings.");
        }
        break;
      }

      case Instruction::Call:
        execute_call();
        break;

      case Instruction::CloseUpvalue:
        close_upvalues(stack_.top());
        stack_.discard();
        break;

      case Instruction::ConditionalJump: {
        const auto jmp = read_integer<ByteCodeArg>();
        if (is_truthy(stack_.top())) {
          ip_ += jmp;
        }
        break;
      }

      case Instruction::CreateClass: {
        auto name = read_string()->as_std_string();
        const auto cls = make_object<ClassObject>(std::move(name));
        stack_.emplace(InPlace<ObjectPtr>(), cls);
        break;
      }

      case Instruction::CreateClosure:
        execute_create_closure();
        break;

      case Instruction::CreateMethod: {
        const auto cls = get_object<ClassObject>(stack_.top(1));
        const auto closure = get_object<ClosureObject>(stack_.top());
        const auto name = read_string();

        cls->set_method(name, closure);
        stack_.discard();
        break;
      }

      case Instruction::CreateSubclass: {
        const auto super = get_object<ClassObject>(stack_.top());

        if (not super) {
          throw make_runtime_error("Superclass must be a class.");
        }

        auto name = read_string()->as_std_string();
        const auto cls = make_object<ClassObject>(std::move(name), super);
        stack_.emplace(InPlace<ObjectPtr>(), cls);
        break;
      }

      case Instruction::DefineGlobal: {
        const auto varname = read_string();
        globals_[varname] = stack_.pop();
        break;
      }

      case Instruction::Divide: {
        const auto second = stack_.pop();
        const auto first = stack_.pop();
        check_number_operands(first, second);
        stack_.emplace(unsafe_get<double>(first) / unsafe_get<double>(second));
        break;
      }

      case Instruction::Equal: {
        const auto second = stack_.pop();
        const auto first = stack_.pop();

        stack_.emplace(InPlace<bool>(), are_equal(first, second));
        break;
      }

      case Instruction::False:
        stack_.emplace(InPlace<bool>(), false);
        break;

      case Instruction::GetGlobal: {
        const auto varname = read_string();

        const auto global = globals_.get(varname);

        if (not global) {
          throw make_runtime_error(
              "Undefined variable '" + varname->as_std_string() + "'.");
        }

        stack_.push(global->second);
        break;
      }

      case Instruction::GetLocal: {
        const auto arg = read_integer<UByteCodeArg>();
        stack_.push(call_stack_.top().slot(arg));
        break;
      }

      case Instruction::GetProperty: {
        const auto instance = get_object<InstanceObject>(stack_.top());
        if (not instance) {
          throw make_runtime_error("Only instances have properties.");
        }

        const auto name = read_string();
        const auto& field = instance->field(name);

        if (field) {
          stack_.discard();
          stack_.push(field->second);
        }
        else if (const auto& method = instance->cls().method(name)) {
          const auto new_method =
              make_object<MethodObject>(*method->second, *instance);
          stack_.discard();
          stack_.emplace(InPlace<ObjectPtr>(), new_method);
        }
        else {
          throw make_runtime_error(
              "Undefined property '" + name->as_std_string() + "'.");
        }
        break;
      }

      case Instruction::GetSuperFunc: {
        const auto cls_value = stack_.pop();
        const auto cls = get_object<ClassObject>(cls_value);
        const auto instance = get_object<InstanceObject>(stack_.top());
        const auto name = read_string();

        if (const auto& method_elem = cls->method(name)) {
          auto method =
              make_object<MethodObject>(*method_elem->second, *instance);
          stack_.discard();
          stack_.emplace(InPlace<ObjectPtr>(), method);
        }
        else {
          throw make_runtime_error(
              "Undefined property '" + name->as_std_string() + "'.");
        }
        break;
      }

      case Instruction::GetUpvalue: {
        const auto slot = read_integer<UByteCodeArg>();
        stack_.push(call_stack_.top().closure()->upvalue(slot)->value());
        break;
      }

      case Instruction::Greater: {
        const auto second = stack_.pop();
        const auto first = stack_.pop();
        check_number_operands(first, second);
        stack_.emplace(InPlace<bool>(),
                       unsafe_get<double>(first) > unsafe_get<double>(second));
        break;
      }

      case Instruction::Jump:
        ip_ += read_integer<ByteCodeArg>();
        break;

      case Instruction::Less: {
        const auto second = stack_.pop();
        const auto first = stack_.pop();
        check_number_operands(first, second);
        stack_.emplace(InPlace<bool>(),
                       unsafe_get<double>(first) < unsafe_get<double>(second));
        break;
      }

      case Instruction::LoadConstant:
        stack_.push(code_object_->constants[read_integer<UByteCodeArg>()]);
        break;

      case Instruction::Multiply: {
        const auto second = stack_.pop();
        const auto first = stack_.pop();
        check_number_operands(first, second);
        stack_.emplace(unsafe_get<double>(first) * unsafe_get<double>(second));
        break;
      }

      case Instruction::Negate: {
        if (not holds_alternative<double>(stack_.top())) {
          throw make_runtime_error("Unary operand must be a number.");
        }
        const auto number = unsafe_get<double>(stack_.pop());
        stack_.emplace(-number);
        break;
      }

      case Instruction::Nil:
        stack_.emplace();
        break;

      case Instruction::Not:
        stack_.emplace(InPlace<bool>(), not is_truthy(stack_.pop()));
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
        stack_.discard(stack_.size() - frame.prev_stack_size());
        stack_.push(result);
        code_object_ = frame.prev_code_object();
        ip_ = frame.prev_ip();
        break;
      }

      case Instruction::SetGlobal: {
        const auto varname = read_string();

        if (globals_.count(varname) == 0) {
          throw make_runtime_error(
              "Undefined variable '" + varname->as_std_string() + "'.");
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
          throw make_runtime_error("Only instances have fields.");
        }

        const auto name = read_string();

        obj->set_field(name, stack_.top());
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

      case Instruction::Subtract: {
        const auto second = stack_.pop();
        const auto first = stack_.pop();
        check_number_operands(first, second);
        stack_.emplace(unsafe_get<double>(first) - unsafe_get<double>(second));
        break;
      }

      case Instruction::True:
        stack_.emplace(InPlace<bool>(), true);
        break;

      default:
        std::cout << "Unknown instruction: "
                  << static_cast<unsigned int>(instruction) << std::endl;
        break;
      }
    }
  }


  void VirtualMachine::print_object(Value variant) const
  {
    std::cout << variant << std::endl;
  }


  void VirtualMachine::execute_call()
  {
    const auto num_args = read_integer<UByteCodeArg>();

    if (not holds_alternative<ObjectPtr>(stack_.top(num_args))) {
      throw make_runtime_error("Can only call functions and classes.");
    }

    const auto obj = unsafe_get<ObjectPtr>(stack_.top(num_args));

    switch (obj->type()) {
      case ObjectType::Class: {
        const auto cls = static_cast<ClassObject*>(obj);
        unsafe_get<ObjectPtr>(stack_.top(num_args)) =
            make_object<InstanceObject>(cls);

        if (const auto& method = cls->method(init_lexeme_.get())) {
          call(method->second, num_args);
          break;
        }

        if (num_args != 0) {
          std::stringstream ss;
          ss << "Expected 0 arguments but got " << num_args << '.';
          throw make_runtime_error(ss.str());
        }
        break;
      }

      case ObjectType::Closure: {
        call(static_cast<ClosureObject*>(obj), num_args);
        break;
      }

      case ObjectType::Method: {
        const auto method = static_cast<MethodObject*>(obj);
        unsafe_get<ObjectPtr>(stack_.top(num_args)) = method->instance();
        call(method->closure(), num_args);
        break;
      }

      case ObjectType::Native: {
        const auto native = static_cast<NativeObject*>(obj);

        if (native->arity() != num_args) {
          std::stringstream ss;
          ss << "Expected " << native->arity() << " arguments but got "
             << num_args << '.';
          throw make_runtime_error(ss.str());
        }

        const auto result = native->call(&stack_.top(num_args),
                                         static_cast<unsigned int>(num_args));
        stack_.discard(num_args);
        stack_.top(num_args) = result;

        break;
      }

      default:
        throw make_runtime_error("Can only call functions and classes.");
    }
  }


  void VirtualMachine::execute_create_closure()
  {
    const auto& func_value =
        code_object_->constants[read_integer<UByteCodeArg>()];
    const auto& func_obj = unsafe_get<ObjectPtr>(func_value);
    auto func = static_cast<FuncObject*>(func_obj);

    auto closure = make_object<ClosureObject>(func);

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

    stack_.push(Value(InPlace<ObjectPtr>(), closure));
  }


  UpvalueObject* VirtualMachine::capture_upvalue(Value& local)
  {
    if (open_upvalues_.empty()) {
      open_upvalues_.push_back(make_object<UpvalueObject>(local));
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
        open_upvalues_.insert(upvalue, make_object<UpvalueObject>(local));

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


  void VirtualMachine::call(ClosureObject* closure,
                            const std::size_t num_args)
  {
    if (closure->function().arity() != num_args) {
      std::stringstream ss;
      ss << "Expected " << closure->function().arity()
         << " arguments but got " << num_args << '.';
      throw make_runtime_error(ss.str());
    }

    call_stack_.emplace(ip_, code_object_, stack_.size() - num_args - 1,
                        stack_.top(num_args), closure);
    code_object_ = closure->function().code_object();
    ip_ = 0;
  }


  void VirtualMachine::print_stack() const
  {
    std::cout << "          ";

    for (unsigned int i = 0; i < stack_.size(); ++i) {
      std::cout << "[ ";
      std::cout << stack_.get(i);
      std::cout << " ] ";
    }
    std::cout << std::endl;
  }


  loxx::StringObject* VirtualMachine::read_string()
  {
    return get_object<StringObject>(
        code_object_->constants[read_integer<UByteCodeArg>()]);
  }


  void VirtualMachine::check_number_operands(
      const Value& first, const Value& second) const
  {
    if (not holds_alternative<double>(first) or
        not holds_alternative<double>(second)) {
      throw make_runtime_error("Binary operands must both be numbers.");
    }
  }


  bool VirtualMachine::are_equal(const Value& first,
                                 const Value& second) const
  {
    if (first.index()  == Value::npos and
        second.index() == Value::npos) {
      return true;
    }

    return first == second;
  }


  bool VirtualMachine::is_truthy(const Value& value) const
  {
    if (value.index() == Value::npos) {
      return false;
    }
    if (holds_alternative<bool>(value)) {
      return unsafe_get<bool>(value);
    }
    return true;
  }


  RuntimeError VirtualMachine::make_runtime_error(const std::string& msg) const
  {
    return RuntimeError(get_current_line(*code_object_, ip_), msg);
  }
}
