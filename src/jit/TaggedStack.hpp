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

#include <algorithm>
#include <array>


namespace loxx
{
  namespace jit
  {
    template <typename T, typename Tag>
    struct TaggedElem
    {
      bool has_tag(const Tag tag) const
      { return (tags & static_cast<std::uint8_t>(tag)) != 0; }
      void add_tag(const Tag tag) { tags |= static_cast<std::uint8_t>(tag); }
      void remove_tag(const Tag tag)
      { tags &= ~static_cast<std::uint8_t>(tag); }

      T value;
      std::uint8_t tags;
    };


    template <typename T, typename Tag, std::size_t N>
    class TaggedStack
    {
    public:
      using Elem = TaggedElem<T, Tag>;

      TaggedStack();

      void set(
          const std::size_t i, const T& value,
          std::initializer_list<Tag> tags = {});
      const T& get(const std::size_t i) const { return stack_[i].value; }
      const Elem& operator[] (const std::size_t i) const { return stack_[i]; }
      template <typename... Args>
      void emplace(Args&&... values);
      void push(const T& value, std::initializer_list<Tag> tags = {});
      T pop();
      const T& top(const std::size_t depth = 0) const
      { return stack_[top_ - depth - 1].value; }
      void reset_slot(const std::size_t i) { stack_[i].tags = 0; }

      std::size_t size() const { return top_; }
      void resize(const std::size_t size) { top_ = size; }

      bool has_tag(const std::size_t i, const Tag tag) const;
      void add_tag(const std::size_t i, const Tag tag);
      void remove_tag(const std::size_t i, const Tag tag);

      auto begin() { return stack_.begin(); }
      auto begin() const { return stack_.begin(); }
      auto end() { return stack_.begin() + top_; }
      auto end() const { return stack_.begin() + top_; }

    private:
      std::size_t top_;
      std::array<Elem, N> stack_;
    };


    template <typename T, typename Tag, std::size_t N>
    TaggedStack<T, Tag, N>::TaggedStack()
        : top_(0)
    {
    }


    template <typename T, typename Tag, std::size_t N>
    void TaggedStack<T, Tag, N>::set(
        const std::size_t i, const T& value, std::initializer_list<Tag> tags)
    {
      stack_[i] = Elem{value, 0};
      std::for_each(
          tags.begin(), tags.end(),
          [&] (const Tag tag) {
            stack_[i].add_tag(tag);
          });

      if (i >= top_) {
        top_ = i + 1;
      }
    }


    template <typename T, typename Tag, std::size_t N>
    template <typename... Args>
    void TaggedStack<T, Tag, N>::emplace(Args&&... values)
    {
      stack_[top_++] = Elem{T(std::forward<Args>(values)...), 0};
    }


    template <typename T, typename Tag, std::size_t N>
    void TaggedStack<T, Tag, N>::push(
        const T& value, std::initializer_list<Tag> tags)
    {
      stack_[top_] = Elem{value, 0};
      std::for_each(
          tags.begin(), tags.end(),
          [&] (const Tag tag) {
            stack_[top_].add_tag(tag);
          });
      ++top_;
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
      return stack_[i].has_tag(tag);
    }


    template <typename T, typename Tag, std::size_t N>
    void TaggedStack<T, Tag, N>::add_tag(const std::size_t i, const Tag tag)
    {
      stack_[i].add_tag(tag);
    }


    template <typename T, typename Tag, std::size_t N>
    void TaggedStack<T, Tag, N>::remove_tag(const std::size_t i, const Tag tag)
    {
      stack_[i].remove_tag(tag);
    }
  }
}

#endif // LOXX_JIT_TAGGEDSTACK_HPP
