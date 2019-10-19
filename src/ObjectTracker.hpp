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

#include <algorithm>
#include <list>
#include <memory>
#include <vector>

#include "Object.hpp"
#include "HashSet.hpp"
#include "HashTable.hpp"
#include "Stack.hpp"
#include "Value.hpp"


namespace loxx
{
  template <typename... Roots>
  class ObjectTracker
  {
  public:
    static ObjectTracker& instance();

    void add_object(std::unique_ptr<Object> object);
    void set_roots(Roots&... roots) { roots_ = std::make_tuple(&roots...); }

  private:
    ObjectTracker()
        : roots_()
    {
      objects_.reserve(gc_size_trigger_);
    }

    void collect_garbage();

    static constexpr std::size_t gc_size_trigger_ = 65536;
    std::vector<std::unique_ptr<Object>> objects_;
    std::tuple<Roots*...> roots_;
  };


  namespace detail
  {
    inline void grey_roots_impl() {}


    template <typename T0, typename... Ts>
    void grey_roots_impl(T0 head, Ts... tail);


    template <std::size_t... Is, typename... Ts>
    void grey_roots(std::index_sequence<Is...>, std::tuple<Ts...>& roots);


    template <typename... Ts>
    void grey_roots(std::tuple<Ts...>& roots);
  }


  template <typename... Roots>
  ObjectTracker<Roots...>& ObjectTracker<Roots...>::instance()
  {
    static ObjectTracker<Roots...> ret;
    return ret;
  }


  template <typename... Roots>
  void ObjectTracker<Roots...>::add_object(std::unique_ptr<Object> object)
  {
    if (objects_.size() > gc_size_trigger_) {
      collect_garbage();
    }

    objects_.push_back(std::move(object));
  }


  template <typename... Roots>
  void ObjectTracker<Roots...>::collect_garbage()
  {
    for (auto& object : objects_) {
      object->set_colour(TriColour::WHITE);
    }

    detail::grey_roots(roots_);

    const auto is_grey =
        [] (const std::unique_ptr<Object>& obj)
        {
          return obj and obj->colour() == TriColour::GREY;
        };

    auto num_greys = std::count_if(objects_.begin(), objects_.end(), is_grey);

    std::vector<std::unique_ptr<Object>> reachable_objects;
    reachable_objects.reserve(objects_.size());

    while (num_greys > 0) {
      for (auto& object : objects_) {
        if (not object or object->colour() != TriColour::GREY) {
          continue;
        }

        object->set_colour(TriColour::BLACK);
        object->grey_references();

        reachable_objects.push_back(std::move(object));
      }

      num_greys = std::count_if(objects_.begin(), objects_.end(), is_grey);
    }

    objects_ = std::move(reachable_objects);
  }


  using ObjTracker =
      ObjectTracker<
        Stack<Value, max_stack_size>,
        std::list<UpvalueObject*>,
        StringHashTable<Value>
      >;


  template <typename T0, typename... Ts>
  T0* make_object(Ts&& ... args)
  {
      auto ptr = std::make_unique<T0>(std::forward<Ts>(args)...);
      auto ret = ptr.get();
      ObjTracker::instance().add_object(std::move(ptr));
      return ret;
  };


  StringObject* make_string(std::string std_str);


  void grey_roots(Stack<Value, max_stack_size>* stack);


  void grey_roots(std::list<UpvalueObject*>* upvalues);


  void grey_roots(StringHashTable<Value>* globals);


  template <typename T0, typename... Ts>
  void detail::grey_roots_impl(T0 head, Ts... tail)
  {
    grey_roots(head);
    grey_roots_impl(tail...);
  }


  template <std::size_t... Is, typename... Ts>
  void detail::grey_roots(std::index_sequence<Is...>, std::tuple<Ts...>& roots)
  {
    grey_roots_impl(std::get<Is>(roots)...);
  }


  template <typename... Ts>
  void detail::grey_roots(std::tuple<Ts...>& roots)
  {
    grey_roots(std::index_sequence_for<Ts...>(), roots);
  }
}

#endif //LOXX_OBJECTTRACKER_HPP
