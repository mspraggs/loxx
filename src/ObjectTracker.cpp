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
 * Created by Matt Spraggs on 23/04/18.
 */

#include "ObjectTracker.hpp"


namespace loxx
{
  StringObject* make_string(std::string std_str)
  {
    static HashSet<StringObject*, HashStringObject, CompareStringObject> cache;

    auto str_obj = std::make_unique<StringObject>(std::move(std_str));

    const auto cached = cache.find(str_obj.get());

    if (cached) {
      return *cached;
    }

    cache.insert(str_obj.get());
    const auto ret = str_obj.get();
    ObjTracker::instance().add_object(std::move(str_obj));
    return ret;
  }


  void grey_roots(Stack<Value, max_stack_size>* stack)
  {
    for (std::size_t i = 0; i < stack->size(); ++i) {
      auto& value = stack->get(i);

      if (not holds_alternative<ObjectPtr>(value)) {
        continue;
      }

      auto object = get<ObjectPtr>(value);
      object->set_colour(TriColour::GREY);
    }
  }


  void grey_roots(std::list<UpvalueObject*>* upvalues)
  {
    for (auto upvalue : *upvalues) {
      upvalue->set_colour(TriColour::GREY);
    }
  }


  void grey_roots(StringHashTable<Value>* globals)
  {
    for (auto& value : *globals) {
      value.first->set_colour(TriColour::GREY);

      if (not holds_alternative<ObjectPtr>(value.second)) {
        continue;
      }

      get<ObjectPtr>(value.second)->set_colour(TriColour::GREY);
    }
  }
}