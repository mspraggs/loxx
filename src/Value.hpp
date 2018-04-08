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
 * Created by Matt Spraggs on 28/03/2018.
 */

#ifndef LOXX_VALUE_HPP
#define LOXX_VALUE_HPP

#include <vector>

#include "Variant.hpp"

namespace loxx
{
  class Object;


  enum class ObjectType
  {
    Closure,
    Function,
    Upvalue,
  };


  using Value = Variant<double, bool, std::string, std::shared_ptr<Object>>;


  class Object
  {
  public:
    virtual ~Object() = default;

    ObjectType type() const { return type_; }

  protected:
    explicit Object(const ObjectType type) : type_(type) {}

  private:
    ObjectType type_;
  };


  class FuncObject : public Object
  {
  public:
    FuncObject(std::string lexeme, const std::size_t bytecode_offset,
               const unsigned int arity, const UByteCodeArg num_upvalues)
        : Object(ObjectType::Function),
          arity_(arity), num_upvalues_(num_upvalues),
          bytecode_offset_(bytecode_offset), lexeme_(std::move(lexeme))
    {}

    std::size_t bytecode_offset() const { return bytecode_offset_; }

    unsigned int arity() const { return arity_; }
    UByteCodeArg num_upvalues() const { return num_upvalues_; }
    const std::string& lexeme() const { return lexeme_; }

  private:
    unsigned int arity_;
    UByteCodeArg num_upvalues_;
    std::size_t bytecode_offset_;
    std::string lexeme_;
  };


  class UpvalueObject : public Object
  {
  public:
    explicit UpvalueObject(Value& slot)
        : Object(ObjectType::Upvalue), value_(&slot), next_(nullptr)
    {}

    void close()
    {
      closed_ = *value_;
      value_ = nullptr;
    }

    const Value* value() const { return value_; }
    void set_value(Value& value) { value_ = &value; }

  private:
    Value* value_;
    Value closed_;
    UpvalueObject* next_;
  };


  class ClosureObject : public Object
  {
  public:
    explicit ClosureObject(std::shared_ptr<FuncObject> func)
        : Object(ObjectType::Closure),
          function_(std::move(func)), upvalues_(function_->num_upvalues())
    {
    }

    std::vector<UpvalueObject*>& upvalues() { return upvalues_; }
    const std::vector<UpvalueObject*>& upvalues() const { return upvalues_; }

    const FuncObject& function() const { return *function_; }

  private:
    std::shared_ptr<FuncObject> function_;
    std::vector<UpvalueObject*> upvalues_;
  };
}

#endif // LOXX_VALUE_HPP
