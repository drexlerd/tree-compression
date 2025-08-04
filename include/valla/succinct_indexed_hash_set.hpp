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

#ifndef VALLA_INCLUDE_SUCCINCT_INDEXED_HASH_SET_HPP_
#define VALLA_INCLUDE_SUCCINCT_INDEXED_HASH_SET_HPP_

#include "valla/declarations.hpp"
#include "valla/succinct_flat_hash_set.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <sdsl/int_vector.hpp>
#include <stack>

namespace valla
{
template<IsUint64tCodable T, std::unsigned_integral I, typename Hash = Hash<T>, typename EqualTo = EqualTo<T>>
class SuccinctIndexedHashSet
{
private:
    struct IndexReferencedHash
    {
        const sdsl::int_vector<>* vec;
        Hash hash;

        IndexReferencedHash(const sdsl::int_vector<>& vec) : vec(&vec), hash() {}

        size_t operator()(I el) const
        {
            assert(el < vec->size());
            return hash(T::from_uint64_t(vec->operator[](el), vec->width()));
        }
    };

    struct IndexReferencedEqualTo
    {
        const sdsl::int_vector<>* vec;
        EqualTo equal_to;

        IndexReferencedEqualTo(const sdsl::int_vector<>& vec) : vec(&vec), equal_to() {}

        size_t operator()(I lhs, I rhs) const
        {
            assert(lhs < vec->size());
            assert(rhs < vec->size());
            return equal_to(vec->operator[](lhs), vec->operator[](rhs));
        }
    };

    void resize_width(uint8_t old_width, uint8_t new_width)
    {
        /* Rebuild index_to_slot */
        auto slots = sdsl::int_vector<>(m_capacity, 0, new_width);

        for (I i = 0; i < m_size; ++i)
            slots[i] = Slot<I>::from_uint64_t(m_slots[i], old_width).to_uint64_t(new_width);

        std::swap(m_slots, slots);

        /* Rebuild uniqueness */
        m_uniqueness = absl::flat_hash_set<I, IndexReferencedHash, IndexReferencedEqualTo>(0, IndexReferencedHash(m_slots), IndexReferencedEqualTo(m_slots));
        for (I i = 0; i < m_size; ++i)
            m_uniqueness.emplace(i);
    }

public:
    SuccinctIndexedHashSet() :
        m_size(0),
        m_capacity(1),
        m_slots(1, 0, 2),  // size 0, value 0, width 2
        m_uniqueness(0, IndexReferencedHash(m_slots), IndexReferencedEqualTo(m_slots))
    {
    }
    // Uncopieable and unmoveable to avoid dangling references of m_slots in hash and equal_to.
    SuccinctIndexedHashSet(const SuccinctIndexedHashSet& other) = delete;
    SuccinctIndexedHashSet& operator=(const SuccinctIndexedHashSet& other) = delete;
    SuccinctIndexedHashSet(SuccinctIndexedHashSet&& other) = delete;
    SuccinctIndexedHashSet& operator=(SuccinctIndexedHashSet&& other) = delete;

    I insert(T slot)
    {
        assert(m_uniqueness.size() != std::numeric_limits<I>::max() && "SuccinctIndexedHashSet: Index overflow! The maximum number of slots reached.");

        /* Resize on insufficient capacity. */
        if (m_size == m_capacity)
            m_slots.resize(m_capacity <<= 1);

        const auto new_width = slot.bit_width();
        const auto old_width = m_slots.width();

        /* Rebuild on insufficient width. */
        if (new_width > old_width)
            resize_width(old_width, new_width);

        I index = m_size++;

        m_slots[index] = slot.to_uint64_t(new_width);
        const auto result = m_uniqueness.emplace(index);

        if (!result.second)
            --m_size;

        return *result.first;
    }

    T operator[](I index) const
    {
        assert(index < m_slots.size() && "Index out of bounds");

        return Slot<I>::from_uint64_t(m_slots[index], m_slots.width());
    }

    size_t size() const { return m_size; }

private:
    size_t m_size;
    size_t m_capacity;
    sdsl::int_vector<> m_slots;
    absl::flat_hash_set<I, IndexReferencedHash, IndexReferencedEqualTo> m_uniqueness;  // TODO: change to succinct_flat_hash_set
};
}

#endif
