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
        const auto second = vreg_stack_.pop();
        const auto first = vreg_stack_.pop();

        const auto result_type = [&] {
          if (second.type == ValueType::FLOAT and
              first.type == ValueType::FLOAT) {
            return ValueType::FLOAT;
          }
          return ValueType::OBJECT;
        } ();

        vreg_stack_.emplace(
            VirtualRegisterGenerator::make_register(result_type));

        ssa_ir_.emplace_back(
            Operator::ADD, vreg_stack_.top(), first, second);

        break;
      }

      case Instruction::CONDITIONAL_JUMP: {
        const auto offset = read_integer_at_pos<InstrArgUShort>(ip + 1);
        jump_targets_.push_back(std::make_pair(
            ip + sizeof(InstrArgUShort) + 1 + offset, ssa_ir_.size()));
        ssa_ir_.emplace_back(
            Operator::CONDITIONAL_JUMP, 0ul, vreg_stack_.top());
        break;
      }

      case Instruction::GET_LOCAL: {
        const auto idx = read_integer_at_pos<InstrArgUByte>(ip + 1);
        const auto& value = context.stack_frame.slot(idx);

        const auto destination = VirtualRegisterGenerator::make_register(
            static_cast<ValueType>(value.index()));
        const auto& result = vreg_cache_.insert(&value, destination);

        vreg_stack_.push(result.first->second);

        if (result.second) {
          ssa_ir_.emplace_back(
              Operator::MOVE, vreg_stack_.top(), Operand(&value));
        }
        break;
      }

      case Instruction::LESS: {
        const auto second = vreg_stack_.pop();
        const auto first = vreg_stack_.pop();

        if (not virtual_registers_are_floats(first, second)) {
          is_recording_ = false;
          return;
        }
        vreg_stack_.emplace(
            VirtualRegisterGenerator::make_register(ValueType::BOOLEAN));

        ssa_ir_.emplace_back(
            Operator::LESS, vreg_stack_.top(), first, second);

        break;
      }

      case Instruction::LOAD_CONSTANT: {
        const auto idx = read_integer_at_pos<InstrArgUByte>(ip + 1);
        const auto& value = context.code.constants[idx];

        const auto destination = VirtualRegisterGenerator::make_register(
            static_cast<ValueType>(value.index()));

        vreg_stack_.push(destination);

        ssa_ir_.emplace_back(
            Operator::MOVE, vreg_stack_.top(), Operand(value));
        break;
      }

      case Instruction::LOOP: {
        is_recording_ = false;

        if (not instruction_ends_current_block(ip)) {
          return;
        }

        patch_jumps();
        peel_loop(ip);
        emit_exit_assignments();

        trace_cache_->add_ssa_ir(
            current_block_head_, ip + sizeof(InstrArgUShort) + 1,
            std::move(recorded_instructions_), std::move(ssa_ir_));
        break;
      }

      case Instruction::MULTIPLY: {
        const auto second = vreg_stack_.pop();
        const auto first = vreg_stack_.pop();

        if (not virtual_registers_are_floats(first, second)) {
          is_recording_ = false;
          return;
        }
        vreg_stack_.emplace(
            VirtualRegisterGenerator::make_register(ValueType::FLOAT));

        ssa_ir_.emplace_back(
            Operator::MULTIPLY, vreg_stack_.top(), first, second);

        break;
      }

      case Instruction::POP:
        vreg_stack_.pop();
        break;

      case Instruction::SET_LOCAL: {
        const auto idx = read_integer_at_pos<InstrArgUByte>(ip + 1);
        const auto& value = context.stack_frame.slot(idx);

        const auto destination = VirtualRegisterGenerator::make_register(
            static_cast<ValueType>(value.index()));
        const auto& result = vreg_cache_.insert(&value, destination);

        ssa_ir_.emplace_back(
            Operator::MOVE, result.first->second, vreg_stack_.top());
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
      vreg_cache_.clear();
      VirtualRegisterGenerator::reset_reg_count();
      is_recording_ = true;
    }


    bool CodeProfiler::instruction_ends_current_block(
        const CodeObject::InsPtr ip) const
    {
      const auto instruction = static_cast<Instruction>(*ip);

      if (instruction == Instruction::LOOP) {
        const auto offset = read_integer_at_pos<InstrArgUShort>(ip + 1);
        const auto target = ip + sizeof(InstrArgUShort) - offset + 1;

        return target <= current_block_head_;
      }

      throw JITError("unhandled backward branch instruction");
    }


    void CodeProfiler::peel_loop(const CodeObject::InsPtr ip)
    {
      const auto prev_ssa_size = ssa_ir_.size();
      const auto loop_vreg_map = build_loop_vreg_map();

      for (std::size_t i = 0; i < prev_ssa_size; ++i) {
        const auto& instruction = ssa_ir_[i];
        ssa_ir_.emplace_back(instruction.op());
        const auto& operands = instruction.operands();

        for (int j = 0; j < operands.size(); ++j) {
          if (operands[j].index() == Operand::npos) {
            break;
          }

          if (holds_alternative<VirtualRegister>(operands[j])) {
            const auto current_vreg = unsafe_get<VirtualRegister>(operands[j]);
            const auto mapped_vreg = loop_vreg_map.get(current_vreg);

            ssa_ir_.back().set_operand(
                j, mapped_vreg ? Operand(mapped_vreg->second) : operands[j]);
          }
          else if (holds_alternative<const Value*>(operands[j])) {
            const auto address = unsafe_get<const Value*>(operands[j]);
            const auto vreg = vreg_cache_.get(address);

            ssa_ir_.back().set_operand(
                j, vreg ? Operand(vreg->second) : operands[j]);
          }
          else if (holds_alternative<std::size_t>(operands[j])) {
            const auto old_offset = unsafe_get<std::size_t>(operands[j]);
            const auto new_offset = old_offset + loop_vreg_map.size();
            const auto peeled_offset = new_offset + prev_ssa_size;

            ssa_ir_.back().set_operand(
                j, Operand(InPlace<std::size_t>(), new_offset));
            ssa_ir_[i].set_operand(
                j, Operand(InPlace<std::size_t>(), peeled_offset));
          }
          else {
            ssa_ir_.back().set_operand(j, operands[j]);
          }
        }
      }

      emit_loop_moves(loop_vreg_map);
      emit_loop(ip, prev_ssa_size + loop_vreg_map.size());
    }


    VRegHashTable<VirtualRegister> CodeProfiler::build_loop_vreg_map() const
    {
      VRegHashTable<VirtualRegister> loop_vreg_map;

      for (const auto& elem : vreg_cache_) {
        loop_vreg_map[elem.second] =
            VirtualRegisterGenerator::make_register(elem.second.type);
      }

      return loop_vreg_map;
    }


    void CodeProfiler::emit_loop_moves(
        const VRegHashTable<VirtualRegister>& loop_vreg_map)
    {
      for (const auto& elem : loop_vreg_map) {
        ssa_ir_.emplace_back(
            Operator::MOVE, Operand(elem.first), Operand(elem.second));
      }
    }


    void CodeProfiler::emit_loop(
        const CodeObject::InsPtr ip, const std::size_t ir_offset)
    {
      const auto jump_size = read_integer_at_pos<InstrArgUShort>(ip + 1);
      const auto target = ip + sizeof(InstrArgUShort) - jump_size + 1;
      const auto jump_pos = ssa_ir_map_.get(target);

      const auto operand = Operand(
          InPlace<std::size_t>(), ir_offset + 1 - jump_pos->second);
      ssa_ir_.emplace_back(Operator::LOOP, operand);
    }


    void CodeProfiler::patch_jumps()
    {
      for (const auto& jump_target : jump_targets_) {
        const auto instruction_pos = jump_target.second;
        const auto target_elem = ssa_ir_map_.get(jump_target.first);

        const auto target_pos =
            target_elem ? target_elem->second : ssa_ir_.size();

        const auto pos = Operand(
            InPlace<std::size_t>(), target_pos - instruction_pos);

        ssa_ir_[instruction_pos].set_operand(0, pos);
      }
    }


    void CodeProfiler::emit_exit_assignments()
    {
      for (const auto& assignment : exit_assignments_) {
        ssa_ir_.emplace_back(
            Operator::MOVE, assignment.first, assignment.second);
      }
    }


    bool CodeProfiler::virtual_registers_are_floats(
        const VirtualRegister& first, const VirtualRegister& second) const
    {
      return first.type == ValueType::FLOAT and second.type == ValueType::FLOAT;
    }
  }
}
