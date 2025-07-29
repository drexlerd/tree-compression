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

#ifndef VALLA_INCLUDE_PLAIN_SWISS_HPP_
#define VALLA_INCLUDE_PLAIN_SWISS_HPP_

#include "valla/declarations.hpp"
#include "valla/indexed_hash_set.hpp"
#include "valla/unique_object_pool.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <concepts>
#include <iostream>
#include <stack>

namespace valla::plain::uint::swiss
{

///////////////////////////////////////////
/// General case with special leaf table
///////////////////////////////////////////

/**
 * Insert recursively
 */

template<std::input_iterator Iterator, typename T>
    requires std::same_as<std::iter_value_t<Iterator>, T>
inline Index insert_recursively(Iterator it, Iterator end, size_t size, IndexedHashSet<Slot>& table, IndexedHashSet<T>& leaf_table)
{
    /* Base cases */
    if (size == 1)
        return leaf_table.insert(*it);

    if (size == 2)
    {
        const auto i1 = leaf_table.insert(*it);
        const auto i2 = leaf_table.insert(*(it + 1));
        return table.insert(Slot(i1, i2));
    }

    /* Divide */
    const auto mid = std::bit_floor(size - 1);

    /* Conquer */
    const auto mid_it = it + mid;
    const auto i1 = insert_recursively(it, mid_it, mid, table, leaf_table);
    const auto i2 = insert_recursively(mid_it, end, size - mid, table, leaf_table);

    return table.insert(Slot(i1, i2));
}

template<std::ranges::input_range Range, typename T>
    requires std::same_as<std::ranges::range_value_t<Range>, T>
auto insert(const Range& state, IndexedHashSet<Slot>& table, IndexedHashSet<T>& leaf_table)
{
    // Note: O(1) for random access iterators, and O(N) otherwise by repeatedly calling operator++.
    const auto size = static_cast<Index>(std::distance(state.begin(), state.end()));

    if (size == 0)               ///< Special case for empty state.
        return EMPTY_ROOT_SLOT;  ///< Len 0 marks the empty state, the tree index can be arbitrary so we set it to 0.

    return Slot(insert_recursively(state.begin(), state.end(), size, table, leaf_table), size);
}

/**
 * Read recursively
 */

template<typename T>
inline void read_state_recursively(Index index, size_t size, const IndexedHashSet<Slot>& table, const IndexedHashSet<T>& leaf_table, std::vector<T>& ref_state)
{
    /* Base case */
    if (size == 1)
    {
        ref_state.push_back(leaf_table[index]);
        return;
    }

    const auto& slot = table[index];

    /* Base case */
    if (size == 2)
    {
        ref_state.push_back(leaf_table[slot.i1]);
        ref_state.push_back(leaf_table[slot.i2]);
        return;
    }

    /* Divide */
    const auto mid = std::bit_floor(size - 1);

    /* Conquer */
    read_state_recursively(slot.i1, mid, table, leaf_table, ref_state);
    read_state_recursively(slot.i2, size - mid, table, leaf_table, ref_state);
}

template<typename T>
inline void read_state(Index tree_index, size_t size, const IndexedHashSet<Slot>& table, const IndexedHashSet<T>& leaf_table, std::vector<T>& out_state)
{
    out_state.clear();

    if (size == 0)  ///< Special case for empty state.
        return;

    read_state_recursively(tree_index, size, table, leaf_table, out_state);
}

/// @brief Read the `out_state` from the given `root_index` from the `root_table`.
/// @param root_index is the index of the slot in the root table.
/// @param table is the tree table.
/// @param root_table is the root table.
/// @param out_state is the output state.
template<typename T>
inline void read_state(Slot root_slot, const IndexedHashSet<Slot>& table, const IndexedHashSet<T>& leaf_table, std::vector<T>& out_state)
{
    /* Observe: a root slot wraps the root tree_index together with the length that defines the tree structure! */
    read_state(root_slot.i1, root_slot.i2, table, leaf_table, out_state);
}

/**
 * ConstIterator
 */

template<typename T>
class const_iterator
{
private:
    const IndexedHashSet<Slot>* m_inner_table;
    const IndexedHashSet<T>* m_leaf_table;
    UniqueObjectPoolPtr<std::vector<Entry>> m_inner_stack;
    UniqueObjectPoolPtr<std::vector<T>> m_leaf_stack;
    std::optional<T> m_value;

    const IndexedHashSet<Slot>& inner_table() const
    {
        assert(m_inner_table);
        return *m_inner_table;
    }

    const IndexedHashSet<T>& leaf_table() const
    {
        assert(m_leaf_table);
        return *m_leaf_table;
    }

