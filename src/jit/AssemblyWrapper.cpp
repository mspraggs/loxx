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
    void AssemblyWrapper::add_byte(const std::uint8_t byte)
    {
      check_unlocked();
      assembly_.push_back(byte);
    }


    void AssemblyWrapper::write_byte(
        const std::size_t pos, const std::uint8_t byte)
    {
      check_unlocked();
      assembly_[pos] = byte;
    }


    void AssemblyWrapper::lock()
    {
      const auto result = mprotect(
          assembly_.data(), assembly_.capacity(),
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
