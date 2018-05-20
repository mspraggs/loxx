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

#include "ObjectTracker.hpp"
#include "Value.hpp"


namespace loxx
{
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

    if (instance_) {
      instance_->set_colour(TriColour::Grey);
    }

    for (auto upvalue : upvalues_) {
      if (upvalue) {
        upvalue->set_colour(TriColour::Grey);
      }
    }
  }


  bool ClassObject::has_method(const raw_ptr<StringObject> name) const
  {
    bool ret = methods_.count(name) != 0;

    if (superclass_) {
      ret |= superclass_->has_method(name);
    }

    return ret;
  }


  raw_ptr<ClosureObject> ClassObject::method(
      const raw_ptr<StringObject> name) const
  {
    if (methods_.count(name) != 0) {
      return methods_.at(name);
    }
    if (superclass_) {
      return superclass_->method(name);
    }
    return nullptr;
  }


  raw_ptr<ClosureObject> ClassObject::method(const raw_ptr<StringObject> name)
  {
    if (methods_.count(name) != 0) {
      return methods_[name];
    }
    if (superclass_) {
      return superclass_->method(name);
    }
    return nullptr;
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
