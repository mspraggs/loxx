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
#include <iterator>
#include <sstream>

#include "logging.hpp"
#include "ObjectTracker.hpp"
#include "RuntimeError.hpp"
#include "VirtualMachine.hpp"


namespace loxx
{
  VirtualMachine::VirtualMachine(CodeProfiler* profiler, const bool debug)
      : debug_(debug), ip_(0),
        init_lexeme_(make_string("init")),
        profiler_(profiler)
  {
    NativeObject::Fn fn =
        [] (const Value*, const unsigned int)
        {
          using namespace std::chrono;
          const auto millis =
              system_clock::now().time_since_epoch() / milliseconds(1);

          return Value(static_cast<double>(millis) / 1000.0);
        };

    auto str = make_string("clock");
    globals_[str] =
        Value(InPlace<ObjectPtr>(), make_object<NativeObject>(fn, 0));

    ObjTracker::instance().set_roots(stack_, open_upvalues_, globals_);
  }


  void VirtualMachine::execute(std::unique_ptr<CodeObject> code_object)
  {
    const auto top_level_func =
        std::make_unique<FuncObject>("top level", std::move(code_object), 0, 0);
    const auto top_level_closure =
        std::make_unique<ClosureObject>(top_level_func.get());

    code_object_ = top_level_func->code_object();
    ip_ = top_level_func->code_object()->bytecode.begin();
    call_stack_.emplace(ip_, code_object_, stack_.data(),
                        top_level_closure.get());

    while (true) {

#ifndef NDEBUG
      if (debug_) {
        std::cout << "          " << stack_ << std::endl;
        print_instruction(
            *call_stack_.top().closure()->function().code_object(), ip_);
      }
#endif

      const auto init_ip = ip_;
      if (profiler_->block_start_flagged()) {
        profiler_->count_basic_block(code_object.get(), init_ip);
      }
      const auto instruction = static_cast<Instruction>(*ip_++);

      switch (instruction) {

      case Instruction::ADD: {
        const auto second = stack_.pop();
        const auto first = stack_.pop();

        const auto first_str = get_object<StringObject>(first);
        const auto second_str = get_object<StringObject>(second);

        if (first_str and second_str) {
          const auto combined_str = make_string(
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

        profiler_->profile_instruction(init_ip, stack_.top(), first, second);
        break;
      }

      case Instruction::CALL:
        execute_call();
        break;

      case Instruction::CLOSE_UPVALUE:
        close_upvalues(stack_.top());
        stack_.discard();
        break;

      case Instruction::CONDITIONAL_JUMP: {
        const auto jmp = read_integer<InstrArgUShort>();
        if (not is_truthy(stack_.top())) {
          ip_ += jmp;
        }
        profiler_->flag_block_start();
        break;
      }

      case Instruction::CREATE_CLASS: {
        auto name = read_string()->as_std_string();
        const auto cls = make_object<ClassObject>(std::move(name));
        stack_.emplace(InPlace<ObjectPtr>(), cls);
        break;
      }

      case Instruction::CREATE_CLOSURE:
        execute_create_closure();
        break;

      case Instruction::CREATE_METHOD: {
        const auto cls = get_object<ClassObject>(stack_.top(1));
        const auto closure = get_object<ClosureObject>(stack_.top());
        const auto name = read_string();

        cls->set_method(name, closure);
        stack_.discard();
        break;
      }

      case Instruction::CREATE_SUBCLASS: {
        const auto super = get_object<ClassObject>(stack_.top());

        if (not super) {
          throw make_runtime_error("Superclass must be a class.");
        }

        auto name = read_string()->as_std_string();
        const auto cls = make_object<ClassObject>(std::move(name), super);
        stack_.emplace(InPlace<ObjectPtr>(), cls);
        break;
      }

      case Instruction::DEFINE_GLOBAL: {
        const auto varname = read_string();
        globals_[varname] = stack_.pop();
        break;
      }

      case Instruction::DIVIDE: {
        const auto second = stack_.pop();
        const auto first = stack_.pop();
        check_number_operands(first, second);
        stack_.emplace(unsafe_get<double>(first) / unsafe_get<double>(second));
        break;
      }

      case Instruction::EQUAL: {
        const auto second = stack_.pop();
        const auto first = stack_.pop();

        stack_.emplace(InPlace<bool>(), are_equal(first, second));
        break;
      }

      case Instruction::FALSE:
        stack_.emplace(InPlace<bool>(), false);
        break;

      case Instruction::GET_GLOBAL: {
        const auto varname = read_string();

        const auto& global = globals_.get(varname);

        if (not global) {
          throw make_runtime_error(
              "Undefined variable '" + varname->as_std_string() + "'.");
        }

        stack_.push(global->second);
        break;
      }

      case Instruction::GET_LOCAL: {
        const auto arg = read_integer<InstrArgUByte>();
        stack_.push(call_stack_.top().slot(arg));
        break;
      }

      case Instruction::GET_PROPERTY: {
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

      case Instruction::GET_SUPER_FUNC: {
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

      case Instruction::GET_UPVALUE: {
        const auto slot = read_integer<InstrArgUByte>();
        stack_.push(call_stack_.top().closure()->upvalue(slot)->value());
        break;
      }

      case Instruction::GREATER: {
        const auto second = stack_.pop();
        const auto first = stack_.pop();
        check_number_operands(first, second);
        stack_.emplace(InPlace<bool>(),
                       unsafe_get<double>(first) > unsafe_get<double>(second));
        break;
      }

      case Instruction::INVOKE: {
        const auto name = read_string();
        const auto num_args = read_integer<InstrArgUByte>();

        const auto instance = get_object<InstanceObject>(stack_.top(num_args));
        if (not instance) {
          throw make_runtime_error("Only instances have methods.");
        }

        const auto& field = instance->field(name);

        if (field) {
          if (not holds_alternative<ObjectPtr>(field->second)) {
            throw make_runtime_error("Can only call functions and classes.");
          }
          call_object(num_args, unsafe_get<ObjectPtr>(field->second));
        }
        else if (const auto& method = instance->cls().method(name)) {
          call_object(num_args, method->second);
        }
        else {
          throw make_runtime_error(
              "Undefined property '" + name->as_std_string() + "'.");
        }
        break;
      }

      case Instruction::JUMP:
        ip_ += read_integer<InstrArgUShort>();
        profiler_->flag_block_start();
        break;

      case Instruction::LESS: {
        const auto second = stack_.pop();
        const auto first = stack_.pop();
        check_number_operands(first, second);
        stack_.emplace(InPlace<bool>(),
                       unsafe_get<double>(first) < unsafe_get<double>(second));
        break;
      }

      case Instruction::LOAD_CONSTANT:
        stack_.push(read_constant());
        break;

      case Instruction::LOOP:
        ip_ -= read_integer<InstrArgUShort>();
        profiler_->flag_block_start();
        break;

      case Instruction::MULTIPLY: {
        const auto second = stack_.pop();
        const auto first = stack_.pop();
        check_number_operands(first, second);
        stack_.emplace(unsafe_get<double>(first) * unsafe_get<double>(second));
        break;
      }

      case Instruction::NEGATE: {
        if (not holds_alternative<double>(stack_.top())) {
          throw make_runtime_error("Unary operand must be a number.");
        }
        const auto number = unsafe_get<double>(stack_.pop());
        stack_.emplace(-number);
        break;
      }

      case Instruction::NIL:
        stack_.emplace();
        break;

      case Instruction::NOT:
        stack_.emplace(InPlace<bool>(), not is_truthy(stack_.pop()));
        break;

      case Instruction::POP:
        stack_.pop();
        break;

      case Instruction::PRINT:
        std::cout << stack_.pop() << std::endl;
        break;

      case Instruction::PROFILE_TYPE:
        profiler_->count_variable_type(
            code_object_, read_string(),
            static_cast<ValueType>(stack_.top().index()));
        break;

      case Instruction::RETURN: {
        const auto result = stack_.pop();
        close_upvalues(call_stack_.top().slot(0));
        const auto frame = call_stack_.pop();

        if (call_stack_.size() == 0) {
          return;
        }

        // We add one here to discard the function that was previously called
        stack_.discard(
            static_cast<std::size_t>(&stack_.top() - &frame.slot(0)) + 1);
        stack_.push(result);
        code_object_ = frame.prev_code_object();
        ip_ = frame.prev_ip();
        break;
      }

      case Instruction::SET_GLOBAL: {
        const auto varname = read_string();

        if (globals_.count(varname) == 0) {
          throw make_runtime_error(
              "Undefined variable '" + varname->as_std_string() + "'.");
        }

        globals_[varname] = stack_.top();
        break;
      }

      case Instruction::SET_LOCAL: {
        const auto arg = read_integer<InstrArgUByte>();
        call_stack_.top().slot(arg) = stack_.top();
        break;
      }

      case Instruction::SET_PROPERTY: {
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

      case Instruction::SET_UPVALUE: {
        const auto slot = read_integer<InstrArgUByte>();
        call_stack_.top().closure()->upvalue(slot)->set_value(stack_.top());
        break;
      }

      case Instruction::SUBTRACT: {
        const auto second = stack_.pop();
        const auto first = stack_.pop();
        check_number_operands(first, second);
        stack_.emplace(unsafe_get<double>(first) - unsafe_get<double>(second));
        break;
      }

      case Instruction::TRUE:
        stack_.emplace(InPlace<bool>(), true);
        break;

      default:
        std::cout << "Unknown instruction: "
                  << static_cast<unsigned int>(instruction) << std::endl;
        break;
      }
    }
  }


  void VirtualMachine::execute_call()
  {
    const auto num_args = read_integer<InstrArgUByte>();

    if (not holds_alternative<ObjectPtr>(stack_.top(num_args))) {
      throw make_runtime_error("Can only call functions and classes.");
    }

    const auto obj = unsafe_get<ObjectPtr>(stack_.top(num_args));

    call_object(num_args, obj);
  }


  void VirtualMachine::call_object(
      const InstrArgUByte num_args, const ObjectPtr obj)
  {
    switch (obj->type()) {
      case ObjectType::CLASS: {
        const auto cls = static_cast<ClassObject*>(obj);
        unsafe_get<ObjectPtr>(stack_.top(num_args)) =
            make_object<InstanceObject>(cls);

        if (const auto& method = cls->method(init_lexeme_)) {
          call(method->second, num_args);
          break;
        }

        if (num_args != 0) {
          incorrect_arg_num(0, num_args);
        }
        break;
      }

      case ObjectType::CLOSURE: {
        call(static_cast<ClosureObject*>(obj), num_args);
        break;
      }

      case ObjectType::METHOD: {
        const auto method = static_cast<MethodObject*>(obj);
        unsafe_get<ObjectPtr>(stack_.top(num_args)) = method->instance();
        call(method->closure(), num_args);
        break;
      }

      case ObjectType::NATIVE: {
        const auto native = static_cast<NativeObject*>(obj);

        if (native->arity() != num_args) {
          incorrect_arg_num(native->arity(), num_args);
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
    const auto& func_value = read_constant();
    const auto& func_obj = unsafe_get<ObjectPtr>(func_value);
    auto func = static_cast<FuncObject*>(func_obj);

    auto closure = make_object<ClosureObject>(func);

    for (unsigned int i = 0; i < closure->num_upvalues(); ++i) {
      const auto is_local = read_integer<InstrArgUByte>() != 0;
      const auto index = read_integer<InstrArgUByte>();

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
      incorrect_arg_num(closure->function().arity(), num_args);
    }

    if (call_stack_.size() == max_call_frames) {
      throw make_runtime_error("Stack overflow.");
    }

    call_stack_.emplace(ip_, code_object_, stack_.top(num_args), closure);
    code_object_ = closure->function().code_object();
    ip_ = code_object_->bytecode.begin();
  }


  Value VirtualMachine::read_constant()
  {
    return code_object_->constants[read_integer<InstrArgUByte>()];
  }


  loxx::StringObject* VirtualMachine::read_string()
  {
    return get_object<StringObject>(read_constant());
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


  void VirtualMachine::incorrect_arg_num(const InstrArgUByte arity,
                                         const InstrArgUByte num_args) const
  {
    std::stringstream ss;
    ss << "Expected " << static_cast<unsigned>(arity)
       << " arguments but got " << static_cast<unsigned>(num_args) << '.';
    throw make_runtime_error(ss.str());
  }


  RuntimeError VirtualMachine::make_runtime_error(const std::string& msg) const
  {
    const auto pos = std::distance(code_object_->bytecode.begin(), ip_);
    return RuntimeError(get_current_line(*code_object_, pos), msg);
  }
}
