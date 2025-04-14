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
#include <cassert>
#include <cmath>
#include <iostream>

namespace valla
{
class Root
{
private:
    Slot m_root;
    IndexedHashSet& m_tree_table;

    friend class const_iterator;

public:
    Root(Slot root, IndexedHashSet& tree_table);

    /**
     * Iterators
     */

    class const_iterator
    {
    private:
        Index m_node;
        Index m_size;
        IndexedHashSet& m_tree_table;

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = Index;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator();
        const_iterator(Slot root, IndexedHashSet& tree_table, bool begin);
        value_type operator*() const;
        const_iterator& operator++();
        const_iterator operator++(int);
        bool operator==(const const_iterator& other) const;
        bool operator!=(const const_iterator& other) const;
    };

    const_iterator begin() const;
    const_iterator end() const;
};
}

#endif
