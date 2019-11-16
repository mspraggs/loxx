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
      RETURN,
      SUBTRACT,
    };


    class Operand
    {
    public:
      enum class Type
      {
        MEMORY = 0,
        REGISTER = 1,
        UNUSED = 2
      };

      Operand() = default;
      explicit Operand(const ValueType type);
      Operand(const Value& value);

      bool is_memory() const { return target_.index() == 0; }
      bool is_register() const { return target_.index() == 1; }
      bool is_used() const { return target_.index() != 2; }

      Type op_type() const { return static_cast<Type>(target_.index()); }
      ValueType value_type() const { return type_; }

      const Value* memory_address() const { return get<0>(target_); }
      std::size_t reg_index() const { return get<1>(target_); }

    private:
      static std::size_t reg_count_;
      ValueType type_;
      Variant<const Value*, std::size_t> target_;
    };


    class SSAInstruction
    {
    public:
      explicit SSAInstruction(
          const Operator op,
          const Operand operand0 = Operand(),
          const Operand operand1 = Operand());

      Operator op() const { return op_; }
      const Operand& operand0() const { return operand0_; }
      const Operand& operand1() const { return operand1_; }

    private:
      Operator op_;
      Operand operand0_;
      Operand operand1_;
    };


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
    Os& operator<<(Os& os, const Operand& operand)
    {
      if (operand.is_register()) {
        switch (operand.value_type()) {
        case ValueType::FLOAT:
          os << 'f';
          break;
        case ValueType::BOOLEAN:
          os << 'b';
          break;
        case ValueType::OBJECT:
          os << 'p';
          break;
        default:
          os << '?';
        }
        os << operand.reg_index();
      }
      else if (operand.is_memory()) {
        os << '[' << operand.memory_address() << ']';
      }

      return os;
    }
  }
}

#endif // LOXX_JIT_SSAINSTRUCTION_HPP
