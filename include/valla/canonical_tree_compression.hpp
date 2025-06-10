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

#ifndef VALLA_INCLUDE_CANONICAL_TREE_COMPRESSION_HPP_
#define VALLA_INCLUDE_CANONICAL_TREE_COMPRESSION_HPP_

#include "valla/bitset_pool.hpp"
#include "valla/declarations.hpp"
#include "valla/indexed_hash_set.hpp"
#include "valla/root_slot.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <concepts>
#include <iostream>
#include <ranges>
#include <stack>

namespace valla::canonical
{

/// @brief Recursively insert the elements from `it` until `end` into the `table`.
/// @param it points to the first element.
/// @param end points after the last element.
/// @param size is the number of elements in the range from it to end.
/// @param view is the ordering.
/// @param bit is the position in the ordering for the next tree node being created.
/// @param table is the table to uniquely insert the slots.
/// @return the index of the slot at the root.
template<std::forward_iterator Iterator>
    requires std::same_as<std::iter_value_t<Iterator>, Index>
inline Index insert_recursively(Iterator it, Iterator end, size_t size, BitsetView view, size_t& bit, RootIndexedHashSet& table)
{
    /* Base cases */
    if (size == 1)
        return *it;  ///< Skip node creation

    if (size == 2)
    {
        return table.insert_slot(make_slot(*it, *(it + 1))).first->second;
    }

    /* Divide */
    const auto mid = std::bit_floor(size - 1);

    /* Conquer */
    const auto mid_it = it + mid;
    const auto left_index = insert_recursively(it, mid_it, mid, view, bit, table);
    const auto right_index = insert_recursively(mid_it, end, size - mid, view, bit, table);

    return table.insert_slot(make_slot(left_index, right_index)).first->second;
}

/// @brief Inserts the elements from the given `state` into the `tree_table` and the `root_table`.
/// @param state is the given state.
/// @param tree_table is the tree table whose nodes encode the tree structure without size information.
/// @param root_table is the root_table whose nodes encode the root tree index + the size of the state that defines the tree structure.
/// @return A pair (it, bool) where it points to the entry in the root table and bool is true if and only if the state was newly inserted.
template<std::ranges::forward_range Range>
    requires std::same_as<std::ranges::range_value_t<Range>, Index>
auto insert(const Range& state, IndexedHashSet& tree_table, RootIndexedHashSet& root_table, BitsetPool& pool)
{
    assert(std::is_sorted(state.begin(), state.end()));

    // Note: O(1) for random access iterators, and O(N) otherwise by repeatedly calling operator++.
    const auto size = static_cast<size_t>(std::distance(state.begin(), state.end()));

    if (size == 0)  ///< Special case for empty state.
        return root_table.insert_slot(
            RootSlot(make_slot(Index(0), Index(0)), pool.allocate(0)));  ///< Len 0 marks the empty state, the tree index can be arbitrary so we set it to 0.

    auto ordering = pool.allocate(size + 1);  // for sorted leafs, we could skil size/2, but it is not costly so we keep it general

    size_t bit = 0;
    return root_table.insert_slot(RootSlot(make_slot(insert_recursively(state.begin(), state.end(), size, ordering, bit, tree_table), size), ordering));
}

}

#endif