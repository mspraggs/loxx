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
 * Created by Matt Spraggs on 23/11/2019.
 */

#include <numeric>
#include <utility>

#include "../HashTable.hpp"

#include "logging.hpp"
#include "RegisterAllocator.hpp"
#include "TraceCache.hpp"


namespace loxx
{
  namespace jit
  {
    RegisterAllocator::RegisterAllocator(
        const std::vector<Register>& registers)
        : stack_index_(0)
    {
      register_pool_.fill(false);
      for (const auto reg : registers) {
        add_register(reg);
      }
    }


    void RegisterAllocator::allocate(Trace& trace)
    {
      const auto& ir_buffer = trace.ir_buffer;
      auto& allocation_map = trace.allocation_map;
      allocation_map.resize(trace.ir_buffer.size(), {});

      for (int i = ir_buffer.size() - 1; i >= 0; --i) {
        const auto& instruction = ir_buffer[i];

        const auto& op0 = instruction.operand(0);
        const auto& op1 = instruction.operand(1);

        if (op0.type() == Operand::Type::IR_REF) {
          const auto ref = op0.index();
          if (not allocation_map[ref]) {
            const auto reg = get_register(ir_buffer[ref].type());
            if (reg) {
              allocation_map[ref] = *reg;
            }
            else {
              allocation_map[ref] = stack_index_++;
            }
          }
        }

        if (op1.type() == Operand::Type::IR_REF) {
          const auto ref = op1.index();
          if (not allocation_map[ref]) {
            const auto reg = get_register(ir_buffer[ref].type());
            if (reg) {
              allocation_map[ref] = *reg;
            }
            else {
              allocation_map[ref] = stack_index_++;
            }
          }
        }

        if (allocation_map[i] and
            holds_alternative<Register>(*allocation_map[i])) {
          add_register(unsafe_get<Register>(*allocation_map[i]));
        }
      }
    }


    Optional<Register> RegisterAllocator::get_register(const ValueType type)
    {
      for (unsigned int i = 0; i < register_pool_.size(); ++i) {
        if (register_pool_[i] and
            reg_supports_value_type(static_cast<Register>(i), type)) {
          register_pool_[i] = false;
          return static_cast<Register>(i);
        }
      }

      return {};
    }


    void RegisterAllocator::add_register(const Register reg)
    {
      register_pool_[static_cast<unsigned int>(reg)] = true;
    }
  }
}