    void advance()
    {
        while (!(m_leaf_stack->empty() && m_inner_stack->empty()))
        {
            if (!m_leaf_stack->empty())
            {
                m_value = m_leaf_stack->back();
                m_leaf_stack->pop_back();
                return;
            }
            else
            {
                while (!m_inner_stack->empty())
                {
                    auto entry = m_inner_stack->back();
                    m_inner_stack->pop_back();

                    if (entry.m_size == 1)
                    {
                        m_leaf_stack->emplace_back(this->leaf_table()[entry.m_index]);
                        break;
                    }
                    else if (entry.m_size == 2)
                    {
                        const auto& slot = this->inner_table()[entry.m_index];
                        m_leaf_stack->emplace_back(this->leaf_table()[slot.i2]);
                        m_leaf_stack->emplace_back(this->leaf_table()[slot.i1]);
                        break;
                    }

                    assert(entry.m_size > 2);

                    const auto& slot = this->inner_table()[entry.m_index];

                    Index mid = std::bit_floor(entry.m_size - 1);

                    // Emplace i2 first to ensure i1 is visited first in dfs.
                    m_inner_stack->emplace_back(slot.i2, entry.m_size - mid);
                    m_inner_stack->emplace_back(slot.i1, mid);
                }
            }
        }

        m_value = std::nullopt;
    }

public:
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = value_type*;
    using reference = value_type;
    using iterator_category = std::input_iterator_tag;
    using iterator_concept = std::input_iterator_tag;

    const_iterator() : m_inner_table(nullptr), m_leaf_table(nullptr), m_inner_stack(), m_leaf_stack(), m_value(std::nullopt) {}
    const_iterator(const const_iterator& other) :
        m_inner_table(other.m_inner_table),
        m_leaf_table(other.m_leaf_table),
        m_inner_stack(other.m_inner_stack.clone()),
        m_leaf_stack(other.m_leaf_stack.clone()),
        m_value(other.m_value)
    {
    }
    const_iterator& operator=(const const_iterator& other)
    {
        if (*this != other)
        {
            m_inner_table = other.m_inner_table;
            m_leaf_table = other.m_leaf_table;
            m_inner_stack = other.m_inner_stack.clone();
            m_leaf_stack = other.m_leaf_stack.clone();
            m_value = other.m_value;
        }
        return *this;
    }
    const_iterator(const_iterator&& other) = default;
    const_iterator& operator=(const_iterator&& other) = default;
    const_iterator(const IndexedHashSet<Slot>& inner_table, const IndexedHashSet<T>& leaf_table, Slot root_slot, bool begin) :
        m_inner_table(&inner_table),
        m_leaf_table(&leaf_table),
        m_inner_stack(),
        m_leaf_stack(),
        m_value(std::nullopt)
    {
        assert(m_inner_table);

        if (begin && root_slot.i2 > 0)
        {
            m_inner_stack = get_stack_pool<std::vector<Entry>>().get_or_allocate();
            m_inner_stack->clear();
            m_leaf_stack = get_stack_pool<std::vector<T>>().get_or_allocate();
            m_leaf_stack->clear();

            if (root_slot.i2 == 1)
            {
                m_leaf_stack->emplace_back(this->leaf_table()[root_slot.i1]);
            }
            else if (root_slot.i2 == 2)
            {
                const auto& slot = this->inner_table()[root_slot.i1];
                m_leaf_stack->emplace_back(this->leaf_table()[slot.i2]);
                m_leaf_stack->emplace_back(this->leaf_table()[slot.i1]);
            }
            else if (root_slot.i2 > 2)
            {
                m_inner_stack->emplace_back(root_slot.i1, root_slot.i2);
            }

            advance();
        }
    }
    value_type operator*() const { return *m_value; }
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

template<typename T>
inline const_iterator<T> begin(Slot root, const IndexedHashSet<Slot>& table, const IndexedHashSet<T>& leaf_table)
{
    return const_iterator<T>(table, leaf_table, root, true);
}

template<typename T>
inline const_iterator<T> end(const IndexedHashSet<T>&)
{
    return const_iterator<T>();
}

template<typename T>
inline auto range(Slot root, const IndexedHashSet<Slot>& table, const IndexedHashSet<T>& leaf_table)
{
    return std::ranges::subrange(begin(root, table, leaf_table), end(leaf_table));
}

///////////////////////////////////////////
/// Special case with Index range
///////////////////////////////////////////

/**
 * Insert recursively
 */

template<std::input_iterator Iterator>
    requires std::same_as<std::iter_value_t<Iterator>, Index>
inline Index insert_recursively(Iterator it, Iterator end, size_t size, IndexedHashSet<Slot>& table)
{
    /* Base cases */
    if (size == 1)
        return *it;  ///< Skip node creation

    if (size == 2)
        return table.insert(Slot(*it, *(it + 1)));

    /* Divide */
    const auto mid = std::bit_floor(size - 1);

    /* Conquer */
    const auto mid_it = it + mid;
    const auto i1 = insert_recursively(it, mid_it, mid, table);
    const auto i2 = insert_recursively(mid_it, end, size - mid, table);

    return table.insert(Slot(i1, i2));
}

template<std::ranges::input_range Range>
    requires std::same_as<std::ranges::range_value_t<Range>, Index>
auto insert(const Range& state, IndexedHashSet<Slot>& table)
{
    // Note: O(1) for random access iterators, and O(N) otherwise by repeatedly calling operator++.
    const auto size = static_cast<Index>(std::distance(state.begin(), state.end()));

    if (size == 0)               ///< Special case for empty state.
        return EMPTY_ROOT_SLOT;  ///< Len 0 marks the empty state, the tree index can be arbitrary so we set it to 0.

    return Slot(insert_recursively(state.begin(), state.end(), size, table), size);
}

/**
 * Read recursively
 */

inline void read_state_recursively(Index index, size_t size, const IndexedHashSet<Slot>& table, IndexList& ref_state)
{
    /* Base case */
    if (size == 1)
    {
        ref_state.push_back(index);
        return;
    }

    const auto slot = table[index];

    /* Base case */
    if (size == 2)
    {
        ref_state.push_back(slot.i1);
        ref_state.push_back(slot.i2);
        return;
    }

    /* Divide */
    const auto mid = std::bit_floor(size - 1);

    /* Conquer */
    read_state_recursively(slot.i1, mid, table, ref_state);
    read_state_recursively(slot.i2, size - mid, table, ref_state);
}

inline void read_state(Index tree_index, size_t size, const IndexedHashSet<Slot>& table, IndexList& out_state)
{
    out_state.clear();

    if (size == 0)  ///< Special case for empty state.
        return;

    read_state_recursively(tree_index, size, table, out_state);
}

inline void read_state(Slot root_slot, const IndexedHashSet<Slot>& table, IndexList& out_state)
{
    /* Observe: a root slot wraps the root tree_index together with the length that defines the tree structure! */
    read_state(root_slot.i1, root_slot.i2, table, out_state);
}

/**
 * ConstIterator
 */

template<>
class const_iterator<Index>
{
private:
    const IndexedHashSet<Slot>* m_table;
    UniqueObjectPoolPtr<std::vector<Entry>> m_stack;
    Index m_value;

