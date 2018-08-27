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
 * Created by Matt Spraggs on 18/04/18.
 */

#include "CodeObject.hpp"
#include "ObjectTracker.hpp"
#include "Object.hpp"


namespace loxx
{
  FuncObject::FuncObject(
      std::string lexeme, std::unique_ptr<CodeObject> code_object,
      const unsigned int arity, const UByteCodeArg num_upvalues)
      : Object(ObjectType::Function),
        arity_(arity), num_upvalues_(num_upvalues),
        code_object_(std::move(code_object)), lexeme_(std::move(lexeme))
  {
  }


  void UpvalueObject::grey_references()
  {
    if (holds_alternative<ObjectPtr>(*value_)) {
      auto obj = get<ObjectPtr>(*value_);
      obj->set_colour(TriColour::Grey);
    }
  }


  void ClosureObject::grey_references()
  {
    function_->set_colour(TriColour::Grey);

    for (auto upvalue : upvalues_) {
      if (upvalue) {
        upvalue->set_colour(TriColour::Grey);
      }
    }
  }


  bool ClassObject::has_method(StringObject* name) const
  {
    bool ret = methods_.has_item(name);

    if (ret) {
      return ret;
    }

    if (superclass_) {
      return superclass_->has_method(name);
    }

    return false;
  }


  auto ClassObject::method(StringObject* name) const
      -> const StringHashTable<ClosureObject*>::Elem&
  {
    const auto& elem = methods_.get(name);
    if (elem) {
      return elem;
    }
    if (superclass_) {
      return superclass_->method(name);
    }
    return elem;
  }


  void MethodObject::grey_references()
  {
    closure_->set_colour(TriColour::Grey);
    instance_->set_colour(TriColour::Grey);
  }


  void ClassObject::grey_references()
  {
    for (auto& method : methods_) {
      method.first->set_colour(TriColour::Grey);
      method.second->set_colour(TriColour::Grey);
    }

    if (superclass_) {
      superclass_->set_colour(TriColour::Grey);
    }
  }


  void InstanceObject::grey_references()
  {
    cls_->set_colour(TriColour::Grey);

    for (auto& field : fields_) {
      field.first->set_colour(TriColour::Grey);

      if (not holds_alternative<ObjectPtr>(field.second)) {
        continue;
      }

      auto obj = get<ObjectPtr>(field.second);
      obj->set_colour(TriColour::Grey);
    }
  }
}
