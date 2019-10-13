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
 * Created by Matt Spraggs on 13/10/2019.
 */

#include <functional>
#include <iostream>

#include "CodeProfiler.hpp"


namespace loxx
{
  namespace detail
  {
    std::size_t compute_variable_type_hash(
        const CodeObject* code, const StringObject* varname,
        const std::size_t type)
    {
      const auto varname_hash = varname->hash();
      const auto code_hash = std::hash<const CodeObject*>()(code);

      const auto combine_hashes =
          [] (const std::size_t h0, const std::size_t h1) {
            return h0 ^ (0x9e3779b9 + (h1 << 6) + (h1 >> 2));
          };

      return combine_hashes(combine_hashes(varname_hash, code_hash), type);
    }
  }

  void CodeProfiler::count_variable_type(
      const CodeObject* code, const StringObject* varname,
      const std::size_t type)
  {
    const auto hash = detail::compute_variable_type_hash(code, varname, type);
    const auto type_info = VariableTypeInfo{code, varname, type, 0};
    auto& count_elem = variable_type_counts_.insert(hash, type_info);
    count_elem->second.count += 1;
  }
}
