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
  ObjectTracker& ObjectTracker::instance()
  {
    static ObjectTracker ret;
    return ret;
  }


  void ObjectTracker::add_object(std::shared_ptr<Object> object)
  {
    objects_.push_back(std::move(object));

    if (objects_.size() > gc_size_trigger_) {
      collect_garbage();
    }
  }


  void ObjectTracker::collect_garbage()
  {

  }


  void ObjectTracker::grey_roots()
  {
    for (std::size_t i = 0; i < roots_.stack->size(); ++i) {
      auto& value = roots_.stack->get(i);

      if (not holds_alternative<std::shared_ptr<Object>>(value)) {
        continue;
      }

      auto object = get<std::shared_ptr<Object>>(value);

      object->set_colour(TriColour::Grey);
    }

    for (auto upvalue : *roots_.upvalues) {
      upvalue->set_colour(TriColour::Grey);
    }

    for (auto& value : *roots_.globals) {
      if (not holds_alternative<std::shared_ptr<Object>>(value.second)) {
        continue;
      }

      get<std::shared_ptr<Object>>(value.second)->set_colour(TriColour::Grey);
    }
  }
}