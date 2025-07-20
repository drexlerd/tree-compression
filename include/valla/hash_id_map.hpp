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

#ifndef VALLA_INCLUDE_HASH_ID_MAP_HPP_
#define VALLA_INCLUDE_HASH_ID_MAP_HPP_

#include "valla/declarations.hpp"
#include "valla/indexed_hash_set.hpp"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <immintrin.h>  // for SSE2, AVX2, etc.
#include <vector>

namespace valla
{

/// @brief `ctrl_t` implements the control byte in a Swiss table.
enum class ctrl_t : int8_t
{
    kEmpty = -128,   // 0b10000000
    kDeleted = -2,   // 0b11111110
    kSentinel = -1,  // 0b11111111
};

alignas(16) inline static const __m128i kEmptyPattern = _mm_set1_epi8(static_cast<signed char>(ctrl_t::kEmpty));

/// @brief `HashIDMap` implements a hash ID map with open addressing in a Swiss table format where the position of a key implicitly becomes the index.
/// @tparam Key is the key.
/// @tparam Hash is the hash functor for a key.
/// @tparam EqualTo is the equality comparison functor for a key.
/// @tparam RehashPolicy is the rehash policy that gets executed upon rehash.
/// @tparam InitialCapacity is the initial capacity, which must be a multiplicative of 16.
template<typename Key, typename RehashPolicy, typename Hash = std::hash<Key>, typename EqualTo = std::equal_to<Key>, size_t InitialCapacity = 1024>
class HashIDMap
{
private:
    static_assert(InitialCapacity % 16 == 0, "InitialCapacity must be a multiple of 16.");

    static constexpr Index INDEX_SENTINEL = std::numeric_limits<Index>::max();  ///< used to indicate insertion failure to trigger a rehash.

    static constexpr double MAX_LOAD_FACTOR = static_cast<double>(7) / 8;

    RehashPolicy m_rehash_policy;

    std::vector<Key> m_slots;
    std::vector<ctrl_t> m_controls;
    size_t m_size;
    size_t m_capacity;

    Hash m_hash;
    EqualTo m_equal_to;

    friend RehashPolicy;

    struct Statistics
    {
        size_t m_num_rehashes = 0;
        std::chrono::milliseconds m_total_rehash_time = std::chrono::milliseconds::zero();
        size_t m_num_probes = 0;
        size_t m_sum_probe_lengths = 0;
    };

    Statistics m_statistics;

    void rehash(double factor = 2.)
    {
        using clock = std::chrono::high_resolution_clock;

        auto start = clock::now();  // Start timing

        ++m_statistics.m_num_rehashes;

        size_t new_capacity = factor * m_capacity;

        m_rehash_policy(*this, new_capacity);

        auto end = clock::now();
        m_statistics.m_total_rehash_time += std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    }

public:
    HashIDMap(RehashPolicy& rehash_policy) :
        m_rehash_policy(rehash_policy),
        m_slots(),
        m_controls(),
        m_size(0),
        m_capacity(InitialCapacity),
        m_hash(),
        m_equal_to()
    {
        m_slots.resize(m_capacity);

        // Sentinel-padded rolling buffer
        m_controls.reserve(m_capacity + 15);
        m_controls.resize(m_capacity, ctrl_t::kEmpty);
        m_controls.resize(m_capacity + 15, ctrl_t::kSentinel);
    }

    Index insert(const Key& slot)
    {
        size_t h = m_hash(slot);
        size_t mask = (m_capacity - 1);
        size_t i = h & mask;
        ctrl_t ctrl = static_cast<ctrl_t>(h >> 57);
        assert(static_cast<int8_t>(ctrl) >= 0);

        while (true)
        {
            assert(i < m_capacity);
            assert(i + 15 < m_controls.size());

            // Load 16 control bytes
            __m128i ctrl_block = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&m_controls[i]));
            __m128i broadcast_ctrl = _mm_set1_epi8(static_cast<signed char>(ctrl));

            // Compare against ctrl byte
            __m128i cmp_ctrl = _mm_cmpeq_epi8(ctrl_block, broadcast_ctrl);
            int mask_ctrl = _mm_movemask_epi8(cmp_ctrl);

            // Check if slot exists
            while (mask_ctrl != 0)
            {
                int offset = __builtin_ctz(mask_ctrl);
                size_t idx = (i + offset) & mask;

                if (m_equal_to(m_slots[idx], slot))
                {
                    m_statistics.m_sum_probe_lengths += offset;
                    return idx;
                }

                mask_ctrl &= mask_ctrl - 1;  // Clear the lowest set bit
            }

            // Compare against kEmpty
            __m128i cmp_empty = _mm_cmpeq_epi8(ctrl_block, kEmptyPattern);
            int mask_empty = _mm_movemask_epi8(cmp_empty);

            // Second: insert into first empty slot if found
            if (mask_empty != 0)
            {
                int offset = __builtin_ctz(mask_empty);
                m_statistics.m_sum_probe_lengths += offset;
                size_t idx = (i + offset) & mask;

                if (m_controls[idx] == ctrl_t::kEmpty)
                {
                    assert(m_controls[idx] == ctrl_t::kEmpty && "Unexpected overwrite!");

                    m_slots[idx] = slot;
                    m_controls[idx] = ctrl;
                    ++m_size;
                    ++m_statistics.m_num_probes;

                    return idx;
                }
            }

