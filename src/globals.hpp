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
 * Created by Matt Spraggs on 08/03/2018.
 */

#ifndef LOXX_GLOBALS_HPP
#define LOXX_GLOBALS_HPP

#include <cstddef>
#include <cstdint>
#include <type_traits>


#if defined __GNUC__ || defined __clang__ || defined __MINGW64__

#define LOXX_NOINLINE __attribute__((noinline))

#elif defined _MSC_VER

#define LOXX_NOINLINE __declspec(noinline)

#else

#define LOXX_NOINLINE

#endif


namespace loxx
{
  using InstrArgUByte = std::uint8_t;
  using InstrArgSByte = std::make_signed_t<InstrArgUByte>;
  using InstrArgUShort = std::uint16_t;
  using InstrArgSShort = std::make_signed_t<InstrArgUShort>;

  constexpr std::size_t max_scope_constants = 256;
  constexpr std::size_t max_call_frames     = 64;
  constexpr std::size_t max_stack_size      =
      max_call_frames * max_scope_constants;
}

#endif // LOXX_GLOBALS_HPP
