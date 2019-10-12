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
  namespace detail
  {
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


  StringObject* make_string(std::string std_str)
  {
    static HashSet<StringObject*, HashStringObject, CompareStringObject> cache;

    auto str_obj = std::make_unique<StringObject>(std::move(std_str));

    const auto cached = cache.find(
        str_obj.get(),
        [&] (StringObject* candidate) {
          return candidate->as_std_string() == str_obj->as_std_string();
        });

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


  namespace detail
  {
    void grey_roots_impl() {}


    template <typename T0, typename... Ts>
    void grey_roots_impl(T0 head, Ts... tail)
    {
      grey_roots(head);
      grey_roots_impl(tail...);
    }


    template <std::size_t... Is, typename... Ts>
    void grey_roots(std::index_sequence<Is...>, std::tuple<Ts...>& roots)
    {
      grey_roots_impl(std::get<Is>(roots)...);
    }


    template <typename... Ts>
    void grey_roots(std::tuple<Ts...>& roots)
    {
      grey_roots(std::index_sequence_for<Ts...>(), roots);
    }
  }
}