    static constexpr const Index END_POS = Index(-1);

    const IndexedHashSet<Slot>& table() const
    {
        assert(m_table);
        return *m_table;
    }

    void advance()
    {
        while (!m_stack->empty())
        {
            auto entry = m_stack->back();
            m_stack->pop_back();

            if (entry.m_size == 1)
            {
                m_value = entry.m_index;
                return;
            }

            const auto slot = table()[entry.m_index];

            Index mid = std::bit_floor(entry.m_size - 1);

            // Emplace i2 first to ensure i1 is visited first in dfs.
            m_stack->emplace_back(slot.i2, entry.m_size - mid);
            m_stack->emplace_back(slot.i1, mid);
        }

        m_value = END_POS;
    }

public:
    using difference_type = std::ptrdiff_t;
    using value_type = Index;
    using pointer = value_type*;
    using reference = value_type;
    using iterator_category = std::input_iterator_tag;
    using iterator_concept = std::input_iterator_tag;

    const_iterator() : m_table(nullptr), m_stack(), m_value(END_POS) {}
    const_iterator(const const_iterator& other) : m_table(other.m_table), m_stack(other.m_stack.clone()), m_value(other.m_value) {}
    const_iterator& operator=(const const_iterator& other)
    {
        if (*this != other)
        {
            m_table = other.m_table;
            m_stack = other.m_stack.clone();
            m_value = other.m_value;
        }
        return *this;
    }
    const_iterator(const_iterator&& other) = default;
    const_iterator& operator=(const_iterator&& other) = default;
    const_iterator(const IndexedHashSet<Slot>& table, Slot root, bool begin) : m_table(&table), m_stack(), m_value(END_POS)
    {
        assert(m_table);

        if (begin)
        {
            m_stack = get_stack_pool<std::vector<Entry>>().get_or_allocate();
            m_stack->clear();

            if (root.i2 > 0)  ///< Push to stack only if there leafs
            {
                m_stack->emplace_back(root.i1, root.i2);
                advance();
            }
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

inline const_iterator<Index> begin(Slot root, const IndexedHashSet<Slot>& table) { return const_iterator<Index>(table, root, true); }

inline const_iterator<Index> end() { return const_iterator<Index>(); }

inline auto range(Slot root, const IndexedHashSet<Slot>& table) { return std::ranges::subrange(begin(root, table), end()); }
}

#endif