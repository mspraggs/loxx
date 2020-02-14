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
 * Created by Matt Spraggs on 13/10/2019.
 */

#include <algorithm>
#include <functional>
#include <iostream>
#include <numeric>

#include "../logging.hpp"
#include "../VirtualMachine.hpp"

#include "CodeProfiler.hpp"
#include "JITError.hpp"
#include "logging.hpp"


namespace loxx
{
  namespace jit
  {
    void CodeProfiler::count_basic_block(const CodeObject::InsPtr ip)
    {
      if (is_recording_) {
        return;
      }

      current_block_head_ = ip;
      if (ignored_blocks_.has_item(ip)) {
        return;
      }
      auto count_elem = block_counts_.insert(ip, 0);
      count_elem.first->second += 1;

  #ifndef NDEBUG
      if (debug_) {
        std::cout << "Block hit count @ " << static_cast<const void*>(&(*ip))
                  << " = " << count_elem.first->second << '\n';
      }
  #endif

      if (count_elem.first->second >= block_count_threshold_) {
        start_recording();
      }
    }


    void CodeProfiler::skip_current_block()
    {
      ignored_blocks_.insert(current_block_head_);
    }


    void CodeProfiler::record_instruction(
        const CodeObject::InsPtr ip, const RuntimeContext context)
    {
      const auto instruction = static_cast<Instruction>(*ip);

      switch (instruction) {

      case Instruction::ADD: {
        const auto second = op_stack_.pop();
        const auto first = op_stack_.pop();

        const auto result_type = [&] {
          if (second.value_type() == ValueType::FLOAT and
              first.value_type() == ValueType::FLOAT) {
            return ValueType::FLOAT;
          }
          return ValueType::OBJECT;
        } ();

        op_stack_.emplace(result_type);

        ssa_ir_.emplace_back(
            Operator::ADD, op_stack_.top(), first, second);

        break;
      }

      case Instruction::GET_LOCAL: {
        const auto idx = read_integer_at_pos<InstrArgUByte>(ip + 1);
        const auto& value = context.stack_frame.slot(idx);

        const auto destination =
            Operand(static_cast<ValueType>(value.index()));
        const auto& result = operand_cache_.insert(
            std::make_pair(OperandLocation::LOCAL, idx), destination);

        op_stack_.push(result.first->second);

        if (result.second) {
          ssa_ir_.emplace_back(
              Operator::MOVE, op_stack_.top(), Operand(&value));
        }
        break;
      }

      case Instruction::LESS: {
        const auto second = op_stack_.pop();
        const auto first = op_stack_.pop();

        op_stack_.emplace(ValueType::BOOLEAN);

        ssa_ir_.emplace_back(
            Operator::LESS, op_stack_.top(), first, second);

        break;
      }

      case Instruction::LOAD_CONSTANT: {
        const auto idx = read_integer_at_pos<InstrArgUByte>(ip + 1);
        const auto& value = context.code.constants[idx];

        const auto destination =
            Operand(static_cast<ValueType>(value.index()));
        const auto& result = operand_cache_.insert(
            std::make_pair(OperandLocation::CONSTANT, idx), destination);

        op_stack_.push(result.first->second);

        if (result.second) {
          ssa_ir_.emplace_back(
              Operator::MOVE, op_stack_.top(), Operand(value));
        }
        break;
      }

      case Instruction::LOOP:
        is_recording_ = false;
        break;


      case Instruction::POP:
        op_stack_.pop();
        break;
      }
    }


    void CodeProfiler::start_recording()
    {
      block_counts_.erase(current_block_head_);
      ssa_ir_.clear();
      operand_cache_.clear();
      Operand::reset_reg_count();
      is_recording_ = true;
    }


    bool CodeProfiler::OperandIndexHasher::operator() (
        const CodeProfiler::OperandIndex& value) const
    {
      return combine_hashes(static_cast<std::size_t>(value.first), value.second);
    }
  }
}
