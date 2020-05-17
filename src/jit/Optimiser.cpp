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
 * Created by Matt Spraggs on 21/02/2020.
 */

#include "Optimiser.hpp"


namespace loxx
{
  namespace jit
  {
    bool instructions_target_same_dest(
        const IRInstruction<2>& first, const IRInstruction<2>& second)
    {
      return first.operands()[0] == second.operands()[0];
    }

    bool instructions_contain_redundant_move(
        const IRInstruction<2>& first, const IRInstruction<2>& second)
    {
      return second.op() == Operator::MOVE and
          first.operands()[0] == second.operands()[1];
    }


    void Optimiser::optimise()
    {
      unroll_loop();
      trace_->state = Trace::State::IR_COMPLETE;

      auto& ir_buffer = trace_->ir_buffer;
      ir_refs_in_use_.resize(ir_buffer.size(), false);

      for (int i = ir_buffer.size() - 1; i > 0; --i) {
        const auto& cur_ins = ir_buffer[i];
        if (not ir_refs_in_use_[i] and cur_ins.needs_dependents()) {
          ir_buffer[i] = IRInstruction<2>(Operator::NOOP, ValueType::UNKNOWN);
          continue;
        }

        const auto& operands = cur_ins.operands();
        for (const auto& operand : operands) {
          if (operand.type() == Operand::Type::UNUSED) {
            break;
          }

          if (operand.type() == Operand::Type::IR_REF) {
            const auto ir_ref = unsafe_get<std::size_t>(operand);
            ir_refs_in_use_[ir_ref] = true;
          }
        }
      }
    }


    void Optimiser::unroll_loop()
    {
      auto& ir_buffer = trace_->ir_buffer;
      const auto init_ir_size = ir_buffer.size();
      copied_ir_refs_.resize(init_ir_size);
      phi_flags_.resize(init_ir_size, false);
      auto snap = trace_->snaps.begin();

      for (std::size_t ref = 0; ref < init_ir_size - 1; ++ref) {
        if (snap->ir_ref == ref) {
          for (const auto& ir_mapping : snap->stack_ir_map) {
            stack_->set(
                ir_mapping.first, get_ref(ir_mapping.second.value),
                {StackTag::CACHED, StackTag::WRITTEN});
          }
          ++snap;
        }
        const auto& instruction = ir_buffer[ref];

        if (instruction.is_loop_invariant()) {
          handle_invariant_instruction(ref, instruction);
          continue;
        }

        ir_buffer.push_back(make_instruction(ref, instruction));
      }

      emit_phi_instructions();

      ir_buffer[init_ir_size - 1] = IRInstruction<2>(
          Operator::LOOP_START, ValueType::UNKNOWN);

      const auto jump_size = ir_buffer.size() + 1 - init_ir_size;
      ir_buffer.emplace_back(
          Operator::LOOP, ValueType::UNKNOWN,
          Operand(Operand::Type::JUMP_OFFSET, jump_size));
    }


    void Optimiser::handle_invariant_instruction(
        const std::size_t ref, const IRInstruction<2>& instruction)
    {
      if (instruction.op() == Operator::LOAD) {
        const auto stack_pos =
            unsafe_get<std::size_t>(instruction.operand(0));
        copied_ir_refs_[ref] = stack_->get(stack_pos);
      }
      else {
        copied_ir_refs_[ref] = ref;
      }
    }


    IRInstruction<2> Optimiser::make_instruction(
        const std::size_t ref, const IRInstruction<2>& instruction)
    {
      const auto buf_size = trace_->ir_buffer.size();

      const auto op0 = make_operand(instruction.operand(0));
      const auto op1 = make_operand(instruction.operand(1));

      copied_ir_refs_[ref] = buf_size;
      for (const auto& op : {&op0, &op1}) {
        if (op->type() == Operand::Type::IR_REF) {
          const auto op_ref = unsafe_get<std::size_t>(*op);
          if (op_ref < copied_ir_refs_.size() and copied_ir_refs_[op_ref]) {
            phi_flags_[op_ref] = true;
          }
        }
      }

      return IRInstruction<2>(instruction.op(), instruction.type(), op0, op1);
    }


    Operand Optimiser::make_operand(const Operand& operand)
    {
      if (operand.type() == Operand::Type::IR_REF) {
        const auto op_ref = unsafe_get<std::size_t>(operand);
        if (copied_ir_refs_[op_ref]) {
          return Operand(Operand::Type::IR_REF, *copied_ir_refs_[op_ref]);
        }
      }
      else if (operand.type() == Operand::Type::STACK_REF) {
        const auto stack_ref = unsafe_get<std::size_t>(operand);
        const auto& vstack_elem = (*stack_)[stack_ref];
        if (vstack_elem.tags != 0) {
          return Operand(Operand::Type::IR_REF, vstack_elem.value);
        }
      }
      else if (operand.type() == Operand::Type::EXIT_NUMBER) {
        const auto prev_exit_num = unsafe_get<std::size_t>(operand);
        const auto new_exit_num = create_snapshot(trace_->snaps[prev_exit_num]);
        return Operand(Operand::Type::EXIT_NUMBER, new_exit_num);
      }
      return operand;
    }


    std::size_t Optimiser::create_snapshot(const Snapshot& prev_snapshot)
    {
      const auto snapshot_index = trace_->snaps.size();
      trace_->snaps.emplace_back(Snapshot{
          .ir_ref = trace_->ir_buffer.size(),
          .next_ip = prev_snapshot.next_ip,
          .stack_ir_map = compress_stack(*stack_)});

      for (const auto& mapping : trace_->snaps.back().stack_ir_map) {
        if (mapping.second.tags != 0) {
          phi_flags_[mapping.second.value] = true;
        }
      }
      return snapshot_index;
    }


    void Optimiser::update_phi(const Operand& operand)
    {
      if (operand.type() == Operand::Type::IR_REF) {
        const auto op_ref = unsafe_get<std::size_t>(operand);
        if (op_ref < copied_ir_refs_.size() and copied_ir_refs_[op_ref]) {
          phi_flags_[op_ref] = true;
        }
      }
    }


    void Optimiser::emit_phi_instructions() const
    {
      for (std::size_t ref = 0; ref < phi_flags_.size(); ++ref) {
        if (phi_flags_[ref] and copied_ir_refs_[ref]) {
          trace_->ir_buffer.emplace_back(
              Operator::PHI, ValueType::UNKNOWN,
              Operand(Operand::Type::IR_REF, ref),
              Operand(Operand::Type::IR_REF, *copied_ir_refs_[ref]));
        }
      }
    }
  }
}
