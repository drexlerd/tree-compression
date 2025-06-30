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

#ifndef VALLA_INCLUDE_TREE_DATABASE_HPP_
#define VALLA_INCLUDE_TREE_DATABASE_HPP_

#include "valla/declarations.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace valla
{
template<typename Hash = std::hash<Slot>, typename EqualTo = std::equal_to<Slot>, size_t BucketSize = 8>
class TreeDatabase
{
private:
    static_assert(BucketSize != 0 && (BucketSize & (BucketSize - 1)) == 0 && "BucketSize must be a power of two.");

    static constexpr Index INDEX_SENTINEL = std::numeric_limits<Index>::max();

    std::vector<Index> m_root_to_unstable;
    std::vector<Index> m_unstable_to_root;
    size_t m_num_roots;

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
            root_to_unstable(capacity, INDEX_SENTINEL),
            unstable_to_root(capacity, INDEX_SENTINEL),
            bucket_data(capacity),
            bucket_sizes(num_buckets, 0),
            remapping(capacity, INDEX_SENTINEL)
        {
        }
    };

    struct RehashTriggered : public std::exception
    {
    };

    Index insert(Slot slot, RehashData& tmp)
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

        Index unstable_index = offset + tmp.bucket_sizes[h]++;

        tmp.bucket_data[unstable_index] = slot;

        return unstable_index;
    }

    Index rehash_recursively(Index unstable_index, size_t size, RehashData& tmp)
    {
        /* Base case 1: unstable index was already relocated */
        if (tmp.remapping[unstable_index] != INDEX_SENTINEL)
            return tmp.remapping[unstable_index];

        /* Base case 2: skipped node creation */
        if (size == 1)
            return unstable_index;

        const auto& slot = m_bucket_data[unstable_index];

        /* Base case 3: rellocate slot */
        if (size == 2)
        {
            Index new_unstable_index = insert(slot, tmp);
            tmp.remapping[unstable_index] = new_unstable_index;
            return new_unstable_index;
        }

        /* Divide */
        const auto mid = std::bit_floor(size - 1);

        /* Conquer */
        Index i1 = rehash_recursively(slot.i1, mid, tmp);
        Index i2 = rehash_recursively(slot.i2, size - mid, tmp);

        Index new_unstable_index = insert(Slot(i1, i2), tmp);
        tmp.remapping[unstable_index] = new_unstable_index;
        return new_unstable_index;
    }

    void rehash(double factor = 2.)
    {
        try
        {
            size_t new_num_buckets = factor * m_num_buckets;
            size_t new_capacity = factor * m_capacity;

            auto tmp = RehashData(new_num_buckets, new_capacity);

            for (Index stable_index = 0; stable_index < m_num_roots; ++stable_index)
            {
                assert(m_root_to_unstable[stable_index] != INDEX_SENTINEL);
                assert(m_root_to_unstable[stable_index] == root.i1);

                Index unstable_index = m_root_to_unstable[stable_index];

                const auto& root = m_bucket_data[unstable_index];

                Index new_unstable_index = insert(Slot(rehash_recursively(root.i1, root.i2, tmp), root.i2), tmp);
                tmp.remapping[unstable_index] = new_unstable_index;
                tmp.root_to_unstable[stable_index] = new_unstable_index;
                tmp.unstable_to_root[new_unstable_index] = stable_index;
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

    Index insert(Slot slot)
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
            throw RehashTriggered {};

        Index unstable_index = offset + m_bucket_sizes[h]++;

        m_bucket_data[unstable_index] = slot;
        ++m_size;

        return unstable_index;
    }

    std::pair<Index, bool> insert_checked(Slot slot)
    {
        size_t h = hash(slot, m_num_buckets);
        size_t offset = BucketSize * h;

        for (size_t i = 0; i < m_bucket_sizes[h]; ++i)
        {
            Index unstable_index = offset + i;

            if (EqualTo {}(m_bucket_data[unstable_index], slot))
                return { unstable_index, false };
        }

        if (m_bucket_sizes[h] == BucketSize)
            throw RehashTriggered {};

        Index unstable_index = offset + m_bucket_sizes[h]++;

        m_bucket_data[unstable_index] = slot;
        ++m_size;

        return { unstable_index, true };
    }

    template<std::input_iterator Iterator>
        requires std::same_as<std::iter_value_t<Iterator>, Index>
    inline Index insert_recursively(Iterator it, Iterator end, size_t size)
    {
        /* Base cases */
        if (size == 1)
            return *it;  ///< Skip node creation

        if (size == 2)
            return insert(Slot(*it, *(it + 1)));

        /* Divide */
        const auto mid = std::bit_floor(size - 1);

        /* Conquer */
        const auto mid_it = it + mid;
        const auto i1 = insert_recursively(it, mid_it, mid);
        const auto i2 = insert_recursively(mid_it, end, size - mid);

        return insert(Slot(i1, i2));
    }

public:
    TreeDatabase(size_t num_buckets = 1024) :
        m_root_to_unstable(),
        m_unstable_to_root(),
        m_num_roots(0),
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

    template<std::ranges::input_range Range>
        requires std::same_as<std::ranges::range_value_t<Range>, Index>
    Index insert(const Range& range)
    {
        assert(std::is_sorted(range.begin(), range.end()));

        // Note: O(1) for random access iterators, and O(N) otherwise by repeatedly calling operator++.
        const auto size = static_cast<Index>(std::distance(range.begin(), range.end()));

        if (size == 0)  ///< Special case for empty range.
            return 0;   ///< Len 0 marks the empty range, the tree index can be arbitrary so we set it to 0.

        double factor = 1.0;

        while (true)
        {
            try
            {
                const auto [unstable_index, success] = insert_checked(Slot(insert_recursively(range.begin(), range.end(), size), size));

                if (success)
                {
                    Index stable_index = m_num_roots++;
                    m_root_to_unstable[stable_index] = unstable_index;
                    m_unstable_to_root[unstable_index] = stable_index;
                }

                return m_unstable_to_root[unstable_index];
            }
            catch (const RehashTriggered&)
            {
                factor *= 2;
                rehash(factor);
            }
        }
    }

    size_t num_roots() const { return m_num_roots; }
    size_t size() const { return m_size; }
    size_t capacity() const { return m_capacity; }
    size_t num_buckets() const { return m_num_buckets; }

    friend std::ostream& operator<<(std::ostream& os, const TreeDatabase& db)
    {
        os << "Bucket data: " << db.m_bucket_data << "\n"
           << "Bucket sizes: " << db.m_bucket_sizes << "\n"
           << "Root to unstable: " << db.m_root_to_unstable << "\n"
           << "Unstable to root: " << db.m_unstable_to_root;
        return os;
    }
};
}

#endif