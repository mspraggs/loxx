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

#include <list>
#include <memory>
#include <vector>

#include "HashSet.hpp"
#include "HashTable.hpp"
#include "Stack.hpp"
#include "Value.hpp"


namespace loxx
{
  class ObjectTracker
  {
  public:
    struct Roots
    {
      Stack<Value, max_stack_size>* stack;
      std::list<UpvalueObject*>* upvalues;
      StringHashTable<Value>* globals;
    };

    static ObjectTracker& instance();

    StringObject* add_string(std::unique_ptr<StringObject> str);
    void add_object(std::unique_ptr<Object> object);
    void set_roots(const Roots roots) { roots_ = roots; }

  private:
    ObjectTracker()
        : roots_{nullptr, nullptr, nullptr}
    {
      objects_.reserve(gc_size_trigger_);
    }

    void collect_garbage();

    void grey_roots();

    static constexpr std::size_t gc_size_trigger_ = 65536;
    std::vector<std::unique_ptr<Object>> objects_;
    HashSet<StringObject*, HashStringObject, CompareStringObject> strings_;
    Roots roots_;
  };


  template <typename T0, typename... Ts>
  T0* make_object(Ts&& ... args)
  {
    auto ptr = std::make_unique<T0>(std::forward<Ts>(args)...);
    const auto ret = ptr.get();
    ObjectTracker::instance().add_object(std::move(ptr));
    return ret;
  };
}

#endif //LOXX_OBJECTTRACKER_HPP
