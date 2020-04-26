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
    void CodeProfiler::handle_basic_block_head(
        const CodeObject::InsPtr ip, const RuntimeContext context)
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
        start_recording(ip, context);
      }
    }


    void CodeProfiler::skip_current_block()
    {
      ignored_blocks_.insert(trace_cache_->active_trace()->init_ip);
    }


    void CodeProfiler::record_instruction(
        const CodeObject::InsPtr ip, const RuntimeContext context)
    {
      trace_->ir_map[ip] = trace_->ir_buffer.size();
      trace_->recorded_instructions.push_back(ip);
      const auto instruction = static_cast<Instruction>(*ip);

      switch (instruction) {

      case Instruction::ADD: {
        const auto second = vreg_stack_.pop();
        const auto first = vreg_stack_.pop();

        const auto result_type = [&] {
          const auto& op_first = trace_->ir_buffer[first];
          const auto& op_second = trace_->ir_buffer[second];
          if (op_second.type() == ValueType::FLOAT and
              op_first.type() == ValueType::FLOAT) {
            return ValueType::FLOAT;
          }
          return ValueType::OBJECT;
        } ();

        vreg_stack_.emplace(emit_ir(
            Operator::ADD, result_type,
            Operand(Operand::Type::IR_REF, first),
            Operand(Operand::Type::IR_REF, second)));

        break;
      }

      case Instruction::CONDITIONAL_JUMP: {
        const auto offset = read_integer_at_pos<InstrArgUShort>(ip + 1);
        jump_targets_.push_back(std::make_pair(
            ip + 1 + sizeof(InstrArgUShort) + offset, trace_->ir_buffer.size()));
        vreg_stack_.emplace(emit_ir(
            Operator::CONDITIONAL_JUMP, ValueType::UNKNOWN,
            Operand(Operand::Type::JUMP_OFFSET, 0ul),
            Operand(Operand::Type::IR_REF, vreg_stack_.top())));
        break;
      }

      case Instruction::GET_LOCAL: {
        const auto idx = read_integer_at_pos<InstrArgUByte>(ip + 1);
        const auto& value = context.stack_frame.slot(idx);

        const auto pos = std::distance(context.stack.data(), &value);
        const auto ref =
            ref_cache_[pos] ? *ref_cache_[pos] : trace_->ir_buffer.size();

        vreg_stack_.push(ref);

        if (not ref_cache_[pos]) {
          /// TODO: Store type information in a snapshot for use in guards
          emit_ir(
              Operator::LOAD, static_cast<ValueType>(value.index()),
              Operand(Operand::Type::STACK_REF, pos));
          ref_cache_[pos] = ref;
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

        emit_ir(
            Operator::LESS, ValueType::BOOLEAN,
            Operand(Operand::Type::IR_REF, first),
            Operand(Operand::Type::IR_REF, second));

        break;
      }

      case Instruction::LOAD_CONSTANT: {
        const auto idx = read_integer_at_pos<InstrArgUByte>(ip + 1);
        const auto& value = context.code.constants[idx];
        const auto type = static_cast<ValueType>(value.index());

        const auto operand = [&] {
          switch (type) {
          case ValueType::FLOAT:
            return Operand(unsafe_get<double>(value));
          case ValueType::BOOLEAN:
            return Operand(unsafe_get<bool>(value));
          case ValueType::OBJECT:
            return Operand(unsafe_get<ObjectPtr>(value));
          case ValueType::UNKNOWN:
            return Operand(Operand::Type::LITERAL_NIL);
          }
        } ();

        vreg_stack_.push(emit_ir(Operator::LITERAL, type, operand));
        break;
      }

      case Instruction::LOOP: {
        is_recording_ = false;

        if (not instruction_ends_current_block(ip)) {
          return;
        }

        // emit_ir(Operator::LOOP, ValueType::UNKNOWN, );
        // peel_loop();
        // emit_exit_assignments();
        emit_ir(
            Operator::LOOP, ValueType::UNKNOWN,
            Operand(Operand::Type::JUMP_OFFSET, trace_->ir_buffer.size() + 1));
        patch_jumps();

        /// TODO: Remove this hack and update the stack when we leave the trace
        trace_->next_ip = ip + sizeof(InstrArgUShort) + 2;
        trace_->state = Trace::State::IR_COMPLETE;
        break;
      }

      case Instruction::MULTIPLY: {
        const auto second = vreg_stack_.pop();
        const auto first = vreg_stack_.pop();

        if (not virtual_registers_are_floats(first, second)) {
          is_recording_ = false;
          return;
        }

        vreg_stack_.emplace(emit_ir(
            Operator::MULTIPLY, ValueType::FLOAT,
            Operand(Operand::Type::IR_REF, first),
            Operand(Operand::Type::IR_REF, second)));

        break;
      }

      case Instruction::POP:
        vreg_stack_.pop();
        break;

      case Instruction::SET_LOCAL: {
        const auto idx = read_integer_at_pos<InstrArgUByte>(ip + 1);
        const auto& value = context.stack_frame.slot(idx);

        const auto pos = std::distance(context.stack.data(), &value);
        const auto ref =
            ref_cache_[pos] ? *ref_cache_[pos] : trace_->ir_buffer.size();

        emit_ir(
            Operator::STORE, static_cast<ValueType>(value.index()),
            Operand(Operand::Type::STACK_REF, pos),
            Operand(Operand::Type::IR_REF, vreg_stack_.top()));
        exit_assignments_[pos] = ref;
        ref_cache_[pos].reset();
        break;
      }

      default:
        is_recording_ = false;

      }

    }


    void CodeProfiler::start_recording(
        const CodeObject::InsPtr ip, const RuntimeContext context)
    {
      block_counts_.erase(ip);
      trace_cache_->make_new_trace();
      trace_ = trace_cache_->active_trace();
      trace_->init_ip = ip;
      trace_->stack_base = context.stack.data();
      trace_->code_object = &context.code;
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


    void CodeProfiler::peel_loop()
    {
      const auto prev_ssa_size = trace_->ir_buffer.size();
      const auto loop_vreg_map = build_loop_ir_ref_map();

      for (std::size_t i = 0; i < prev_ssa_size; ++i) {
        const auto& instruction = trace_->ir_buffer[i];
        if (instruction.op() == Operator::LOOP_START) {
          continue;
        }
        trace_->ir_buffer.emplace_back(instruction.op(), instruction.type());
        const auto& operands = instruction.operands();

        for (int j = 0; j < operands.size(); ++j) {
          if (operands[j].type() == Operand::Type::UNUSED) {
            break;
          }

          if (operands[j].type() == Operand::Type::IR_REF) {
            const auto current_vreg = unsafe_get<std::size_t>(operands[j]);
            const auto mapped_vreg = loop_vreg_map.get(current_vreg);

            const auto new_op =
                mapped_vreg ?
                Operand(operands[j].type(), mapped_vreg->second) : operands[j];

            trace_->ir_buffer.back().set_operand(j, new_op);
          }
          else if (operands[j].type() == Operand::Type::STACK_REF) {
            const auto pos = unsafe_get<std::size_t>(operands[j]);
            const auto ref = ref_cache_[pos];

            trace_->ir_buffer.back().set_operand(
                j, ref ? Operand(Operand::Type::IR_REF, *ref) : operands[j]);
          }
          else if (operands[j].type() == Operand::Type::JUMP_OFFSET) {
            const auto old_offset = unsafe_get<std::size_t>(operands[j]);
            const auto new_offset = old_offset + loop_vreg_map.size();
            const auto peeled_offset = new_offset + prev_ssa_size;

            trace_->ir_buffer.back().set_operand(
                j, Operand(Operand::Type::JUMP_OFFSET, new_offset));
            trace_->ir_buffer[i].set_operand(
                j, Operand(Operand::Type::JUMP_OFFSET, peeled_offset));
          }
          else {
            trace_->ir_buffer.back().set_operand(j, operands[j]);
          }
        }
      }

      emit_loop_moves(loop_vreg_map);
      emit_loop();
    }


    auto CodeProfiler::build_loop_ir_ref_map() const
        -> HashTable<std::size_t, std::size_t>
    {
      HashTable<std::size_t, std::size_t> loop_vreg_map;

      for (std::size_t i = 0; i < ref_cache_.size(); ++i) {
        const auto& elem = ref_cache_[i];
        if (not elem) {
          continue;
        }
        // loop_vreg_map[*elem] =
        //     VirtualRegisterGenerator::make_register(elem.second.type);
      }

      return loop_vreg_map;
    }


    void CodeProfiler::emit_loop_moves(
        const HashTable<std::size_t, std::size_t>& loop_vreg_map)
    {
      // for (const auto& elem : loop_vreg_map) {
      //   emit_ir(Operator::MOVE, elem.first, elem.second);
      // }
    }


    void CodeProfiler::emit_loop()
    {
      const auto loop_head_pos = std::find_if(
          trace_->ir_buffer.begin(), trace_->ir_buffer.end(),
          [] (const SSAInstruction<3>& instruction) {
            return instruction.op() == Operator::LOOP_START;
          });

      const auto offset = std::distance(loop_head_pos, trace_->ir_buffer.end());
      emit_ir(
          Operator::LOOP, ValueType::UNKNOWN,
          Operand(Operand::Type::JUMP_OFFSET, offset + 1));
    }


    void CodeProfiler::patch_jumps()
    {
      for (const auto& jump_target : jump_targets_) {
        const auto instruction_pos = jump_target.second;
        const auto target_elem = trace_->ir_map.get(jump_target.first);

        const auto target_pos =
            target_elem ? target_elem->second : trace_->ir_buffer.size();

        const auto jump = Operand(
            Operand::Type::JUMP_OFFSET, target_pos - instruction_pos);

        trace_->ir_buffer[instruction_pos].set_operand(0, jump);
      }
    }


    void CodeProfiler::emit_exit_assignments()
    {
      for (const auto& assignment : exit_assignments_) {
        // emit_ir(Operator::MOVE, assignment.first, assignment.second);
      }
    }


    bool CodeProfiler::virtual_registers_are_floats(
        const std::size_t first, const std::size_t second) const
    {
      return trace_->ir_buffer[first].type() == ValueType::FLOAT and
          trace_->ir_buffer[second].type() == ValueType::FLOAT;
    }
  }
}
