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

#include <algorithm>
#include <functional>
#include <iostream>
#include <numeric>

#include "CodeProfiler.hpp"


namespace loxx
{
  namespace detail
  {
    std::size_t combine_hashes(const std::size_t h0, const std::size_t h1)
    {
      return h0 ^ (0x9e3779b9 + (h1 << 6) + (h1 >> 2));
    }
  }

  void CodeProfiler::count_variable_type(
      const CodeObject* code, const StringObject* varname, const ValueType type)
  {
    const auto type_info = TypeInfo{code, varname, type};
    auto& count_elem = variable_type_counts_.insert(type_info, 0);
    count_elem->second += 1;
  }


  void CodeProfiler::count_function_call(
      const CodeObject* code, const ClosureObject* function,
      const std::size_t num_args, const Value* args)
  {
    const auto extract_type = [] (const Value& value) {
      return static_cast<ValueType>(value.index());
    };

    std::vector<ValueType> signature(num_args);
    std::transform(args, args + num_args, signature.begin(), extract_type);

    const auto call_info = CallInfo{code, function, signature};
    auto& count_elem = function_call_counts_.insert(call_info, 0);
    count_elem->second += 1;
  }


  void CodeProfiler::count_basic_block(
      const CodeObject* code, const CodeObject::InsPtr ip)
  {
    const auto block_info = BlockInfo{code, ip};
    auto& count_elem = block_counts_.insert(block_info, 0);
    count_elem->second += 1;
  }


  std::size_t CodeProfiler::TypeInfoHasher::operator() (
      const TypeInfo& info) const
  {
    using namespace detail;

    const auto varname_hash = info.varname->hash();
    const auto code_hash = std::hash<const CodeObject*>()(info.code);
    const auto type_hash = std::hash<ValueType>()(info.type);

    return combine_hashes(combine_hashes(varname_hash, code_hash), type_hash);
  }


  std::size_t CodeProfiler::CallInfoHasher::operator() (
      const CallInfo& info) const
  {
    using namespace detail;
    const std::size_t types_hash = hash_signature(info.signature);

    const auto code_hash = std::hash<const CodeObject*>()(info.code);
    const auto func_hash = std::hash<const ClosureObject*>()(info.function);

    return combine_hashes(combine_hashes(func_hash, code_hash), types_hash);
  }


  std::size_t CodeProfiler::CallInfoHasher::hash_signature(
      const CallSignature& signature) const
  {
    const auto type_hasher = std::hash<ValueType>();
    const auto hash_reduce_types =
        [&] (const std::size_t hash, const ValueType& type) {
          return (hash << 2) + type_hasher(type);
        };

    return std::accumulate(
        signature.begin(), signature.end(), 0UL, hash_reduce_types);
  }


  std::size_t CodeProfiler::InsPtrHasher::operator() (
      const CodeObject::InsPtr ptr) const
  {
    return ptr_hasher(&(*ptr));
  }


  std::size_t CodeProfiler::BlockInfoHasher::operator() (
      const BlockInfo& info) const
  {
    return ip_hasher(info.ip);
  }


  bool CodeProfiler::TypeInfoCompare::operator() (
      const TypeInfo& info1, const TypeInfo& info2) const
  {
    return
        info1.code == info2.code and
        info1.varname == info2.varname and
        info1.type == info2.type;
  }


  bool CodeProfiler::CallInfoCompare::operator() (
      const CallInfo& info1, const CallInfo& info2) const
  {
    return
        info1.code == info2.code and
        info1.function == info2.function and
        info1.signature == info2.signature;
  }


  bool CodeProfiler::BlockInfoCompare::operator() (
      const BlockInfo& info1, const BlockInfo& info2) const
  {
    return info1.ip == info2.ip;
  }
}
