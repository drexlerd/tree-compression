/*
 * Copyright (C) 2025 Dominik Drexler
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef VALLA_INCLUDE_SEQUENCE_CODER_HPP_
#define VALLA_INCLUDE_SEQUENCE_CODER_HPP_

#include "valla/indexed_hash_set.hpp"

#include <concepts>
#include <iterator>
#include <ranges>

namespace valla
{

template<typename T, std::unsigned_integral I, std::ranges::input_range Range, std::output_iterator<I> OutIterator>
    requires std::same_as<std::ranges::range_value_t<const Range>, T>
inline void encode_as_unsigned_integrals(const Range& sequence, IndexedHashSet<T, I>& table, OutIterator out)
{
    for (const auto& element : sequence)
        *out++ = table.insert(element);
}

template<typename T, std::unsigned_integral I, std::ranges::input_range Range, std::output_iterator<I> OutIterator>
    requires std::same_as<std::ranges::range_value_t<const Range>, I>
inline void decode_from_unsigned_integrals(const Range& sequence, const IndexedHashSet<T, I>& table, OutIterator out)
{
    for (const auto& index : sequence)
        *out++ = table.lookup(index);
}

}

#endif