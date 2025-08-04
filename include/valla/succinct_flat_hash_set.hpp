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

#ifndef VALLA_INCLUDE_SUCCINCT_FLAT_HASH_SET_HPP_
#define VALLA_INCLUDE_SUCCINCT_FLAT_HASH_SET_HPP_

#include "valla/declarations.hpp"

#include <absl/container/internal/raw_hash_set.h>
#include <sdsl/int_vector.hpp>

namespace valla
{
template<IsUint64tCodable T, std::unsigned_integral I, typename Hash = Hash<T>, typename EqualTo = EqualTo<T>, size_t InitialCapacity = 127>
class succinct_flat_hash_set
{
private:
    static_assert(((InitialCapacity + 1) & InitialCapacity) == 0, "InitialCapacity must be 2^{InitialCapacity}-1.");
    static_assert(InitialCapacity >= 127, "InitialCapacity must be greater than 127.");

    sdsl::int_vector<> m_slots;
    std::vector<absl::container_internal::ctrl_t> m_controls;
    absl::container_internal::CommonFields m_common;

    Hash m_hash;
    EqualTo m_equal_to;

    HashSetStatistics m_statistics;

public:
    class const_iterator;

    using value_type = T;
    using const_iterator_type = const_iterator;

    std::pair<const_iterator, bool> insert(const T& key)
    {
        assert(size() < capacity() && "Insert failed. Rehashing to higher capacity is required.");

        size_t h = m_hash(slot);
        absl::container_internal::h2_t h2 = h >> 57;

        absl::container_internal::probe_seq<absl::container_internal::Group::kWidth> probe(h, m_common.capacity());

        while (true)
        {
            absl::container_internal::Group group(&m_controls[probe.offset()]);

            for (const auto offset : group.Match(h2))
            {
                m_statistics.m_sum_probe_lengths += offset;
                size_t idx = probe.offset() + offset;
                assert(is_within_bounds(m_slots, idx));

                if (m_equal_to(m_slots[idx], slot))

                    return idx;
            }

            auto mask_empty = group.MaskEmpty();
            if (mask_empty)
            {
                int offset = mask_empty.LowestBitSet();
                m_statistics.m_sum_probe_lengths += offset;

                size_t idx = probe.offset() + offset;
                assert(is_within_bounds(m_slots, idx));

                m_slots[idx] = slot;
                m_controls[idx] = static_cast<absl::container_internal::ctrl_t>(h2);
                m_common.increment_size();
                ++m_statistics.m_num_probes;
                return idx;
            }

            probe.next();
            m_statistics.m_sum_probe_lengths += absl::container_internal::Group::kWidth;
        }
    }

    const_iterator find(const T& key) {}

    class const_iterator
    {
    private:
        const sdsl::int_vector<>* m_slots;
        size_t m_pos;
        I m_value;

    public:
    };

    size_t size() const {}
    size_t capacity() const {}
    double load_factor() const { return static_cast<double>(size()) / capacity(); }
    constexpr double max_load_factor() const { return MAX_LOAD_FACTOR; }

    size_t mem_usage() const
    {
        size_t usage = 0;
        usage += m_slots.capacity() * sizeof(T);
        usage += m_controls.capacity() * sizeof(absl::container_internal::ctrl_t);
        return usage;
    }
};
}

#endif
