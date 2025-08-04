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

#ifndef VALLA_INCLUDE_INDEXED_HASH_SET_HPP_
#define VALLA_INCLUDE_INDEXED_HASH_SET_HPP_

#include "valla/declarations.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <stack>

namespace valla
{
template<typename T, std::unsigned_integral I, typename Hash = Hash<T>, typename EqualTo = EqualTo<T>>
class IndexedHashSet
{
public:
    using ValueType = T;
    using IndexType = I;

    IndexedHashSet() : m_slots(), m_uniqueness(0, IndexReferencedHash<T, I, Hash>(m_slots), IndexReferencedEqualTo<T, I, EqualTo>(m_slots)) {}
    // Uncopieable and unmoveable to avoid dangling references of m_slots in hash and equal_to.
    IndexedHashSet(const IndexedHashSet& other) = delete;
    IndexedHashSet& operator=(const IndexedHashSet& other) = delete;
    IndexedHashSet(IndexedHashSet&& other) = delete;
    IndexedHashSet& operator=(IndexedHashSet&& other) = delete;

    I insert(T slot)
    {
        assert(m_uniqueness.size() != std::numeric_limits<I>::max() && "IndexedHashSet: Index overflow! The maximum number of slots reached.");

        I index = m_slots.size();

        m_slots.push_back(slot);

        const auto result = m_uniqueness.emplace(index);

        if (!result.second)
            m_slots.pop_back();

        return *result.first;
    }

    const T& operator[](I index) const
    {
        assert(index < m_slots.size() && "Index out of bounds");

        return m_slots[index];
    }

    size_t size() const { return m_slots.size(); }

    size_t mem_usage() const
    {
        size_t usage = 0;
        usage += m_slots.capacity() * sizeof(T);
        usage += m_uniqueness.capacity() * (sizeof(I) + sizeof(absl::container_internal::ctrl_t));
        return usage;
    }

private:
    std::vector<T> m_slots;
    absl::flat_hash_set<I, IndexReferencedHash<T, I, Hash>, IndexReferencedEqualTo<T, I, EqualTo>> m_uniqueness;

    template<std::unsigned_integral I_, typename Hash_, typename EqualTo_, size_t InitialCapacity_>
    friend class TreeHashIDMap;
};

}

#endif
