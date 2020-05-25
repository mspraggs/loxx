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
 * Created by Matt Spraggs on 15/12/2018.
 */

#ifndef LOXX_JIT_ASSEMBLYWRAPPER_HPP
#define LOXX_JIT_ASSEMBLYWRAPPER_HPP

#include <cstdint>
#include <cstdlib>
#include <limits>
#include <memory>
#include <vector>

// TODO: Windows support
#include <sys/mman.h>


namespace loxx
{
  namespace jit
  {
    class AssemblyWrapper
    {
    public:
      AssemblyWrapper(const std::size_t capacity = 4096);

      AssemblyWrapper(AssemblyWrapper&&) = default;

      AssemblyWrapper& operator=(AssemblyWrapper&&) = default;

      void add_byte(const std::uint8_t byte);

      template <typename It>
      void add_bytes(const It begin, const It end);

      template <typename... Args>
      void add_bytes(Args&&... bytes);

      void add_bytes(std::initializer_list<std::uint8_t> bytes);

      void write_byte(const std::size_t pos, const std::uint8_t byte);

      template <typename It>
      void write_bytes(const std::size_t pos, const It begin, const It end);

      template <typename T>
      void write_integer(const std::size_t pos, const T value);

      void lock();

      std::size_t size() const { return std::distance(data_.get(), ptr_); }
      const std::uint8_t* start() const { return data_.get(); }
      const std::uint8_t* finish() const { return ptr_; }

    private:
      template <typename T>
      struct MUnMapper
      {
        void operator() (T* ptr) const;
        AssemblyWrapper* wrapper;
      };

      template <typename T>
      friend struct MUnMapper;

      void add_bytes_impl() {}

      template <typename... Args>
      void add_bytes_impl(const std::uint8_t byte0, Args&&... bytes);

      void check_locked() const;
      void check_unlocked() const;

      bool locked_;
      std::size_t capacity_;
      std::uint8_t* ptr_;
      std::unique_ptr<std::uint8_t[], MUnMapper<std::uint8_t>> data_;
    };


    template <typename It>
    void AssemblyWrapper::add_bytes(const It begin, const It end)
    {
      std::copy(begin, end, ptr_);
      ptr_ += std::distance(begin, end);
    }


    template <typename... Args>
    void AssemblyWrapper::add_bytes(Args&&... bytes)
    {
      add_bytes_impl(std::forward<Args>(bytes)...);
    }


    template <typename It>
    void AssemblyWrapper::write_bytes(
        const std::size_t pos, const It begin, const It end)
    {
      std::copy(begin, end, data_.get() + pos);
    }


    template <typename T>
    void AssemblyWrapper::write_integer(const std::size_t pos, const T value)
    {
      const auto ptr = reinterpret_cast<const std::uint8_t*>(&value);
      write_bytes(pos, ptr, ptr + sizeof(T));
    }


    template <typename... Args>
    void AssemblyWrapper::add_bytes_impl(
        const std::uint8_t byte0, Args&&... bytes)
    {
      add_byte(byte0);
      add_bytes_impl(std::forward<Args>(bytes)...);
    }


    template <typename T>
    void AssemblyWrapper::MUnMapper<T>::operator() (T* ptr) const
    {
      munmap(ptr, wrapper->capacity_);
    }
  }
}

#endif // LOXX_JIT_ASSEMBLYWRAPPER_HPP
