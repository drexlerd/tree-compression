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
#include "valla/details/unique_object_pool.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace valla::tdb
{

struct Entry
{
    Index m_index;
    Index m_size;

    Entry(Index index, Index size) : m_index(index), m_size(size) {}
};

inline void copy(const std::vector<Entry>& src, std::vector<Entry>& dst)
{
    dst.clear();
    dst.insert(dst.end(), src.begin(), src.end());
}

static thread_local UniqueObjectPool<std::vector<Entry>> s_stack_pool = UniqueObjectPool<std::vector<Entry>> {};

template<typename Hash = std::hash<Slot>, typename EqualTo = std::equal_to<Slot>, size_t BucketSize = 8>
class TreeDatabase
{
private:
    static_assert(BucketSize != 0 && (BucketSize & (BucketSize - 1)) == 0 && "BucketSize must be a power of two.");

    static constexpr Index INDEX_SENTINEL = std::numeric_limits<Index>::max();

    std::vector<Index> m_stable_to_unstable;
    std::vector<Index> m_unstable_to_stable;
    size_t m_num_stable;

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
        std::vector<Index> stable_to_unstable;
        std::vector<Index> unstable_to_stable;
        std::vector<Slot> bucket_data;
        std::vector<uint8_t> bucket_sizes;
        std::vector<Index> remapping;

        RehashData(size_t num_buckets, size_t capacity) :
            num_buckets(num_buckets),
            capacity(capacity),
            stable_to_unstable(capacity, INDEX_SENTINEL),
            unstable_to_stable(capacity, INDEX_SENTINEL),
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

            for (Index stable_index = 0; stable_index < m_num_stable; ++stable_index)
            {
                assert(m_stable_to_unstable[stable_index] != INDEX_SENTINEL);

                Index unstable_index = m_stable_to_unstable[stable_index];

                const auto& root = m_bucket_data[unstable_index];

                assert(m_stable_to_unstable[stable_index] == root.i1);

                Index new_unstable_index = insert(Slot(rehash_recursively(root.i1, root.i2, tmp), root.i2), tmp);
                tmp.remapping[unstable_index] = new_unstable_index;
                tmp.stable_to_unstable[stable_index] = new_unstable_index;
                tmp.unstable_to_stable[new_unstable_index] = stable_index;
            }

            m_num_buckets = new_num_buckets;
            m_capacity = new_capacity;
            std::swap(m_stable_to_unstable, tmp.stable_to_unstable);
            std::swap(m_unstable_to_stable, tmp.unstable_to_stable);
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
        m_stable_to_unstable(),
        m_unstable_to_stable(),
        m_num_stable(0),
        m_bucket_data(),
        m_bucket_sizes(),
        m_num_buckets(num_buckets),
        m_size(0),
        m_capacity(num_buckets * BucketSize)
    {
        m_stable_to_unstable.resize(m_capacity, INDEX_SENTINEL);
        m_unstable_to_stable.resize(m_capacity, INDEX_SENTINEL);
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
                    Index stable_index = m_num_stable++;
                    m_stable_to_unstable[stable_index] = unstable_index;
                    m_unstable_to_stable[unstable_index] = stable_index;
                }

                return m_unstable_to_stable[unstable_index];
            }
            catch (const RehashTriggered&)
            {
                factor *= 2;
                rehash(factor);
            }
        }
    }

    /**
     * ConstIterator
     */

    class const_iterator
    {
    private:
        const TreeDatabase* m_db;
        UniqueObjectPoolPtr<std::vector<Entry>> m_stack;
        Index m_value;

        static constexpr const Index END_POS = Index(-1);

        const TreeDatabase& db() const
        {
            assert(m_db);
            return *m_db;
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

                const auto slot = db().m_bucket_data[entry.m_index];

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

        const_iterator() : m_db(nullptr), m_stack(), m_value(END_POS) {}
        const_iterator(const const_iterator& other) : m_db(other.m_db), m_stack(other.m_stack.clone()), m_value(other.m_value) {}
        const_iterator& operator=(const const_iterator& other)
        {
            if (*this != other)
            {
                m_db = other.m_db;
                m_stack = other.m_stack.clone();
                m_value = other.m_value;
            }
            return *this;
        }
        const_iterator(const_iterator&& other) = default;
        const_iterator& operator=(const_iterator&& other) = default;
        const_iterator(const TreeDatabase& db, Index stable_index, bool begin) : m_db(&db), m_stack(), m_value(END_POS)
        {
            assert(m_db);

            if (begin)
            {
                m_stack = s_stack_pool.get_or_allocate();
                m_stack->clear();

                const auto& root = db.m_bucket_data[db.m_stable_to_unstable[stable_index]];

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

    const_iterator begin(Index stable_index) const { return const_iterator(*this, stable_index, true); }
    const_iterator end() const { return const_iterator(); }

    size_t num_stable() const { return m_num_stable; }
    size_t size() const { return m_size; }
    size_t capacity() const { return m_capacity; }
    size_t num_buckets() const { return m_num_buckets; }

    friend std::ostream& operator<<(std::ostream& os, const TreeDatabase& db)
    {
        os << "Bucket data: " << db.m_bucket_data << "\n"
           << "Bucket sizes: " << db.m_bucket_sizes << "\n"
           << "Stable to unstable: " << db.m_stable_to_unstable << "\n"
           << "Unstable to stable: " << db.m_unstable_to_stable;
        return os;
    }
};
}

#endif