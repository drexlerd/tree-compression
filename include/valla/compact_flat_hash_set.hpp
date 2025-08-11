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

#ifndef VALLA_INCLUDE_COMPACT_FLAT_HASH_SET_HPP_
#define VALLA_INCLUDE_COMPACT_FLAT_HASH_SET_HPP_

#include "valla/compact_hash.hpp"
#include "valla/concepts.hpp"
#include "valla/equal_to.hpp"
#include "valla/growthinfo.hpp"
#include "valla/statistics.hpp"
#include "valla/utils.hpp"

#include <absl/container/flat_hash_map.h>
#include <absl/container/internal/raw_hash_set.h>
#include <cstddef>
#include <cstdint>
#include <sdsl/int_vector.hpp>
#include <vector>

namespace valla
{
template<IsUint64tCodable T, std::unsigned_integral I, typename Hash = CompactHash<uint64_t>, typename EqualTo = EqualTo<T>>
class compact_flat_hash_set
{
public:
    class const_iterator;
    friend class const_iterator;

    using value_type = T;
    using const_iterator_type = const_iterator;

private:
    enum class disp_t : uint8_t
    {
        kOverflow = 255,
    };

    GrowthInfo m_growth_info;

    sdsl::int_vector<> m_slots;
    std::vector<absl::container_internal::ctrl_t> m_controls;
    std::vector<disp_t> m_displacement;
    absl::flat_hash_map<uint32_t, int32_t> m_displacement_ext;

    uint8_t m_width;

    Hash m_hash;
    EqualTo m_equal_to;

    HashSetStatistics m_statistics;

    static inline size_t H1(size_t hash) { return (hash >> 7); }
    static inline absl::container_internal::h2_t H2(size_t hash) { return hash & 0x7F; }

    inline uint64_t decode_key(size_t i) const
    {
        uint64_t h1 = m_slots[i];
        uint64_t h2 = static_cast<uint64_t>(m_controls[i]);
        uint64_t h = (h1 << 7) | h2;
        return m_hash.invert_hash(h, m_slots.width() + 7);
    }

private:
    std::pair<const_iterator, bool> insert_impl(const T& key)
    {
        assert(size() < capacity() && "Insert failed. Rehashing to higher capacity is required.");
        assert(Uint64tCoder<T>::bit_width(key) <= m_slots.width() && "Insert failed. Slot width is insufficient to store the key.");

        m_statistics.increment_num_probes();

        const auto h = m_hash.hash(Uint64tCoder<T>::to_uint64_t(key, m_slots.width() + 7), m_slots.width() + 7);

        assert(std::bit_width(h) <= m_slots.width());

        const auto h1 = H1(h);
        const auto h2 = H2(h);

        absl::container_internal::probe_seq<absl::container_internal::Group::kWidth> probe(h, m_growth_info.mask());

        const auto initial_offset = probe.offset();

        while (true)
        {
            absl::container_internal::Group group(&m_controls[probe.offset()]);

            for (const auto i : group.Match(h2))
            {
                m_statistics.increase_total_probe_length(i);

                size_t offset = probe.offset(i);
                assert(is_within_bounds(m_slots, offset));

                if (m_equal_to(m_slots[offset], h1))
                    return { const_iterator(*this, offset), false };
            }

            auto mask_empty = group.MaskEmpty();
            if (mask_empty)
            {
                int i = mask_empty.LowestBitSet();

                m_statistics.increase_total_probe_length(i);

                size_t offset = probe.offset(i);

                assert(is_within_bounds(m_slots, offset));
                assert(m_controls[offset] == absl::container_internal::ctrl_t::kEmpty);

                m_slots[offset] = h1;
                m_controls[offset] = static_cast<absl::container_internal::ctrl_t>(h2);
                if (offset < absl::container_internal::NumClonedBytes())
                    m_controls[capacity() + offset] = static_cast<absl::container_internal::ctrl_t>(h2);

                uint32_t d = (offset - initial_offset) & m_growth_info.mask();
                if (d < static_cast<uint8_t>(disp_t::kOverflow))
                    m_displacement[offset] = static_cast<disp_t>(d);
                else
                {
                    m_displacement[offset] = disp_t::kOverflow;
                    m_displacement_ext[offset] = d;
                }

                m_growth_info.increment_size();

                return { const_iterator(*this, offset), true };
            }

            m_statistics.increase_total_probe_length(absl::container_internal::Group::kWidth);

            probe.next();
        }
    }

