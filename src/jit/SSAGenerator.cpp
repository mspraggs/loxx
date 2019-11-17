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
#include "../VirtualMachine.hpp"

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


    void SSAGenerator::build_context(const RuntimeContext& context)
    {
      op_stack_.clear();
      for (std::size_t i = 0; i < context.stack.size(); ++i) {
        op_stack_.emplace(context.stack.get(i));
      }

      globals_ = StringHashTable<Operand>();
      for (const auto& global_elem : context.globals) {
        globals_[global_elem.first] = Operand(global_elem.second);
      }

      const auto& constants = context.code.constants;
      constants_ = std::vector<Operand>(constants.size());
      for (std::size_t i = 0; i < constants_.size(); ++i) {
        constants_[i] = Operand(constants[i]);
      }

      upvalues_ = std::vector<Operand>(context.func.num_upvalues());
      for (std::size_t i = 0; i < upvalues_.size(); ++i) {
        upvalues_[i] = Operand(context.func.upvalue(i)->value());
      }
    }


    std::vector<SSAInstruction> SSAGenerator::generate(
        const CodeObject::InsPtr begin, const CodeObject::InsPtr end,
        const CodeObject& code,
        const InstructionDataRepo& instruction_data)
    {
      auto ip = begin;
      std::vector<SSAInstruction> ssa_instructions;

      while (ip != end) {
        const auto& instruction_datum = instruction_data.get(ip);
        const auto& instruction_values = instruction_datum->second;

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
          const auto idx = detail::read_integer<InstrArgUByte>(ip);
          const auto& constant = constants_[idx];
          const auto type = static_cast<ValueType>(constant.value_type());
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
          const auto idx = detail::read_integer<InstrArgUByte>(ip);
          const auto& value = op_stack_.get(idx);
          ssa_instructions.emplace_back(
              Operator::MOVE, value, op_stack_.top());
          break;
        }

        default:
          throw std::runtime_error("unsupported instruction");
        }
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
