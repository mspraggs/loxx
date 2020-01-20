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

#include "AssemblyFunction.hpp"
#include "JITError.hpp"


namespace loxx
{
  namespace jit
  {
    void AssemblyFunction::add_byte(const std::uint8_t byte)
    {
      check_lock();
      assembly_.push_back(byte);
    }


    void AssemblyFunction::lock()
    {
      const auto result = mprotect(
          assembly_.data(), assembly_.capacity(),
          PROT_READ | PROT_EXEC);
      if (result == -1) {
        throw JITError("memory lock failure");
      }
      locked_ = true;
    }


    bool AssemblyFunction::operator() () const
    {
      check_lock();
      const auto func = reinterpret_cast<bool (*) ()>(assembly_.data());
      return func();
    }


    void AssemblyFunction::check_lock() const
    {
      if (not locked_) {
        throw JITError("invalid memory access");
      }
    }
  }
}
