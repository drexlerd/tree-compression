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

#ifndef VALLA_INCLUDE_COMPACT_CLEARY_HASH_SET_HPP_
#define VALLA_INCLUDE_COMPACT_CLEARY_HASH_SET_HPP_

#include "valla/compact_hash.hpp"
#include "valla/equal_to.hpp"
#include "valla/growthinfo.hpp"
#include "valla/hash.hpp"
#include "valla/statistics.hpp"
#include "valla/uint64tcoder.hpp"
#include "valla/utils.hpp"

#include <absl/container/internal/raw_hash_set.h>
#include <sdsl/int_vector.hpp>

namespace valla
{
// Some further references:
// - https://arxiv.org/pdf/2109.04548
// - https://arxiv.org/pdf/cs/0512081
// - https://citeseerx.ist.psu.edu/document?repid=rep1&type=pdf&doi=b7620871f53d759a15cf9e820584b89f984f74c1
template<IsUint64tCodable T, std::unsigned_integral I, typename Hash = CompactHash<T>, typename EqualTo = EqualTo<T>>
class compact_cleary_hash_set
{
public:
    class const_iterator;
    friend class const_iterator;

    using value_type = T;
    using const_iterator_type = const_iterator;

private:
    GrowthInfo m_growth_info;

    // r[i] = r(T[i]) if T[i] != -1, and otherwise, r[i] = -1
    sdsl::int_vector<> m_R;
    sdsl::bit_vector m_E;
    // v[i] = 1 if exists j h(T[j]) = i, and otherwise, v[i] = 0.
    sdsl::bit_vector m_V;
    // c[i] = 1 if T[i] != T[i - 1] or T[i] = -1, and otherwise c[i] = 0
    sdsl::bit_vector m_C;
    // a[i] = a(i) if a(i) in [-Na,Na], and otherwise, a[i] = infinity
    sdsl::int_vector<4> m_A;

    Hash m_hash;
    EqualTo m_Equal_to;

    HashSetStatistics m_statistics;

private:
    inline uint64_t next_idx(uint64_t i) const noexcept { return (i + 1) & m_growth_info.mask(); }

    inline uint64_t prev_idx(uint64_t i) const noexcept { return (i - 1) & m_growth_info.mask(); }

    inline uint64_t quotient(uint64_t h) const noexcept
    {
        assert((h / m_growth_info.capacity()) < m_growth_info.capacity());
        return h / m_growth_info.capacity();
    }

    inline uint64_t remainder(uint64_t h) const noexcept { return h & m_growth_info.mask(); }

    inline uint64_t decode_remainder(uint64_t remainder, uint64_t quotient) const { return m_hash.invert_hash(remainder << m_R.width() | quotient); }

    inline uint64_t find_group(uint64_t i) const
    {
        // Search unsuccessful
        if (m_V[i] == 0)
            return m_growth_info.capacity();  ///< fail

        // Find first empty location down the table
        int count = 0;
        while (m_E[i] != 1)
        {
            count -= m_V[i];
            i = prev_idx(i);
        }

        // Search back to find uppermost member of relevant group
        while (count < 0)
        {
            i = next_idx(i);
            count += m_C[i];
        }

        return i;
    }

    inline std::pair<uint64_t, bool> find_within_group(uint64_t i, uint64_t r) const
    {
        // Search group associated with j
        while (m_R[next_idx(i)] <= r && m_C[next_idx(i)] == 0)
            i = next_idx(i);

        // Check last location to see if key found
        if (m_R[i] == r)
            return { i, true };  ///< success
        else
            return { next_idx(i), false };
    }

    inline uint64_t find_insert_location_within_group(uint64_t i, uint64_t r) const
    {
        while (m_R[i] < r && m_C[i] == 0)
            i = next_idx(i);
        return i;
    }

    inline uint64_t find_next_empty(uint64_t i) const
    {
        while (!m_E[i])
            i = next_idx(i);
        return i;
    }

    inline void shift_right_make_room(uint64_t i, uint64_t j)
    {
        for (uint64_t to = j; to != i; to = prev_idx(to))
        {
            const uint64_t from = prev_idx(to);
            m_R[to] = m_R[from];
            m_C[to] = m_C[from];
            m_E[to] = 0;
        }
    }

