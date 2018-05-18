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

#include <algorithm>

#include "ObjectTracker.hpp"


namespace loxx
{
  ObjectTracker& ObjectTracker::instance()
  {
    static ObjectTracker ret;
    return ret;
  }


  void ObjectTracker::add_object(std::unique_ptr<Object> object)
  {
    if (objects_.size() > gc_size_trigger_) {
      collect_garbage();
    }

    objects_.push_back(std::move(object));
  }


  void ObjectTracker::collect_garbage()
  {
    for (auto& object : objects_) {
      object->set_colour(TriColour::White);
    }

    grey_roots();

    const auto is_grey =
        [] (const std::unique_ptr<Object>& obj)
        {
          return obj and obj->colour() == TriColour::Grey;
        };

    auto num_greys = std::count_if(objects_.begin(), objects_.end(), is_grey);

    std::vector<std::unique_ptr<Object>> reachable_objects;
    reachable_objects.reserve(objects_.size());

    while (num_greys > 0) {
      for (auto& object : objects_) {
        if (not object or object->colour() != TriColour::Grey) {
          continue;
        }

        object->set_colour(TriColour::Black);
        object->grey_references();

        reachable_objects.push_back(std::move(object));
      }

      num_greys = std::count_if(objects_.begin(), objects_.end(), is_grey);
    }

    objects_ = std::move(reachable_objects);
  }


  void ObjectTracker::grey_roots()
  {
    for (auto& constant : *roots_.constants) {
      if (not holds_alternative<ObjectPtr>(constant)) {
        continue;
      }

      auto object = get<ObjectPtr>(constant);
      object->set_colour(TriColour::Grey);
    }

    for (std::size_t i = 0; i < roots_.stack->size(); ++i) {
      auto& value = roots_.stack->get(i);

      if (not holds_alternative<ObjectPtr>(value)) {
        continue;
      }

      auto object = get<ObjectPtr>(value);

      object->set_colour(TriColour::Grey);
    }

    for (auto upvalue : *roots_.upvalues) {
      upvalue->set_colour(TriColour::Grey);
    }

    for (auto& value : *roots_.globals) {
      value.first->set_colour(TriColour::Grey);

      if (not holds_alternative<ObjectPtr>(value.second)) {
        continue;
      }

      get<ObjectPtr>(value.second)->set_colour(TriColour::Grey);
    }
  }
}