    void resize_width(uint8_t old_width, uint8_t new_width)
    {
        auto slots = sdsl::int_vector<>(capacity(), 0, new_width);

        if (size() > 0)
        {
            for (I i = 0; i < capacity(); ++i)
                if (static_cast<int>(m_controls[i]) >= 0)
                {
                    uint64_t h = m_hash.hash(Uint64tCoder<T>::to_uint64_t(Uint64tCoder<T>::from_uint64_t(decode_key(i), old_width), new_width), new_width);
                    assert(std::bit_width(h) <= m_slots.width());

                    const auto h1 = H1(h);
                    const auto h2 = H2(h);

                    m_slots[i] = h1;
                    m_controls[i] = static_cast<absl::container_internal::ctrl_t>(h2);
                }
        }

        std::swap(m_slots, slots);
    }

public:
    compact_flat_hash_set(size_t capacity = absl::container_internal::Group::kWidth,
                          uint8_t bit_width = 1,
                          Hash hash = Hash {},
                          EqualTo equal_to = EqualTo {}) :
        m_growth_info(capacity),
        m_slots(this->capacity(), 0, bit_width),  ///< bit width must be at least one, else it is set to 64
        m_controls(this->capacity() + absl::container_internal::NumClonedBytes(), absl::container_internal::ctrl_t::kEmpty),
        m_displacement(this->capacity()),
        m_displacement_ext(),
        m_hash(hash),
        m_equal_to(equal_to)
    {
        assert(bit_width > 0 && bit_width <= 64 && "bit_width must be in range [1,64].");
    }

    compact_flat_hash_set(Hash hash, EqualTo equal_to) : compact_flat_hash_set(absl::container_internal::Group::kWidth, 1, hash, equal_to) {}

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

    void rehash()
    {
        auto tmp = compact_flat_hash_set(2 * capacity(), m_slots.width(), m_hash, m_equal_to);

        for (size_t i = 0; i < capacity(); ++i)
            if (static_cast<int>(m_controls[i]) >= 0)
                tmp.insert(Uint64tCoder<T>::from_uint64_t(decode_key(i), m_slots.width() + 7));

        tmp.m_statistics += m_statistics;

        std::swap(*this, tmp);
    }

    class const_iterator
    {
    private:
        const compact_flat_hash_set* m_set;
        size_t m_pos;

        const compact_flat_hash_set& set() const
        {
            assert(m_set);
            return *m_set;
        }

        void advance()
        {
            do
            {
                ++m_pos;
            } while (m_pos < set().capacity() && static_cast<int>((set().m_controls)[m_pos]) < 0);
        }

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = T*;
        using reference = T&;
        using iterator_category = std::forward_iterator_tag;
        using iterator_concept = std::forward_iterator_tag;

        const_iterator() : m_set(nullptr), m_pos(-1) {}

        const_iterator(const compact_flat_hash_set& set, bool begin) : m_set(&set), m_pos(begin ? -1 : set.capacity())
        {
            if (begin)
                advance();
        }

        const_iterator(const compact_flat_hash_set& set, size_t pos) : m_set(&set), m_pos(pos) { assert(static_cast<int>(set.m_controls[pos]) >= 0); }

        value_type operator*() const { return Uint64tCoder<T>::from_uint64_t(set().decode_key(m_pos), set().m_slots.width() + 7); }

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
    const sdsl::int_vector<>& slots() const { return m_slots; }
    const std::vector<absl::container_internal::ctrl_t>& controls() const { return m_controls; }
    size_t size() const { return m_growth_info.size(); }
    size_t capacity() const { return m_growth_info.capacity(); }
    uint8_t bit_width() const { return m_slots.width(); }

    size_t mem_usage() const
    {
        size_t usage = 0;
        usage += m_slots.capacity() / 8;
        usage += m_controls.capacity() * sizeof(absl::container_internal::ctrl_t);
        return usage;
    }
};
}

#endif
