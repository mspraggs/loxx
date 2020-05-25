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
 * Created by Matt Spraggs on 15/12/2019.
 */

#include "AssemblyWrapper.hpp"
#include "JITError.hpp"


namespace loxx
{
  namespace jit
  {
    template <typename T>
    T* mmap_allocate(const std::size_t n)
    {
      if (n > std::numeric_limits<std::size_t>::max() / sizeof(T)) {
        throw std::bad_alloc();
      }

      auto ptr = static_cast<T*>(mmap(
          nullptr, n * sizeof(T),
          PROT_READ | PROT_WRITE,
          MAP_PRIVATE | MAP_ANONYMOUS,
          -1, 0));

      if (ptr == nullptr) {
        throw std::bad_alloc();
      }

      return ptr;
    }


    AssemblyWrapper::AssemblyWrapper(const std::size_t capacity)
        : capacity_(capacity),
          data_(
              mmap_allocate<std::uint8_t>(capacity),
              MUnMapper<std::uint8_t>{this})
    {
      ptr_ = data_.get();
    }


    void AssemblyWrapper::add_byte(const std::uint8_t byte)
    {
      *(ptr_++) = byte;
    }


    void AssemblyWrapper::add_bytes(std::initializer_list<std::uint8_t> bytes)
    {
      add_bytes(bytes.begin(), bytes.end());
    }


    void AssemblyWrapper::write_byte(
        const std::size_t pos, const std::uint8_t byte)
    {
      data_[pos] = byte;
    }


    void AssemblyWrapper::lock()
    {
      const auto result = mprotect(
          data_.get(), capacity_,
          PROT_READ | PROT_EXEC);
      if (result == -1) {
        throw JITError("memory lock failure");
      }
      locked_ = true;
    }


    void AssemblyWrapper::check_locked() const
    {
      if (not locked_) {
        throw JITError("invalid memory access");
      }
    }


    void AssemblyWrapper::check_unlocked() const
    {
      if (locked_) {
        throw JITError("invalid memory access");
      }
    }
  }
}