            // Else probe further
            i = (i + 16) & mask;
            m_statistics.m_sum_probe_lengths += 16;
        }
    }

    const Key& operator[](Index pos) const { return m_slots[pos]; }

    size_t size() const { return m_size; }
    size_t capacity() const { return m_capacity; }
    const Statistics& statistics() const { return m_statistics; }
};

/// @brief `TreeRehashPolicy` implements a DFS style rehash policy for chains of perfectly balanced binary trees.
class TreeRehashPolicy
{
private:
    IndexedHashSet<Index>& m_roots;

    struct RehashData
    {
        size_t capacity;
        std::vector<Slot<Index>> slots;
        std::vector<ctrl_t> controls;

        explicit RehashData(size_t capacity) : capacity(capacity), slots(capacity), controls()
        {
            // Sentinel-padded rolling buffer
            controls.reserve(capacity + 15);
            controls.resize(capacity, ctrl_t::kEmpty);
            controls.resize(capacity + 15, ctrl_t::kSentinel);
        }
    };

    template<typename HashIDMapType>
    Index insert(Slot<Index> slot, const HashIDMapType& map, RehashData& tmp)
    {
        size_t h = map.m_hash(slot);
        size_t mask = (tmp.capacity - 1);
        size_t i = h & mask;
        ctrl_t ctrl = static_cast<ctrl_t>(h >> 57);
        assert(static_cast<int8_t>(ctrl) >= 0);

        while (true)
        {
            assert(i < tmp.capacity);
            assert(i + 15 < tmp.controls.size());

            // Load 16 control bytes
            __m128i ctrl_block = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&tmp.controls[i]));
            __m128i broadcast_ctrl = _mm_set1_epi8(static_cast<signed char>(ctrl));

            // Compare against ctrl byte
            __m128i cmp_ctrl = _mm_cmpeq_epi8(ctrl_block, broadcast_ctrl);
            int mask_ctrl = _mm_movemask_epi8(cmp_ctrl);

            // Check if slot exists
            while (mask_ctrl != 0)
            {
                int offset = __builtin_ctz(mask_ctrl);
                size_t idx = (i + offset) & mask;

                if (map.m_equal_to(tmp.slots[idx], slot))
                {
                    map.m_statistics.m_sum_probe_lengths += offset;
                    return idx;
                }

                mask_ctrl &= mask_ctrl - 1;  // Clear the lowest set bit
            }

            // Compare against kEmpty
            __m128i cmp_empty = _mm_cmpeq_epi8(ctrl_block, kEmptyPattern);
            int mask_empty = _mm_movemask_epi8(cmp_empty);

            // Second: insert into first empty slot if found
            if (mask_empty != 0)
            {
                int offset = __builtin_ctz(mask_empty);
                map.m_statistics.m_sum_probe_lengths += offset;
                size_t idx = (i + offset) & mask;

                if (tmp.controls[idx] == ctrl_t::kEmpty)
                {
                    assert(tmp.controls[idx] == ctrl_t::kEmpty && "Unexpected overwrite!");

                    tmp.slots[idx] = slot;
                    tmp.controls[idx] = ctrl;
                    ++map.m_statistics.m_num_probes;
                    return idx;
                }
            }

            // Else probe further
            i = (i + 16) & mask;
            map.m_statistics.m_sum_probe_lengths += 16;
        }
    }

    template<typename HashIDMapType>
    Index rehash_recursively(Index unstable_index, size_t size, const HashIDMapType& map, RehashData& tmp)
    {
        /* Base case 1: skipped node creation */
        if (size == 1)
            return unstable_index;

        /* Note: caching relocation is expensive to cache because the tree structure depends on size. */

        const auto& slot = map.m_slots[unstable_index];

        /* Base case 3: rellocate slot */
        if (size == 2)
            return insert(slot, map, tmp);

        /* Divide */
        assert(size >= 2);
        const auto mid = std::bit_floor(size - 1);

        /* Conquer */
        Index i1 = rehash_recursively(slot.i1, mid, tmp);
        Index i2 = rehash_recursively(slot.i2, size - mid, tmp);

        return insert(Slot<Index>(i1, i2), map, tmp);
    }

public:
    explicit TreeRehashPolicy(IndexedHashSet<Index>& roots) : m_roots(roots) {}

    template<typename HashIDMapType>
    void operator()(HashIDMapType& map, size_t new_capacity) const
    {
        auto tmp = RehashData(new_capacity);

        // Relocate trees underlying the stable indices
        m_roots.m_uniqueness.clear();

        // Skip the empty root.
        for (Index stable_index = 1; stable_index < m_roots.m_index_to_slot.size(); ++stable_index)
        {
            Slot<Index> root = m_roots.m_index_to_slot[stable_index];

            assert(root.i2 > 0);  // Ensure nonempty.

            Index unstable_index = rehash_recursively(root.i1, root.i2, map, tmp);

            m_roots.m_index_to_slot[stable_index] = Slot<Index>(unstable_index, root.i2);
            m_roots.m_uniqueness.emplace(stable_index);
        }

        map.m_capacity = new_capacity;
        std::swap(map.m_slots, tmp.slots);
        std::swap(map.m_controls, tmp.controls);
    }
};
}

#endif