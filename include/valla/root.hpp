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

#ifndef VALLA_INCLUDE_RANGE_HPP_
#define VALLA_INCLUDE_RANGE_HPP_

#include "declarations.hpp"
#include "indexed_hash_set.hpp"

#include <algorithm>
#include <bit>
#include <cassert>
#include <cmath>
#include <concepts>
#include <iostream>
#include <ranges>

namespace valla
{
class Root
{
private:
    IndexedHashSet& m_tree_table;
    Index m_index;
    uint32_t m_size;

    friend class const_iterator;

public:
    Root(IndexedHashSet& tree_table, Slot root) : m_tree_table(tree_table), m_index(read_pos(root, 0)), m_size(read_pos(root, 1)) {}

    /**
     * Iterators
     */

    class const_iterator
    {
    private:
        IndexedHashSet* m_tree_table;

        struct Entry
        {
            Index m_index;
            uint32_t m_size;
        };

        std::vector<Entry> m_stack;

        Index m_value;

        static constexpr const Index END_POS = Index(-1);

        void advance()
        {
            while (!m_stack.empty())
            {
                auto entry = m_stack.back();
                m_stack.pop_back();

                if (entry.m_size == 1)
                {
                    m_value = entry.m_index;
                    return;
                }

                const auto [left, right] = read_slot(m_tree_table->get_slot(entry.m_index));

                Index mid = std::bit_floor(entry.m_size - 1);

                // Emplace right first to ensure left is visited first in dfs.
                m_stack.emplace_back(right, entry.m_size - mid);
                m_stack.emplace_back(left, mid);
            }

            m_value = END_POS;
        }

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = Index;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator() : m_tree_table(nullptr), m_stack(), m_value(END_POS) {}
        const_iterator(IndexedHashSet& tree_table, Index node, Index size, bool begin) : m_tree_table(&tree_table), m_stack(), m_value(END_POS)
        {
            if (begin)
            {
                m_stack.emplace_back(node, size);
                advance();
            }
        }
        value_type operator*() const { return m_value; }
        const_iterator& operator++()
        {
            advance();
            return *this;
        }
        const_iterator operator++(int)
        {
            auto it = *this;
            ++it;
            return it;
        }
        bool operator==(const const_iterator& other) const { return m_value == other.m_value; }
        bool operator!=(const const_iterator& other) const { return !(*this == other); }
    };

    const_iterator begin() const { return const_iterator(m_tree_table, m_index, m_size, true); }
    const_iterator end() const { return const_iterator(m_tree_table, m_index, m_size, false); }
};
}

#endif
