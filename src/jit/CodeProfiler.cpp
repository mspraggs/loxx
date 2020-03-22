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
    void CodeProfiler::handle_basic_block_head(const CodeObject::InsPtr ip)
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
      ssa_ir_map_[ip] = ssa_ir_.size();
      recorded_instructions_.push_back(ip);
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

      case Instruction::CONDITIONAL_JUMP: {
        const auto offset = read_integer_at_pos<InstrArgUShort>(ip + 1);
        jump_targets_.push_back(std::make_pair(
            ip + sizeof(InstrArgUShort) + 1 + offset, ssa_ir_.size()));
        ssa_ir_.emplace_back(Operator::CONDITIONAL_JUMP, 0ul, op_stack_.top());
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
          entry_code_.emplace_back(
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

      case Instruction::LOOP: {
        is_recording_ = false;

        const auto jump_size = read_integer_at_pos<InstrArgUShort>(ip + 1);
        const auto target = ip + sizeof(InstrArgUShort) - jump_size + 1;
        const auto jump_pos = ssa_ir_map_.get(target);

        const auto operand = Operand(
            InPlace<std::size_t>(),
            ssa_ir_.size() + 1 - jump_pos->second);
        ssa_ir_.emplace_back(Operator::LOOP, operand);

        finalise_ir();
        trace_cache_->add_ssa_ir(
            current_block_head_, ip + sizeof(InstrArgUShort) + 1,
            std::move(recorded_instructions_), std::move(ssa_ir_));
        break;
      }

      case Instruction::MULTIPLY: {
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
            Operator::MULTIPLY, op_stack_.top(), first, second);

        break;
      }

      case Instruction::POP:
        op_stack_.pop();
        break;

      case Instruction::SET_LOCAL: {
        const auto idx = read_integer_at_pos<InstrArgUByte>(ip + 1);
        const auto& value = context.stack_frame.slot(idx);

        const auto destination =
            Operand(static_cast<ValueType>(value.index()));
        const auto& result = operand_cache_.insert(
            std::make_pair(OperandLocation::LOCAL, idx), destination);

        ssa_ir_.emplace_back(
            Operator::MOVE, result.first->second, op_stack_.top());
        exit_assignments_[&value] = result.first->second;
        break;
      }

      default:
        is_recording_ = false;

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


    void CodeProfiler::finalise_ir()
    {
      for (const auto& jump_target : jump_targets_) {
        const auto instruction_pos = jump_target.second;
        const auto target_elem = ssa_ir_map_.get(jump_target.first);

        const auto target_pos =
            target_elem ? target_elem->second : ssa_ir_.size();

        const auto pos = Operand(
            InPlace<std::size_t>(),
            target_pos + entry_code_.size() - instruction_pos);

        ssa_ir_[instruction_pos].set_operand(0, pos);
      }

      ssa_ir_.insert(ssa_ir_.begin(), entry_code_.begin(), entry_code_.end());

      for (const auto& assignment : exit_assignments_) {
        ssa_ir_.emplace_back(
            Operator::MOVE, assignment.first, assignment.second);
      }
    }


    bool CodeProfiler::OperandIndexHasher::operator() (
        const CodeProfiler::OperandIndex& value) const
    {
      return combine_hashes(static_cast<std::size_t>(value.first), value.second);
    }
  }
}
