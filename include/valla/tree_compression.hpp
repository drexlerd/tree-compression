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

#ifndef VALLA_INCLUDE_TREE_COMPRESSION_HPP_
#define VALLA_INCLUDE_TREE_COMPRESSION_HPP_

#include "declarations.hpp"
#include "indexed_hash_set.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>

namespace valla
{

inline Slot make_slot(Index lhs, Index rhs) { return Slot(lhs) << 32 | rhs; }

inline std::pair<Index, Index> read_slot(Slot slot) { return { Index(slot >> 32), slot & (Index(-1)) }; }

inline Index insert_recursively(State::const_iterator it, State::const_iterator end, IndexedHashSet& table)
{
    const auto len = static_cast<size_t>(std::distance(it, end));

    /* Base case */
    if (len <= 2)
    {
        return table.insert_slot((len == 1) ? make_slot(*it, NULL_INDEX) : make_slot(*it, *(it + 1))).first->second;
    }

    /* Divide */
    const auto mid = std::bit_floor(len - 1);

    /* Conquer */
    const auto left_index = insert_recursively(it, it + mid, table);
    const auto right_index = insert_recursively(it + mid, end, table);

    return table.insert_slot(make_slot(left_index, right_index)).first->second;
}

inline auto insert(const State& state, IndexedHashSet& tree_table, IndexedHashSet& root_table)
{
    assert(std::is_sorted(state.begin(), state.end()));

    if (state.size() == 0)  ///< Special case for empty state.
        return root_table.insert_slot(make_slot(tree_table.insert_slot(make_slot(NULL_INDEX, NULL_INDEX)).first->second, Index(0)));

    return root_table.insert_slot(make_slot(insert_recursively(state.begin(), state.end(), tree_table), state.size()));
}

inline void read_state_recursively(Index index, size_t len, const IndexedHashSet& tree_table, State& out_state)
{
    const auto [left_index, right_index] = read_slot(tree_table.get_slot(index));

    /* Base case */
    if (len <= 2)
    {
        out_state.push_back(left_index);

        if (len == 2)
            out_state.push_back(right_index);

        return;
    }

    /* Divide */
    const auto mid = std::bit_floor(len - 1);

    /* Conquer */
    read_state_recursively(left_index, mid, tree_table, out_state);
    read_state_recursively(right_index, len - mid, tree_table, out_state);
}

inline void read_state(Index root_index, const IndexedHashSet& tree_table, const IndexedHashSet& root_table, State& out_state)
{
    out_state.clear();

    const auto [tree_index, len] = read_slot(root_table.get_slot(root_index));

    if (len == 0)  ///< Special case for empty state.
        return;

    read_state_recursively(tree_index, len, tree_table, out_state);
}

}

#endif