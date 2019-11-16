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
 * Created by Matt Spraggs on 14/11/2019.
 */

#include <iostream>

#include "../Instruction.hpp"
#include "../logging.hpp"
#include "../utils.hpp"

#include "CodeProfiler.hpp"
#include "logging.hpp"
#include "SSAGenerator.hpp"


namespace loxx
{
  namespace jit
  {
    namespace detail
    {
      template <typename T>
      T read_integer(CodeObject::InsPtr& ip);
    }


    void SSAGenerator::build_stack(const Stack<Value, max_stack_size>& stack)
    {
      op_stack_.clear();

      for (std::size_t i = 0; i < stack.size(); ++i) {
        op_stack_.emplace(stack.get(i));
      }
    }


    std::vector<SSAInstruction> SSAGenerator::generate(
        const CodeObject::InsPtr begin, const CodeObject::InsPtr end,
        const CodeObject& code,
        const InstructionDataRepo& instruction_data)
    {
      auto ip = begin;
      std::vector<SSAInstruction> ssa_instructions;

      HashTable<const Value*, Operand> variable_map;

      std::size_t i = 0;

      while (ip != end) {
        const auto& instruction_datum = instruction_data.get(ip);
        const auto& instruction_values = instruction_datum->second;

        print_instruction(code, ip);

        const auto instruction = static_cast<loxx::Instruction>(*ip++);

        if (not instruction_datum) {
          throw std::runtime_error("missing instruction data");
        }

        switch (instruction) {

        case loxx::Instruction::ADD: {
          const auto second = op_stack_.pop();
          const auto first = op_stack_.pop();

          if (first.value_type() != ValueType::FLOAT
              or second.value_type() != ValueType::FLOAT) {
            throw std::runtime_error("unsupported operand types");
          }

          op_stack_.emplace(ValueType::FLOAT);

          ssa_instructions.emplace_back(Operator::MOVE, op_stack_.top(), first);
          ssa_instructions.emplace_back(Operator::ADD, op_stack_.top(), second);
          break;
        }

        case loxx::Instruction::GET_LOCAL: {
          // ip += sizeof(InstrArgUByte);
          const auto idx = detail::read_integer<InstrArgUByte>(ip);
          auto& cached_value = variable_map.insert(&instruction_values[0]);

          const auto source = op_stack_.get(idx);
          op_stack_.push(Operand(source.value_type()));
          ssa_instructions.emplace_back(
              Operator::MOVE, op_stack_.top(), source);
          break;
        }

        case loxx::Instruction::LOOP: {
          ip += sizeof(InstrArgUShort);
          ssa_instructions.emplace_back(Operator::RETURN);
          break;
        }

        case loxx::Instruction::LOAD_CONSTANT: {
          ip += sizeof(InstrArgUByte);
          const auto& constant = instruction_values[0];
          const auto type = static_cast<ValueType>(constant.index());
          op_stack_.push(Operand(type));
          ssa_instructions.emplace_back(
              Operator::MOVE, op_stack_.top(), Operand(constant));
          break;
        }

        case loxx::Instruction::MULTIPLY: {
          const auto second = op_stack_.pop();
          const auto first = op_stack_.pop();

          if (first.value_type() != ValueType::FLOAT
              or second.value_type() != ValueType::FLOAT) {
            throw std::runtime_error("unsupported operand types");
          }

          op_stack_.emplace(ValueType::FLOAT);

          ssa_instructions.emplace_back(Operator::MOVE, op_stack_.top(), first);
          ssa_instructions.emplace_back(
              Operator::MULTIPLY, op_stack_.top(), second);
          break;
        }

        case loxx::Instruction::POP:
          if (op_stack_.size() > 0) {
            op_stack_.discard();
          }
          break;

        case loxx::Instruction::SET_LOCAL: {
          ip += sizeof(InstrArgUByte);
          const auto& value = instruction_values[0];
          ssa_instructions.emplace_back(
              Operator::MOVE, value, op_stack_.top());
          break;
        }

        default:
          throw std::runtime_error("unsupported instruction");
        }

        for (i; i < ssa_instructions.size(); ++i) {
          std::cout << " -- ";
          print_ssa_instruction(ssa_instructions[i]);
        }
        std::cout << " -- stack: " << op_stack_ << '\n';
      }

      return ssa_instructions;
    }


    template <typename T>
    T detail::read_integer(CodeObject::InsPtr& ip)
    {
      const auto ret = read_integer_at_pos<T>(ip);
      ip += sizeof(T);
      return ret;
    }
  }
}
