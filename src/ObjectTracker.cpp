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
}