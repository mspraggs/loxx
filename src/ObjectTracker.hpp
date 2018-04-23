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

#ifndef LOXX_OBJECTTRACKER_HPP
#define LOXX_OBJECTTRACKER_HPP

#include "Value.hpp"

#include <memory>
#include <vector>


namespace loxx
{
  class ObjectTracker
  {
  public:
    static ObjectTracker& instance();

    void add_object(std::shared_ptr<Object> object);

  private:
    ObjectTracker() : gc_size_trigger_(1024) {}

    void collect_garbage();

    std::size_t gc_size_trigger_;
    std::vector<std::shared_ptr<Object>> objects_;
  };


  template <typename T0, typename... Ts>
  std::shared_ptr<T0> make_shared(Ts&&... args)
  {
    const auto ptr = std::make_shared<T0>(std::forward<Ts>(args)...);
    ObjectTracker::instance().add_object(ptr);
    return ptr;
  };
}

#endif //LOXX_OBJECTTRACKER_HPP