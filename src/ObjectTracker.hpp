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

#include "Stack.hpp"
#include "Value.hpp"

#include <list>
#include <memory>
#include <vector>


namespace loxx
{
  class ObjectTracker
  {
  public:
    struct Roots
    {
      Stack<Value>* stack;
      std::list<std::shared_ptr<UpvalueObject>>* upvalues;
      std::unordered_map<std::string, Value>* globals;
    };

    static ObjectTracker& instance();

    void add_object(ObjectPtr object);

    void set_roots(const Roots roots) { roots_ = roots; }

  private:
    ObjectTracker() : roots_{nullptr, nullptr, nullptr} {}

    void collect_garbage();

    void grey_roots();

    static constexpr std::size_t gc_size_trigger_ =
        1024 * 1024 / sizeof(ObjectPtr);
    std::vector<ObjectPtr> objects_;
    Roots roots_;
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
