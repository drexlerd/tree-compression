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

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>

namespace valla
{

inline Slot make_slot(Index lhs, Index rhs) { return Slot(lhs) << 32 | rhs; }

inline Index insert_recursively(State::const_iterator it, State::const_iterator end, IndexedHashSet& table)
{
    const auto len = static_cast<size_t>(std::distance(it, end));

    /* Base case */
    if (len <= 2)
    {
        const auto leaf_slot = (len == 0) ? make_slot(-1, -1) : (len == 1) ? make_slot(*it, -1) : make_slot(*it, *(it + 1));

        const auto leaf_slot_index = table.emplace(leaf_slot, table.size()).first->second;

        return leaf_slot_index;
    }

    /* Divide */
    const auto mid = std::bit_floor(len - 1);

    /* Conquer */
    const auto left_index = insert_recursively(it, it + mid, table);
    const auto right_index = insert_recursively(it + mid, end, table);

    // Combine left and right index.
    const auto tree_slot = make_slot(left_index, right_index);

    const auto tree_slot_index = table.emplace(tree_slot, table.size()).first->second;

    return tree_slot_index;
}

inline std::pair<Index, bool> insert(const State& state, IndexedHashSet& tree_table, IndexedHashSet& root_table)
{
    assert(std::is_sorted(state.begin(), state.end()));

    const auto root_index = Slot(insert_recursively(state.begin(), state.end(), tree_table));

    // Combine root index and state size.
    const auto root_slot = make_slot(root_index, state.size());

    const auto result = root_table.emplace(root_slot, root_table.size());

    return { result.first->second, result.second };
}

}

#endif