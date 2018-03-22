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
  VirtualMachine::VirtualMachine(const bool debug)
      : debug_(debug), ip_(0)
  {
  }


  void VirtualMachine::execute(const CompilationOutput& compiler_output)
  {
    compiler_output_ = &compiler_output;
    ip_ = 0;

    while (ip_ < compiler_output.bytecode.size()) {

      const auto instruction =
          static_cast<Instruction>(compiler_output.bytecode[ip_]);

      if (debug_) {
        print_stack();
        disassemble_instruction();
      }

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

      case Instruction::ConditionalJump: {
        const auto jmp = read_integer<ByteCodeArg>();
        if (is_truthy(stack_.pop())) {
          ip_ += jmp;
        }
        break;
      }

      case Instruction::DefineGlobal: {
        const auto arg = read_integer<ByteCodeArg>();
        const auto varname = get<std::string>(constants_[arg]);
        globals_[varname] = stack_.pop();
        break;
      }

      case Instruction::False:
        stack_.push(StackVar(InPlace<bool>(), false));
        break;

      case Instruction::GetGlobal: {
        const auto arg = read_integer<ByteCodeArg>();
        const auto varname = get<std::string>(constants_[arg]);

        if (globals_.count(varname) != 1) {
          throw RuntimeError(get_current_line(),
                             "Undefined variable '" + varname + "'.");
        }

        stack_.push(globals_[varname]);
        break;
      }

      case Instruction::GetLocal: {
        const auto arg = read_integer<ByteCodeArg>();
        stack_.push(stack_.get(arg));
        break;
      }

      case Instruction::Jump:
        ip_ += read_integer<ByteCodeArg>();
        break;

      case Instruction::LoadConstant:
        stack_.push(constants_[read_integer<ByteCodeArg>()]);
        break;

      case Instruction::Nil:
        stack_.push(StackVar());
        break;

      case Instruction::Pop:
        stack_.pop();
        break;

      case Instruction::Print:
        print_object(stack_.pop());
        break;

      case Instruction::SetGlobal: {
        const auto arg = read_integer<ByteCodeArg>();
        const auto varname = get<std::string>(constants_[arg]);

        if (globals_.count(varname) == 0) {
          throw RuntimeError(get_current_line(),
                             "Undefined variable '" + varname + "'.");
        }

        globals_[varname] = stack_.top();
        break;
      }

      case Instruction::SetLocal: {
        const auto arg = read_integer<ByteCodeArg>();
        stack_.get(arg) = stack_.top();
        break;
      }

      case Instruction::True:
        stack_.push(StackVar(InPlace<bool>(), true));
        break;

      default:
        std::cout << "Unknown instruction: "
                  << static_cast<unsigned int>(instruction) << std::endl;
        break;
      }

      ++ip_;
    }
  }


  ByteCodeArg VirtualMachine::make_constant(const std::string& lexeme,
                                            const StackVar& value)
  {
    if (constant_map_.count(lexeme) != 0) {
      return constant_map_[lexeme];
    }

    const auto index = static_cast<ByteCodeArg>(constants_.size());

    constants_.push_back(value);
    constant_map_[lexeme] = index;

    return index;
  }


  ByteCodeArg VirtualMachine::get_constant(const std::string& lexeme) const
  {
    if (constant_map_.count(lexeme) == 0) {
      throw RuntimeError(get_current_line(),
                         "Undefined variable '" + lexeme + "'.");
    }

    return constant_map_.at(lexeme);
  }


  void VirtualMachine::print_object(StackVar variant) const
  {
    std::cout << stringify(variant) << std::endl;
  }


  void VirtualMachine::execute_binary_op(const Instruction instruction)
  {
    const auto second = stack_.pop();
    const auto first = stack_.pop();

    switch (instruction) {
    case Instruction::NotEqual:
      stack_.push(StackVar(InPlace<bool>(), not are_equal(first, second)));
      break;
    case Instruction::Equal:
      stack_.push(StackVar(InPlace<bool>(), are_equal(first, second)));
      break;
    case Instruction::Less:
      check_number_operands(first, second);
      stack_.push(StackVar(InPlace<bool>(),
                           get<double>(first) < get<double>(second)));
      break;
    case Instruction::LessEqual:
      check_number_operands(first, second);
      stack_.push(StackVar(InPlace<bool>(),
                           get<double>(first) <= get<double>(second)));
      break;
    case Instruction::Greater:
      check_number_operands(first, second);
      stack_.push(StackVar(InPlace<bool>(),
                           get<double>(first) > get<double>(second)));
      break;
    case Instruction::GreaterEqual:
      check_number_operands(first, second);
      stack_.push(StackVar(InPlace<bool>(),
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


  std::string VirtualMachine::stringify(const StackVar& object) const
  {
    if (object.index() == StackVar::npos) {
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
  }


  void VirtualMachine::print_stack() const
  {
    if (stack_.size() == 0) {
      std::cout << std::endl;
      return;
    }
    std::cout << "          [ ";

    for (unsigned int i = 0; i < stack_.size(); ++i) {
      std::cout << stringify(stack_.get(i));

      if (i < stack_.size() - 1) {
        std::cout << " | ";
      }
    }

    std::cout << " ]" << std::endl;
  }


  void VirtualMachine::disassemble_instruction() const
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

    switch (instruction) {

    case Instruction::Add:
    case Instruction::Divide:
    case Instruction::Equal:
    case Instruction::False:
    case Instruction::Greater:
    case Instruction::GreaterEqual:
    case Instruction::Less:
    case Instruction::LessEqual:
    case Instruction::Multiply:
    case Instruction::Nil:
    case Instruction::NotEqual:
    case Instruction::Pop:
    case Instruction::Print:
    case Instruction::Push:
    case Instruction::Return:
    case Instruction::Subtract:
    case Instruction::True:
      break;

    case Instruction::ConditionalJump:
    case Instruction::Jump: {
      const auto param = read_integer_at_pos<ByteCodeArg>(ip_ + 1);
      std::cout << param;
      break;
    }
    case Instruction::DefineGlobal:
    case Instruction::GetGlobal:
    case Instruction::GetLocal:
    case Instruction::SetGlobal:
    case Instruction::SetLocal:
    case Instruction::LoadConstant: {
      const auto param = read_integer_at_pos<ByteCodeArg>(ip_ + 1);
      std::cout << param << " '" << stringify(constants_[param]) << "'";
      break;
    }
    }

    std::cout << '\n';
  }


  void VirtualMachine::check_number_operands(
      const StackVar& first, const StackVar& second) const
  {
    if (not holds_alternative<double>(first) or
        not holds_alternative<double>(second)) {
      throw RuntimeError(get_current_line(),
                         "Binary operands must both be numbers.");
    }
  }


  bool VirtualMachine::are_equal(const StackVar& first,
                                 const StackVar& second) const
  {
    if (first.index()  == StackVar::npos and
        second.index() == StackVar::npos) {
      return true;
    }
    if (first.index() == StackVar::npos) {
      return false;
    }

    return first == second;
  }


  bool VirtualMachine::is_truthy(const StackVar& value) const
  {
    if (value.index() == StackVar::npos) {
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

      if (instruction_counter > ip_) {
        break;
      }
    }

    return line;
  }
}
