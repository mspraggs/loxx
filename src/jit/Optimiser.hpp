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

#ifndef LOXX_JIT_OPTIMISER_HPP
#define LOXX_JIT_OPTIMISER_HPP

#include "TraceCache.hpp"


namespace loxx
{
  namespace jit
  {
    void unroll_loop(
        Trace& trace, TaggedStack<std::size_t, StackTag, max_stack_size>& stack);

    void optimise(Trace& trace);


    class Optimiser
    {
    public:
      Optimiser(
          Trace& trace,
          TaggedStack<std::size_t, StackTag, max_stack_size>& stack)
          : trace_(&trace), stack_(&stack)
      {}

      void optimise();

    private:
      void unroll_loop();

      void handle_invariant_instruction(
          const std::size_t ref, const IRInstruction<2>& instruction);
      IRInstruction<2> make_instruction(
          const std::size_t ref, const IRInstruction<2>& instruction);
      void emit_phi_instructions() const;

      std::size_t get_ref(const std::size_t ref) const
      { return copied_ir_refs_[ref].value_or(ref); }

      Operand make_operand(const Operand& operand);
      std::size_t create_snapshot(const Snapshot& prev_snapshot);
      void update_phi(const Operand& operand);

      Trace* trace_;
      TaggedStack<std::size_t, StackTag, max_stack_size>* stack_;
      std::vector<bool> ir_refs_in_use_;
      std::vector<Optional<std::size_t>> copied_ir_refs_;
      std::vector<bool> phi_flags_;
    };
  }
}

#endif // LOXX_JIT_OPTIMISER_HPP
