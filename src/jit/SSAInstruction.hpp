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
 * Created by Matt Spraggs on 06/11/2019.
 */

#ifndef LOXX_JIT_SSAINSTRUCTION_HPP
#define LOXX_JIT_SSAINSTRUCTION_HPP

#include <array>

#include "../Value.hpp"
#include "../Variant.hpp"


namespace loxx
{
  namespace jit
  {
    enum class Operator
    {
      ADD,
      DIVIDE,
      MOVE,
      MULTIPLY,
      POP,
      RETURN,
      SUBTRACT,
    };


    struct VirtualRegister
    {
      ValueType type;
      std::size_t index;
    };


    using OperandBase = Variant<
        const Value*,
        VirtualRegister,
        Value,
        std::size_t>;


    class Operand : public OperandBase
    {
    public:
      enum class Type
      {
        MEMORY = 0,
        REGISTER = 1,
        IMMEDIATE = 2,
        OFFSET = 3,
        UNUSED = 4,
      };

      using OperandBase::Variant;

      explicit Operand(const ValueType value_type);

      const Value* memory_address() const { return nullptr; }
      Type op_type() const { return static_cast<Type>(index()); }
      ValueType value_type() const;

    private:
      static std::size_t reg_count_;
    };


    struct VirtualRegisterHasher
    {
      std::size_t operator() (const VirtualRegister& value) const;

      std::hash<const Value*> pointer_hasher;
    };


    struct VirtualRegisterCompare
    {
      bool operator() (
          const VirtualRegister& first, const VirtualRegister& second) const;
    };


    template <std::size_t N>
    class SSAInstruction
    {
    public:
      template <typename... Args>
      explicit SSAInstruction(const Operator op, const Args&... operands);

      Operator op() const { return op_; }
      const std::array<Operand, N>& operands() const { return operands_; }

    private:
      Operator op_;
      std::array<Operand, N> operands_;
    };


    template <std::size_t N>
    template <typename... Args>
    SSAInstruction<N>::SSAInstruction(
        const Operator op, const Args&... operands)
        : op_(op), operands_{operands...}
    {
    }


    template <typename Os>
    Os& operator<<(Os& os, const Operator op)
    {
      switch (op) {
      case Operator::ADD:
        os << "ADD";
        break;
      case Operator::DIVIDE:
        os << "DIVIDE";
        break;
      case Operator::MOVE:
        os << "MOVE";
        break;
      case Operator::MULTIPLY:
        os << "MULTIPLY";
        break;
      case Operator::POP:
        os << "POP";
        break;
      case Operator::RETURN:
        os << "RETURN";
        break;
      case Operator::SUBTRACT:
        os << "SUBTRACT";
        break;
      }

      return os;
    }


    template <typename Os>
    Os& operator<<(Os& os, const VirtualRegister& reg)
    {
      const auto value_type_char = [&] {
        switch (reg.type) {
        case ValueType::FLOAT:
          return 'f';
        case ValueType::BOOLEAN:
          return 'b';
        case ValueType::OBJECT:
          return 'p';
        }
      } ();

      os << value_type_char << reg.index;

      return os;
    }


    template <typename Os>
    Os& operator<<(Os& os, const Operand& operand)
    {
      const auto value_type_char = [&] {
        switch (operand.value_type()) {
        case ValueType::FLOAT:
          return 'f';
        case ValueType::BOOLEAN:
          return 'b';
        case ValueType::OBJECT:
          return 'p';
        default:
          return '?';
        }
      } ();

      if (holds_alternative<VirtualRegister>(operand)) {
        os << value_type_char << unsafe_get<VirtualRegister>(operand).index;
      }
      else if (holds_alternative<const Value*>(operand)) {
        os << "[ " << value_type_char << '@'
           << unsafe_get<const Value*>(operand) << " ]";
      }

      return os;
    }
  }
}

#endif // LOXX_JIT_SSAINSTRUCTION_HPP
