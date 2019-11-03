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

#ifndef LOXX_CODEPROFILER_HPP
#define LOXX_CODEPROFILER_HPP

#include <tuple>

#include "CodeObject.hpp"
#include "Object.hpp"


namespace loxx
{
  class CodeProfiler
  {
  public:
    void count_variable_type(
        const CodeObject* code, const StringObject* varname,
        const ValueType type);

    void count_function_call(
        const CodeObject* code, const ClosureObject* function,
        const std::size_t num_args, const Value* args);

    void count_basic_block(
        const CodeObject* code, const std::size_t ip_offset);

  private:
    struct TypeInfo
    {
      const CodeObject* code;
      const StringObject* varname;
      ValueType type;
    };

    using CallSignature = std::vector<ValueType>;

    struct CallInfo
    {
      const CodeObject* code;
      const ClosureObject* function;
      CallSignature signature;
    };

    struct BlockInfo
    {
      const CodeObject* code;
      std::size_t ip_offset;
    };

    struct TypeInfoHasher
    {
      std::size_t operator() (const TypeInfo& value) const;
    };

    struct TypeInfoCompare
    {
      bool operator() (const TypeInfo& info1, const TypeInfo& info2) const;
    };

    struct CallInfoHasher
    {
      std::size_t operator() (const CallInfo& value) const;
      std::size_t hash_signature(const CallSignature& signature) const;
    };

    struct BlockInfoHasher
    {
      std::size_t operator() (const BlockInfo& value) const;
    };

    struct CallInfoCompare
    {
      bool operator() (const CallInfo& info1, const CallInfo& info2) const;
    };

    struct BlockInfoCompare
    {
      bool operator() (const BlockInfo& info1, const BlockInfo& info2) const;
    };

    HashTable<TypeInfo, std::size_t, TypeInfoHasher, TypeInfoCompare>
        variable_type_counts_;
    HashTable<CallInfo, std::size_t, CallInfoHasher, CallInfoCompare>
        function_call_counts_;
    HashTable<BlockInfo, std::size_t, BlockInfoHasher, BlockInfoCompare>
        block_counts_;
  };
}

#endif // LOXX_CODEPROFILER_HPP
