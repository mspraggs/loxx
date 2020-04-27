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
 * Created by Matt Spraggs on 26/04/2020.
 */

#ifndef LOXX_JIT_TAGGEDSTACK_HPP
#define LOXX_JIT_TAGGEDSTACK_HPP

#include <array>


namespace loxx
{
  namespace jit
  {
    template <typename T, typename Tag, std::size_t N>
    class TaggedStack
    {
    private:
    public:
      explicit TaggedStack(std::initializer_list<Tag> default_tags);

      void set(const std::size_t i, const T& value);
      const T& get(const std::size_t i) const { return stack_[i].value; }
      template <typename... Args>
      void emplace(Args&&... values);
      void push(const T& value);
      T pop();
      const T& top(const std::size_t depth = 0) const
      { return stack_[top_ - depth - 1].value; }
      void reset_slot(const std::size_t i) { stack_[i].tags = 0; }

      std::size_t size() const { return top_; }
      void resize(const std::size_t size) { top_ = size; }

      bool has_tag(const std::size_t i, const Tag tag) const;
      void add_tag(const std::size_t i, const Tag tag);
      void remove_tag(const std::size_t i, const Tag tag);

    private:
      struct Elem
      {
        T value;
        std::size_t tags;
      };

      std::size_t default_tags_;
      std::size_t top_;
      std::array<Elem, N> stack_;
    };


    template <typename T, typename Tag, std::size_t N>
    TaggedStack<T, Tag, N>::TaggedStack(std::initializer_list<Tag> default_tags)
        : default_tags_(0), top_(0)
    {
      for (const auto tag : default_tags) {
        default_tags_ |= static_cast<std::size_t>(tag);
      }
    }


    template <typename T, typename Tag, std::size_t N>
    void TaggedStack<T, Tag, N>::set(const std::size_t i, const T& value)
    {
      stack_[i] = Elem{value, default_tags_};

      if (i >= top_) {
        top_ = i + 1;
      }
    }


    template <typename T, typename Tag, std::size_t N>
    template <typename... Args>
    void TaggedStack<T, Tag, N>::emplace(Args&&... values)
    {
      stack_[top_++] = Elem{T(std::forward<Args>(values)...), default_tags_};
    }


    template <typename T, typename Tag, std::size_t N>
    void TaggedStack<T, Tag, N>::push(const T& value)
    {
      stack_[top_++] = Elem{value, default_tags_};
    }


    template <typename T, typename Tag, std::size_t N>
    T TaggedStack<T, Tag, N>::pop()
    {
      const auto ret = stack_[--top_].value;
      reset_slot(top_);
      return ret;
    }


    template <typename T, typename Tag, std::size_t N>
    bool TaggedStack<T, Tag, N>::has_tag(const std::size_t i, const Tag tag) const
    {
      return (stack_[i].tags & static_cast<std::size_t>(tag)) != 0;
    }


    template <typename T, typename Tag, std::size_t N>
    void TaggedStack<T, Tag, N>::add_tag(const std::size_t i, const Tag tag)
    {
      stack_[i].tags |= static_cast<std::size_t>(tag);
    }


    template <typename T, typename Tag, std::size_t N>
    void TaggedStack<T, Tag, N>::remove_tag(const std::size_t i, const Tag tag)
    {
      stack_[i].tags &= ~static_cast<std::size_t>(tag);
    }
  }
}

#endif // LOXX_JIT_TAGGEDSTACK_HPP
