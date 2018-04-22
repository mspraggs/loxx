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

#include <string>
#include <unordered_map>
#include <vector>

#include "globals.hpp"
#include "Variant.hpp"

namespace loxx
{
  class Object;
  class InstanceObject;


  enum class ObjectType
  {
    Class,
    Closure,
    Function,
    Instance,
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
        : Object(ObjectType::Upvalue), value_(&slot)
    {}

    void close()
    {
      closed_ = *value_;
      value_ = &closed_;
    }

    const Value& value() const { return *value_; }
    void set_value(const Value& value) { *value_ = value; }

  private:
    Value* value_;
    Value closed_;
  };


  class ClosureObject : public Object
  {
  public:
    explicit ClosureObject(std::shared_ptr<FuncObject> func)
        : Object(ObjectType::Closure),
          function_(std::move(func)), upvalues_(function_->num_upvalues())
    {
    }

    std::shared_ptr<UpvalueObject> upvalue(const std::size_t i) const
    { return upvalues_[i]; }
    void set_upvalue(const std::size_t i, std::shared_ptr<UpvalueObject> value)
    { upvalues_[i] = std::move(value); }

    std::size_t num_upvalues() const { return upvalues_.size(); }

    const FuncObject& function() const { return *function_; }

    void bind(std::shared_ptr<InstanceObject> instance)
    { instance_ = std::move(instance); }
    std::shared_ptr<InstanceObject> instance() const { return instance_; }

  private:
    std::shared_ptr<InstanceObject> instance_;
    std::shared_ptr<FuncObject> function_;
    std::vector<std::shared_ptr<UpvalueObject>> upvalues_;
  };


  class ClassObject : public Object
  {
  public:
    explicit ClassObject(std::string lexeme,
                         std::shared_ptr<ClassObject> superclass = {})
        : Object(ObjectType::Class),
          lexeme_(std::move(lexeme)), superclass_(std::move(superclass))
    {}

    const std::string& lexeme() const { return lexeme_; }

    bool has_method(const std::string& name) const;
    std::shared_ptr<ClosureObject> method(const std::string& name) const;
    std::shared_ptr<ClosureObject> method(const std::string& name);

    void set_method(const std::string& name,
                    std::shared_ptr<ClosureObject> method)
    { methods_[name] = std::move(method); }

  private:
    std::string lexeme_;
    std::unordered_map<std::string, std::shared_ptr<ClosureObject>> methods_;
    std::shared_ptr<ClassObject> superclass_;
  };


  class InstanceObject : public Object
  {
  public:
    explicit InstanceObject(std::shared_ptr<ClassObject> cls)
        : Object(ObjectType::Instance),
          cls_(std::move(cls))
    {}

    bool has_field(const std::string& name) const
    { return fields_.count(name) != 0; }

    const Value& field(const std::string& name) const
    { return fields_.at(name); }
    Value& field(const std::string& name) { return fields_[name]; }

    void set_field(const std::string& name, const Value& value)
    { fields_[name] = value; }

    const ClassObject& cls() const { return *cls_; }

  private:
    std::shared_ptr<ClassObject> cls_;
    std::unordered_map<std::string, Value> fields_;
  };
}

#endif // LOXX_VALUE_HPP
