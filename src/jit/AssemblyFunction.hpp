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

#ifndef LOXX_JIT_ASSEMBLYFUNCTION_HPP
#define LOXX_JIT_ASSEMBLYFUNCTION_HPP

#include <cstdint>
#include <cstdlib>
#include <limits>
#include <vector>

// TODO: Windows support
#include <sys/mman.h>


namespace loxx
{
  namespace jit
  {
    class AssemblyFunction
    {
    public:
      AssemblyFunction(const std::size_t reserve_size = 4096)
          : locked_(false)
      {
        assembly_.reserve(reserve_size);
      }
      
      void add_byte(const std::uint8_t byte);

      template <typename Iter>
      void add_bytes(const Iter begin, const Iter end);

      void write_byte(const std::size_t pos, const std::uint8_t byte);

      template <typename Iter>
      void write_bytes(
          const std::size_t pos, const Iter begin, const Iter end);

      void lock();

      bool operator() () const;

      std::size_t size() const { return assembly_.size(); }

    private:
      template <typename T>
      struct MmapAllocator
      {
        using value_type = T;

        T* allocate(const std::size_t n);
        void deallocate(T* ptr, const std::size_t n);
      };

      void check_locked() const;
      void check_unlocked() const;

      bool locked_;
      std::vector<std::uint8_t, MmapAllocator<std::uint8_t>> assembly_;
    };


    template <typename Iter>
    void AssemblyFunction::add_bytes(const Iter begin, const Iter end)
    {
      check_unlocked();
      assembly_.insert(assembly_.end(), begin, end);
    }


    template <typename Iter>
    void AssemblyFunction::write_bytes(
        const std::size_t pos, const Iter begin, const Iter end)
    {
      check_unlocked();
      std::copy(begin, end, assembly_.begin() + pos);
    }


    template <typename T>
    T* AssemblyFunction::MmapAllocator<T>::allocate(const std::size_t n)
    {
      if (n > std::numeric_limits<std::size_t>::max() / sizeof(T)) {
        throw std::bad_alloc();
      }

      auto ptr = static_cast<T*>(mmap(
          nullptr, n * sizeof(T),
          PROT_READ | PROT_WRITE | PROT_EXEC,
          MAP_PRIVATE | MAP_ANONYMOUS,
          -1, 0));

      if (ptr == nullptr) {
        throw std::bad_alloc();
      }

      return ptr;
    }


    template <typename T>
    void AssemblyFunction::MmapAllocator<T>::deallocate(
        T* ptr, const std::size_t n)
    {
      munmap(ptr, n);
    }
  }
}

#endif // LOXX_JIT_ASSEMBLYFUNCTION_HPP
