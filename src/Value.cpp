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


  void UpvalueObject::clear_references()
  {
    if (holds_alternative<ObjectPtr>(*value_)) {
      auto ptr = get<ObjectPtr>(*value_);
      ptr.reset();
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


  void ClosureObject::clear_references()
  {
    instance_.reset();
    function_.reset();
    upvalues_.clear();
  }


  bool ClassObject::has_method(const std::string& name) const
  {
    bool ret = methods_.count(name) != 0;

    if (superclass_) {
      ret |= superclass_->has_method(name);
    }

    return ret;
  }


  std::shared_ptr<ClosureObject> ClassObject::method(
      const std::string& name) const
  {
    if (methods_.count(name) != 0) {
      return std::make_shared<ClosureObject>(*methods_.at(name));
    }
    if (superclass_) {
      return superclass_->method(name);
    }
    return std::shared_ptr<ClosureObject>();
  }


  std::shared_ptr<ClosureObject> ClassObject::method(const std::string& name)
  {
    if (methods_.count(name) != 0) {
      return std::make_shared<ClosureObject>(*methods_[name]);
    }
    if (superclass_) {
      return superclass_->method(name);
    }
    return std::shared_ptr<ClosureObject>();
  }


  void ClassObject::grey_references()
  {
    for (auto& method : methods_) {
      method.second->set_colour(TriColour::Grey);
    }

    if (superclass_) {
      superclass_->set_colour(TriColour::Grey);
    }
  }


  void ClassObject::clear_references()
  {
    superclass_.reset();
    methods_.clear();
  }


  void InstanceObject::grey_references()
  {
    cls_->set_colour(TriColour::Grey);

    for (auto& field : fields_) {
      if (not holds_alternative<ObjectPtr>(field.second)) {
        continue;
      }

      auto obj = get<ObjectPtr>(field.second);
      obj->set_colour(TriColour::Grey);
    }
  }


  void InstanceObject::clear_references()
  {
    cls_.reset();
    fields_.clear();
  }
}