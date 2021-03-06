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
 * Created by Matt Spraggs on 27/08/18.
 */

#ifndef LOXX_OBJECT_HPP
#define LOXX_OBJECT_HPP

#include "StringHashTable.hpp"
#include "Value.hpp"


namespace loxx
{
  struct CodeObject;


  class Object;
  class InstanceObject;
  class StringObject;


  using ObjectPtr = Object*;


  enum class TriColour
  {
    White,
    Grey,
    Black
  };


  enum class ObjectType
  {
    Class,
    Closure,
    Function,
    Instance,
    Method,
    Native,
    String,
    Upvalue,
  };


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
    FuncObject(std::string lexeme, std::unique_ptr<CodeObject> code_object,
               const unsigned int arity, const InstrArgUByte num_upvalues);

    const CodeObject* code_object() const
    { return code_object_.get(); }

    unsigned int arity() const { return arity_; }
    InstrArgUByte num_upvalues() const { return num_upvalues_; }
    const std::string& lexeme() const { return lexeme_; }

  private:
    unsigned int arity_;
    InstrArgUByte num_upvalues_;
    std::unique_ptr<CodeObject> code_object_;
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
    Value* value_;
    Value closed_;
  };


  class ClosureObject : public Object
  {
  public:
    explicit ClosureObject(FuncObject* func)
        : Object(ObjectType::Closure),
          function_(func), upvalues_(function_->num_upvalues())
    {
    }

    UpvalueObject* upvalue(const std::size_t i) const
    { return upvalues_[i]; }
    void set_upvalue(const std::size_t i, UpvalueObject* value)
    { upvalues_[i] = value; }

    std::size_t num_upvalues() const { return upvalues_.size(); }

    const FuncObject& function() const { return *function_; }

    void grey_references() override;

  private:
    FuncObject* function_;
    std::vector<UpvalueObject*> upvalues_;
  };


  class MethodObject : public Object
  {
  public:
    MethodObject(ClosureObject& closure, InstanceObject& instance)
        : Object(ObjectType::Method),
          closure_(&closure), instance_(&instance)
    {}

    ClosureObject* closure() { return closure_; }
    InstanceObject* instance() { return instance_; }

    void grey_references() override;

  private:
    ClosureObject* closure_;
    InstanceObject* instance_;
  };


  class ClassObject : public Object
  {
  public:
    explicit ClassObject(std::string lexeme, ClassObject* superclass = {})
        : Object(ObjectType::Class),
          lexeme_(std::move(lexeme)),
          superclass_(superclass)
    {}

    const std::string& lexeme() const { return lexeme_; }

    bool has_method(StringObject* name) const;
    auto method(StringObject* name) const
        -> const StringHashTable<ClosureObject*>::Elem&;

    void set_method(StringObject* name, ClosureObject* method)
    { methods_[name] = method; }

    void grey_references() override;

  private:
    std::string lexeme_;
    StringHashTable<ClosureObject*> methods_;
    ClassObject* superclass_;
  };


  class InstanceObject : public Object
  {
  public:
    explicit InstanceObject(ClassObject* cls)
        : Object(ObjectType::Instance),
          cls_(cls)
    {}

    bool has_field(StringObject* name) const
    { return fields_.count(name) != 0; }

    auto field(StringObject* name) const -> const StringHashTable<Value>::Elem&
    { return fields_.get(name); }

    void set_field(StringObject* name, const Value& value)
    { fields_[name] = value; }

    const ClassObject& cls() const { return *cls_; }

    void grey_references() override;

  private:
    ClassObject* cls_;
    StringHashTable<Value> fields_;
  };


  class NativeObject : public Object
  {
  public:
    using Fn = Value (*) (const Value*, const unsigned int);

    explicit NativeObject(Fn func, const unsigned int arity)
        : Object(ObjectType::Native),
          arity_(arity), func_(func)
    {}

    unsigned int arity() const { return arity_; }

    Value call(const Value* args, const unsigned int num_args)
    { return func_(args, num_args); }

  private:
    unsigned int arity_;
    Fn func_;
  };


  class StringObject : public Object
  {
  public:
    explicit StringObject(std::string value)
        : Object(ObjectType::String),
          hash_(std::hash<std::string>()(value)), value_(std::move(value))
    {}

    explicit operator std::string() { return value_; }

    const std::string& as_std_string() const { return value_; }

    std::size_t hash() const { return hash_; }

  private:
    std::size_t hash_;
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
  constexpr T* get_object(const Value& value)
  {
    if (holds_alternative<Object*>(value) and
        unsafe_get<Object*>(value)->type() == detail::object_type<T>())
    {
      return static_cast<T*>(unsafe_get<Object*>(value));
    }

    return nullptr;
  }


  template <typename OStream, typename T>
  auto operator<<(OStream& os, const T& value)
      -> std::enable_if_t<std::is_same<T, Value>::value, OStream&>
  {
    if (value.index() == Value::npos) {
      os << "nil";
      return os;
    }
    if (holds_alternative<double>(value)) {
      os << get<double>(value);
      return os;
    }
    else if (holds_alternative<bool>(value)) {
      os << std::boolalpha << get<bool>(value);
    }
    else if (const auto str = get_object<StringObject>(value)) {
      os << str->as_std_string();
    }
    else if (holds_alternative<ObjectPtr>(value)) {
      const auto ptr = get<ObjectPtr>(value);

      switch (ptr->type()) {
        case ObjectType::Function:
          os << "<fn " << static_cast<FuncObject*>(ptr)->lexeme() << '>';
          break;
        case ObjectType::Class: {
          const auto cls = static_cast<ClassObject*>(ptr);
          os << "<class " << cls->lexeme() << '>';
          break;
        }
        case ObjectType::Closure: {
          const auto closure = static_cast<ClosureObject*>(ptr);
          os << "<fn " << closure->function().lexeme() << '>';
          break;
        }
        case ObjectType::Instance: {
          const auto instance = static_cast<InstanceObject*>(ptr);
          os << instance->cls().lexeme() << " instance";
        }
        default:
          break;
      }
    }

    return os;
  }
}

#endif //LOXX_OBJECT_HPP