    /// @brief Inserts a key into the hash set.
    /// Assumes that there is space for the key.
    /// @param key is the key.
    /// @return the index where the key is stored.
    uint64_t insert_impl(const T& key)
    {
        uint64_t h = m_hash.hash(key);
        uint64_t q = quotient(h);
        uint64_t r = remainder(h);
        uint64_t i = q;

        i = find_group(i);
        if (i == m_growth_info.capacity())
        {
            i = q;
            i = find_next_empty(i);

            m_V[q] = 1;
            m_C[i] = 1;
            m_R[i] = r;

            return i;
        }
        else
        {
            const auto [i_, success] = find_within_group(i, r);
            if (success)
                return i_;

            i = i_;
            shift_right_make_room(i, find_next_empty(i));

            m_R[i] = r;
            m_E[i] = 0;

            return i;
        }
    }

    void resize_width(uint8_t old_width, uint8_t new_width)
    {
        auto r = sdsl::int_vector<>(capacity(), 0, new_width);

        if (size() > 0)
            for (I i = 0; i < capacity(); ++i)
            {
                if (!m_E[i])
                    r[i] = m_hash.hash(Uint64tCoder<T>::to_uint64_t(Uint64tCoder<T>::from_uint64_t(decode_remainder(m_R[i], i), old_width), new_width))
                           & m_growth_info.mask();
            }

        std::swap(m_R, r);
    }

public:
    compact_cleary_hash_set(size_t capacity = 1, uint8_t bit_width = 1, Hash hash = Hash {}, EqualTo equal_to = EqualTo {}) :
        m_growth_info(capacity),
        m_R(this->capacity()),
        m_E(this->capacity(), 1),
        m_V(this->capacity(), 0),
        m_A(this->capacity()),
        m_hash(hash),
        m_Equal_to(equal_to)
    {
        assert(bit_width > 0 && bit_width <= 64 && "bit_width must be in range [1,64].");
    }

    compact_cleary_hash_set(Hash hash = Hash {}, EqualTo equal_to = EqualTo {}) : compact_cleary_hash_set(1, 1, hash, equal_to) {}

    compact_cleary_hash_set() : compact_cleary_hash_set(Hash {}, EqualTo {}) {}

    std::pair<const_iterator, bool> insert(const T& key)
    {
        const auto new_width = Uint64tCoder<T>::bit_width(key);
        const auto old_width = m_slots.width();

        if (new_width > old_width)
            resize_width(old_width, new_width);

        if (m_growth_info.growth_left() == 0)
            rehash();

        return insert_impl(key);
    }

    void rehash() {}

    class const_iterator
    {
    private:
        const compact_cleary_hash_set* m_set;
        size_t m_bucket;
        size_t m_pos;

        const compact_cleary_hash_set& set() const
        {
            assert(m_set);
            return *m_set;
        }

        void advance()
        {
            do
            {
                ++m_pos;
            } while (m_pos < set().capacity() && static_cast<int>((set().m_Controls)[m_pos]) < 0);
        }

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = T*;
        using reference = T&;
        using iterator_category = std::forward_iterator_tag;
        using iterator_concept = std::forward_iterator_tag;

        const_iterator() : m_set(nullptr), m_pos(-1) {}

        const_iterator(const compact_cleary_hash_set& set, bool begin) : m_set(set), m_pos(begin ? -1 : set.capacity())
        {
            if (begin)
                advance();
        }

        const_iterator(const compact_cleary_hash_set& set, size_t bucket, size_t pos) : m_set(&set), m_bucket(bucket), m_pos(pos)
        {
            assert(static_cast<int>(set.m_Controls[pos]) >= 0);
        }

        value_type operator*() const { return Uint64tCoder<T>::from_uint64_t((set().m_slots)[m_pos], set().m_slots.width()); }

        const_iterator& operator++()
        {
            ++m_pos;
            advance();
            return *this;
        }

        const_iterator operator++(int)
        {
            const_iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const const_iterator& other) const { return m_pos == other.m_pos; }

        bool operator!=(const const_iterator& other) const { return !(*this == other); }
    };

    const_iterator begin() const { return const_iterator(*this, true); }
    const_iterator end() const { return const_iterator(*this, false); }

    const GrowthInfo& growth_info() const { return m_growth_info; }
    const std::vector<Bucket>& buckets() const { return m_buckets; }
    size_t size() const { return m_growth_info.size(); }
    size_t capacity() const { return m_growth_info.capacity(); }
    bool must_rehash() const { return must_rehash; }

    size_t mem_usage() const
    {
        size_t usage = 0;

        usage += sizeof(compact_cleary_hash_set);

        for (const auto& bucket : m_buckets)
        {
            usage += sizeof(Bucket);
            usage += bucket.controls.capacity() * sizeof(absl::container_internal::ctrl_t);
            usage += bucket.overflows.capacity() / 8;
        }

        return usage;
    }
};
}

#endif
