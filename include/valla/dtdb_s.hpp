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

#ifndef VALLA_INCLUDE_DTDB_S_HPP_
#define VALLA_INCLUDE_DTDB_S_HPP_

#include "valla/indexed_hash_set.hpp"
#include "valla/slot.hpp"

#include <bit>
#include <cassert>
#include <concepts>
#include <iterator>
#include <limits>
#include <ranges>

namespace valla
{

/**
 * Insert recursively
 */

template<typename T, std::unsigned_integral I, std::random_access_iterator Iterator>
    requires std::same_as<std::iter_value_t<Iterator>, I>
inline I insert_sequence_recursively(Iterator it, I size, IndexedHashSet<T, I>& table)
{
    assert(size > I { 0 });

    if (size == I { 1 })
        return *it;  ///< Skip node creation

    if (size == I { 2 })
        return table.insert(Slot<I>(it[0], it[1]));

    const I mid = std::bit_floor(size - I { 1 });

    const I i1 = insert_sequence_recursively<T, I>(it, mid, table);
    const I i2 = insert_sequence_recursively<T, I>(it + mid, size - mid, table);

    return table.insert(Slot<I>(i1, i2));
}

template<typename T, std::unsigned_integral I, typename Range>
    requires std::ranges::random_access_range<const Range> && std::ranges::sized_range<const Range> && std::same_as<std::ranges::range_value_t<const Range>, I>
inline Slot<I> insert_sequence(const Range& sequence, IndexedHashSet<T, I>& table)
{
    const auto n = std::ranges::size(sequence);
    assert(n <= static_cast<decltype(n)>(std::numeric_limits<I>::max()));

    const I size = static_cast<I>(n);

    if (size == I { 0 })
        return Slot<I> {};  ///< Special case for empty sequence.

    return Slot<I> { insert_sequence_recursively<T, I>(std::ranges::begin(sequence), size, table), size };
}

/**
 * Read recursively
 */

template<typename T, std::unsigned_integral I, typename OutIterator>
    requires std::output_iterator<OutIterator, I>
inline void read_sequence_recursively(I index, I size, const IndexedHashSet<T, I>& table, OutIterator out)
{
    assert(size > I { 0 });

    if (size == I { 1 })
    {
        *out++ = index;
        return;
    }

    const auto slot = table.lookup(index);

    if (size == I { 2 })
    {
        *out++ = slot.i1;
        *out++ = slot.i2;
        return;
    }

    const I mid = std::bit_floor(size - I { 1 });

    read_sequence_recursively(slot.i1, mid, table, out);
    read_sequence_recursively(slot.i2, size - mid, table, out);
}

template<typename T, std::unsigned_integral I, typename OutIterator>
    requires std::output_iterator<OutIterator, I>
inline void read_sequence(const Slot<I>& root_slot, const IndexedHashSet<T, I>& table, OutIterator out)
{
    if (root_slot.i2 == I { 0 })  ///< Special case for empty sequence.
        return;

    /* Observe: a root slot wraps the root tree index together with the length that defines the tree structure! */
    read_sequence_recursively(root_slot.i1, root_slot.i2, table, out);
}

}

#endif