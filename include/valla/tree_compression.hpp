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

inline std::pair<Index, Index> read_slot(Slot slot) { return { slot & (Slot(Index(-1)) << 32), slot & (Index(-1)) }; }

inline Index insert_recursively(State::const_iterator it, State::const_iterator end, IndexedHashSet& table)
{
    const auto len = static_cast<size_t>(std::distance(it, end));

    /* Base case */
    if (len <= 2)
    {
        const auto leaf_slot = (len == 0) ? make_slot(-1, -1) : (len == 1) ? make_slot(*it, -1) : make_slot(*it, *(it + 1));

        const auto leaf_slot_index = table.find_or_insert(leaf_slot).first;

        std::cout << "leaf_slot=" << leaf_slot << " leaf_slot_index=" << leaf_slot_index << " " << *it << " " << *(it + 1) << std::endl;

        return leaf_slot_index;
    }

    /* Divide */
    const auto mid = std::bit_floor(len - 1);

    /* Conquer */
    const auto left_index = insert_recursively(it, it + mid, table);
    const auto right_index = insert_recursively(it + mid, end, table);

    // Combine left and right index.
    const auto tree_slot = make_slot(left_index, right_index);

    const auto tree_slot_index = table.find_or_insert(tree_slot).first;

    std::cout << "tree_slot=" << tree_slot << " tree_slot_index=" << tree_slot_index << " left_index=" << left_index << " right_index=" << right_index
              << std::endl;

    return tree_slot_index;
}

inline std::pair<Index, bool> insert(const State& state, IndexedHashSet& tree_table, IndexedHashSet& root_table)
{
    assert(std::is_sorted(state.begin(), state.end()));

    const auto root_index = Slot(insert_recursively(state.begin(), state.end(), tree_table));

    // Combine root index and state size.
    const auto root_slot = make_slot(root_index, state.size());

    std::cout << "Insert: " << root_slot << " " << root_index << " " << state.size() << std::endl;

    const auto result = root_table.find_or_insert(root_slot);

    return result;
}

inline void read_state_recursively(Slot slot, size_t len, const IndexedHashSet& tree_table, State& out_state)
{
    const auto [left_index, right_index] = read_slot(tree_table.get(slot));

    /* Base case */
    if (len <= 2)
    {
        if (len == 1)
        {
            out_state.push_back(left_index);
        }
        else if (len == 2)
        {
            out_state.push_back(left_index);
            out_state.push_back(right_index);
        }
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

    const auto [tree_index, len] = read_slot(root_table.get(root_index));

    std::cout << "Read: " << root_table.get(root_index) << " " << tree_index << " " << len << std::endl;

    read_state_recursively(tree_index, len, tree_table, out_state);
    // size = 8 => depth 2 on both sides
    // size = 6 => depth 2 on left and depth 1 on right side
}

}

#endif