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
    Native,
    String,
    Upvalue,
  };


  enum class TriColour
  {
    White,
    Grey,
    Black
  };


  using ObjectPtr = raw_ptr<Object>;
  using Value = Variant<double, bool, ObjectPtr>;


  class Object
  {
  public:
    virtual ~Object() = default;

    ObjectType type() const { return type_; }

    TriColour colour() const { return colour_; }
    void set_colour(const TriColour colour) { colour_ = colour; }

    virtual void grey_references() {}

  protected:
    explicit Object(const ObjectType type)
        : type_(type), colour_(TriColour::White)
    {}

  private:
    ObjectType type_;
    TriColour colour_;
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

    void grey_references() override;

  private:
    raw_ptr<Value> value_;
    Value closed_;
  };


  class ClosureObject : public Object
  {
  public:
    explicit ClosureObject(raw_ptr<FuncObject> func)
        : Object(ObjectType::Closure),
          instance_(), function_(func), upvalues_(function_->num_upvalues())
    {
    }

    raw_ptr<UpvalueObject> upvalue(const std::size_t i) const
    { return upvalues_[i]; }
    void set_upvalue(const std::size_t i, raw_ptr<UpvalueObject> value)
    { upvalues_[i] = value; }

    std::size_t num_upvalues() const { return upvalues_.size(); }

    const FuncObject& function() const { return *function_; }

    void bind(raw_ptr<InstanceObject> instance) { instance_ = instance; }
    raw_ptr<InstanceObject> instance() const { return instance_; }

    void grey_references() override;

  private:
    raw_ptr<InstanceObject> instance_;
    raw_ptr<FuncObject> function_;
    std::vector<raw_ptr<UpvalueObject>> upvalues_;
  };


  class ClassObject : public Object
  {
  public:
    explicit ClassObject(std::string lexeme,
                         raw_ptr<ClassObject> superclass = {})
        : Object(ObjectType::Class),
          lexeme_(std::move(lexeme)), superclass_(superclass)
    {}

    const std::string& lexeme() const { return lexeme_; }

    bool has_method(const std::string& name) const;
    raw_ptr<ClosureObject> method(const std::string& name) const;
    raw_ptr<ClosureObject> method(const std::string& name);

    void set_method(const std::string& name, raw_ptr<ClosureObject> method)
    { methods_[name] = method; }

    void grey_references() override;

  private:
    std::string lexeme_;
    std::unordered_map<std::string, raw_ptr<ClosureObject>> methods_;
    raw_ptr<ClassObject> superclass_;
  };


  class InstanceObject : public Object
  {
  public:
    explicit InstanceObject(raw_ptr<ClassObject> cls)
        : Object(ObjectType::Instance),
          cls_(cls)
    {}

    bool has_field(const std::string& name) const
    { return fields_.count(name) != 0; }

    const Value& field(const std::string& name) const
    { return fields_.at(name); }
    Value& field(const std::string& name) { return fields_[name]; }

    void set_field(const std::string& name, const Value& value)
    { fields_[name] = value; }

    const ClassObject& cls() const { return *cls_; }

    void grey_references() override;

  private:
    raw_ptr<ClassObject> cls_;
    std::unordered_map<std::string, Value> fields_;
  };


  class NativeObject : public Object
  {
  public:
    using Fn = Value (*) (raw_ptr<const Value>, const unsigned int);

    explicit NativeObject(Fn func)
        : Object(ObjectType::Native),
          func_(func)
    {}

    Value call(raw_ptr<const Value> args, const unsigned int num_args)
    { return func_(args, num_args); }

  private:
    Fn func_;
  };


  class StringObject : public Object
  {
  public:
    explicit StringObject(std::string value)
        : Object(ObjectType::String),
          value_(std::move(value))
    {}

    explicit operator std::string() { return value_; }

  private:
    std::string value_;
  };


  namespace detail
  {
    template <typename T>
    constexpr ObjectType object_type();


    template<>
    constexpr ObjectType object_type<ClassObject>()
    {
      return ObjectType::Class;
    }


    template<>
    constexpr ObjectType object_type<ClosureObject>()
    {
      return ObjectType::Closure;
    }


    template<>
    constexpr ObjectType object_type<FuncObject>()
    {
      return ObjectType::Function;
    }


    template<>
    constexpr ObjectType object_type<InstanceObject>()
    {
      return ObjectType::Instance;
    }


    template<>
    constexpr ObjectType object_type<NativeObject>()
    {
      return ObjectType::Native;
    }


    template<>
    constexpr ObjectType object_type<StringObject>()
    {
      return ObjectType::String;
    }


    template<>
    constexpr ObjectType object_type<UpvalueObject>()
    {
      return ObjectType::Upvalue;
    }
  }


  template <typename T>
  constexpr raw_ptr<T> get_object(const Value& value)
  {
    if (holds_alternative<ObjectPtr>(value) and
        get<ObjectPtr>(value)->type() == detail::object_type<T>())
    {
      return static_cast<raw_ptr<T>>(get<ObjectPtr>(value));
    }

    return nullptr;
  }
}

#endif // LOXX_VALUE_HPP
