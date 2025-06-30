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

#ifndef VALLA_INCLUDE_HASH_ID_SET_HPP_
#define VALLA_INCLUDE_HASH_ID_SET_HPP_

#include "valla/declarations.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace valla
{
template<typename Hash = std::hash<Slot>, typename EqualTo = std::equal_to<Slot>, size_t BucketSize = 8>
class HashIdMap
{
private:
    static_assert(BucketSize != 0 && (BucketSize & (BucketSize - 1)) == 0 && "BucketSize must be a power of two.");

    static constexpr Index INDEX_SENTINEL = std::numeric_limits<Index>::max();

    std::vector<Index> m_root_to_unstable;
    std::vector<Index> m_unstable_to_root;

    std::vector<Slot> m_bucket_data;
    std::vector<uint8_t> m_bucket_sizes;
    size_t m_num_buckets;
    size_t m_size;
    size_t m_capacity;

    static size_t hash(Slot slot, size_t num_buckets) { return Hash {}(slot) % num_buckets; }

    struct RehashData
    {
        size_t num_buckets;
        size_t capacity;
        std::vector<Index> root_to_unstable;
        std::vector<Index> unstable_to_root;
        std::vector<Slot> bucket_data;
        std::vector<uint8_t> bucket_sizes;
        std::vector<Index> remapping;

        RehashData(size_t num_buckets, size_t capacity) :
            num_buckets(num_buckets),
            capacity(capacity),
            root_to_unstable(capacity),
            unstable_to_root(capacity),
            bucket_data(capacity),
            bucket_sizes(num_buckets, 0),
            remapping(capacity, INDEX_SENTINEL)
        {
        }
    };

    struct RehashTriggered : public std::exception
    {
    };

    Index rehash_recursively(Index unstable_index, size_t size, RehashData& tmp)
    {
        /* Base case 1: unstable index was already relocated */
        if (tmp.remapping[unstable_index] != INDEX_SENTINEL)
            return tmp.remapping[unstable_index];

        /* Base case 2: skipped node creation */
        if (size == 1)
            return unstable_index;

        const auto& slot = lookup(unstable_index);

        /* Base case 3: rellocate slot */
        if (size == 2)
        {
            size_t h = hash(slot, tmp.num_buckets);
            size_t offset = BucketSize * h;

            for (size_t i = 0; i < tmp.bucket_sizes[h]; ++i)
            {
                Index unstable_index = offset + i;

                if (EqualTo {}(tmp.bucket_data[unstable_index], slot))
                    return unstable_index;
            }

            if (tmp.bucket_sizes[h] == BucketSize)
                throw RehashTriggered {};

            Index unstable_index = offset + m_bucket_sizes[h]++;

            m_bucket_data[unstable_index] = slot;

            return unstable_index;
        }

        /* Divide */
        const auto mid = std::bit_floor(size - 1);

        /* Conquer */
        Index i1 = rehash_recursively(slot.i1, mid, tmp);
        Index i2 = rehash_recursively(slot.i2, size - mid, tmp);

        Slot new_slot(i1, i2);

        size_t h = hash(new_slot, tmp.num_buckets);
        size_t offset = BucketSize * h;

        for (size_t i = 0; i < tmp.bucket_sizes[h]; ++i)
        {
            Index unstable_index = offset + i;

            if (EqualTo {}(tmp.bucket_data[unstable_index], new_slot))
                return unstable_index;
        }

        if (tmp.bucket_sizes[h] == BucketSize)
            throw RehashTriggered {};

        unstable_index = offset + tmp.bucket_sizes[h]++;

        tmp.bucket_data[unstable_index] = new_slot;
        ++m_size;

        return unstable_index;
    }

    void rehash(double factor = 2.)
    {
        try
        {
            size_t new_num_buckets = factor * m_num_buckets;
            size_t new_capacity = factor * m_capacity;

            auto tmp = RehashData(new_num_buckets, new_capacity);

            for (Index stable_index = 0; stable_index < m_size; ++stable_index)
            {
                if (m_root_to_unstable[stable_index] != INDEX_SENTINEL)
                {
                    const auto& root = lookup_root(stable_index);
                    Index unstable_index = root.i1;
                    Index size = root.i2;
                    unstable_index = rehash_recursively(unstable_index, size, tmp);

                    tmp.root_to_unstable[stable_index] = unstable_index;
                    tmp.unstable_to_root[unstable_index] = stable_index;
                }
            }

            m_num_buckets = new_num_buckets;
            m_capacity = new_capacity;
            std::swap(m_root_to_unstable, tmp.root_to_unstable);
            std::swap(m_unstable_to_root, tmp.unstable_to_root);
            std::swap(m_bucket_data, tmp.bucket_data);
            std::swap(m_bucket_sizes, tmp.bucket_sizes);
        }
        catch (const RehashTriggered&)
        {
            rehash(2 * factor);
        }
    }

public:
    HashIdMap(size_t num_buckets = 1024) :
        m_root_to_unstable(),
        m_unstable_to_root(),
        m_bucket_data(),
        m_bucket_sizes(),
        m_num_buckets(num_buckets),
        m_size(0),
        m_capacity(num_buckets * BucketSize)
    {
        m_root_to_unstable.resize(m_capacity, INDEX_SENTINEL);
        m_unstable_to_root.resize(m_capacity, INDEX_SENTINEL);
        m_bucket_data.resize(m_capacity);
        m_bucket_sizes.resize(m_num_buckets, 0);
    }

    /// @brief Insert a slot and return an index that is stable until the next insertion.
    /// @param slot
    /// @return
    Index insert(Slot slot)
    {
        while (true)
        {
            size_t h = hash(slot, m_num_buckets);
            size_t offset = BucketSize * h;

            for (size_t i = 0; i < m_bucket_sizes[h]; ++i)
            {
                Index unstable_index = offset + i;

                if (EqualTo {}(m_bucket_data[unstable_index], slot))
                    return unstable_index;
            }

            if (m_bucket_sizes[h] == BucketSize)
            {
                rehash();
                continue;
            }

            Index unstable_index = offset + m_bucket_sizes[h]++;

            m_bucket_data[unstable_index] = slot;
            ++m_size;

            return unstable_index;
        }
    }

    /// @brief Insert the root slot and return a **stable** index.
    /// @param slot
    /// @return
    Index insert_root(Slot slot)
    {
        while (true)
        {
            size_t h = hash(slot, m_num_buckets);
            size_t offset = BucketSize * h;

            for (size_t i = 0; i < m_bucket_sizes[h]; ++i)
            {
                Index unstable_index = offset + i;

                if (EqualTo {}(m_bucket_data[unstable_index], slot))
                    return m_unstable_to_root[unstable_index];
            }

            if (m_bucket_sizes[h] == BucketSize)
            {
                rehash();
                continue;
            }

            Index unstable_index = offset + m_bucket_sizes[h]++;

            m_bucket_data[unstable_index] = slot;

            Index stable_index = m_size++;
            m_root_to_unstable[stable_index] = unstable_index;
            m_unstable_to_root[unstable_index] = stable_index;

            return stable_index;
        }
    }

    Slot lookup(Index unstable_index) { return m_bucket_data[unstable_index]; }

    Slot lookup_root(Index stable_index) { return m_bucket_data[m_root_to_unstable[stable_index]]; }

    size_t size() const { return m_size; }
    size_t capacity() const { return m_capacity; }
    size_t num_buckets() const { return m_num_buckets; }
};
}

#endif