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
#include <vector>

namespace valla
{
/// @brief `HashIDMap implements a hash ID map with open addressing in a Swiss table format where the position of a key implicitly becomes the index.
/// @tparam Derived is the derived class that must implement the rehash logic in rehash_impl.
/// @tparam Key is the key.
/// @tparam Hash is the hash functor for a key.
/// @tparam EqualTo is the equality comparison functor for a key.
/// @tparam InitialCapacity is the initial capacity.
template<typename Derived,
         typename Key,
         std::unsigned_integral I,
         typename Hash = Hash<Key>,
         typename EqualTo = std::equal_to<Key>,
         size_t InitialCapacity = 127>
class HashIDMap
{
private:
    static_assert(((InitialCapacity + 1) & InitialCapacity) == 0, "InitialCapacity must be 2^{InitialCapacity}-1.");
    static_assert(InitialCapacity >= 127, "InitialCapacity must be greater than 127.");

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

protected:
    std::vector<Key> m_slots;
    std::vector<absl::container_internal::ctrl_t> m_controls;

    size_t m_size;
    size_t m_capacity;

    Hash m_hash;
    EqualTo m_equal_to;

    struct HashSetStatistics
    {
        size_t m_num_rehashes = 0;
        std::chrono::milliseconds m_total_rehash_time = std::chrono::milliseconds::zero();
        size_t m_num_probes = 0;
        size_t m_sum_probe_lengths = 0;
    };

    HashSetStatistics m_statistics;

public:
    HashIDMap() : m_slots(), m_controls(), m_size(0), m_capacity(InitialCapacity), m_hash(), m_equal_to()
    {
        m_slots.resize(InitialCapacity);

        // Sentinel-padded rolling buffer
        m_controls.reserve(InitialCapacity + absl::container_internal::Group::kWidth);
        m_controls.resize(InitialCapacity, absl::container_internal::ctrl_t::kEmpty);
        m_controls.resize(InitialCapacity + absl::container_internal::Group::kWidth, absl::container_internal::ctrl_t::kSentinel);
    }

    bool has_capacity_for(size_t amount) const { return (static_cast<double>(size() + amount) / capacity()) <= MAX_LOAD_FACTOR; }

    I insert(const Key& slot)
    {
        assert(size() < capacity() && "Insert failed. Rehashing to higher capacity is required.");

        size_t h = m_hash(slot);
        absl::container_internal::h2_t h2 = h >> 57;

        absl::container_internal::probe_seq<absl::container_internal::Group::kWidth> probe(h, capacity());

        while (true)
        {
            absl::container_internal::Group group(&m_controls[probe.offset()]);

            for (const auto i : group.Match(h2))
            {
                m_statistics.m_sum_probe_lengths += i;

                size_t idx = probe.offset(i);
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
                ++m_size;
                ++m_statistics.m_num_probes;

                return idx;
            }

            probe.next();
            m_statistics.m_sum_probe_lengths += absl::container_internal::Group::kWidth;
        }
    }

    const Key& operator[](I pos) const { return m_slots[pos]; }

    size_t size() const { return m_size; }
    size_t capacity() const { return m_capacity; }
    double load_factor() const { return static_cast<double>(size()) / capacity(); }
    constexpr double max_load_factor() const { return MAX_LOAD_FACTOR; }
    const HashSetStatistics& statistics() const { return m_statistics; }
};

/// @brief `TreeHashIDMap` implements a HashIDMap for chains of perfectly balanced binary trees with DFS style rehash policy.
template<std::unsigned_integral I, typename Hash = Hash<Slot<I>>, typename EqualTo = std::equal_to<Slot<I>>, size_t InitialCapacity = 127>
class TreeHashIDMap : public HashIDMap<TreeHashIDMap<I, Hash, EqualTo, InitialCapacity>, Slot<I>, I, Hash, EqualTo, InitialCapacity>
{
public:
    using value_type = Slot<I>;
    using index_type = I;

    static constexpr bool is_stable = false;

private:
    IndexedHashSet<Slot<I>, I> m_roots;  ///< Dynamic hash ID maps require stable mapping for root nodes.
    std::vector<bool> m_stable_leaves;

    struct RehashData
    {
        std::vector<Slot<I>> slots;
        std::vector<absl::container_internal::ctrl_t> controls;
        size_t size;
        size_t capacity;

        explicit RehashData(size_t capacity) : slots(capacity), controls(), size(0), capacity(capacity)
        {
            // Sentinel-padded rolling buffer
            controls.reserve(capacity + absl::container_internal::Group::kWidth);
            controls.resize(capacity, absl::container_internal::ctrl_t::kEmpty);
            controls.resize(capacity + absl::container_internal::Group::kWidth, absl::container_internal::ctrl_t::kSentinel);
        }

        bool has_capacity_for(size_t amount) const { return (static_cast<double>(size + amount) / capacity) <= MAX_LOAD_FACTOR; }
    };

    I insert(const Slot<I>& slot, RehashData& tmp)
    {
        assert(tmp.size < tmp.capacity && "Insert failed. Rehashing to higher capacity is required.");

        size_t h = this->m_hash(slot);
        absl::container_internal::h2_t h2 = h >> 57;

        absl::container_internal::probe_seq<absl::container_internal::Group::kWidth> probe(h, tmp.capacity);

        while (true)
        {
            absl::container_internal::Group group(&tmp.controls[probe.offset()]);

            for (const auto i : group.Match(h2))
            {
                this->m_statistics.m_sum_probe_lengths += i;

                size_t idx = probe.offset(i);
                assert(is_within_bounds(tmp.slots, idx));

                if (this->m_equal_to(tmp.slots[idx], slot))
                    return idx;
            }

            auto mask_empty = group.MaskEmpty();
            if (mask_empty)
            {
                int offset = mask_empty.LowestBitSet();
                this->m_statistics.m_sum_probe_lengths += offset;

                size_t idx = probe.offset() + offset;
                assert(is_within_bounds(tmp.slots, idx));

                tmp.slots[idx] = slot;
                tmp.controls[idx] = static_cast<absl::container_internal::ctrl_t>(h2);
                ++tmp.size;
                ++this->m_statistics.m_num_probes;

                return idx;
            }

            probe.next();
            this->m_statistics.m_sum_probe_lengths += absl::container_internal::Group::kWidth;
        }
    }

    I rehash_recursively(I unstable_index, I size, RehashData& tmp)
    {
        /* Base case 1: skipped node creation */
        if (size == 1)
            return unstable_index;

        /* Note: caching relocation is expensive to cache because the tree structure depends on size. */

        assert(is_within_bounds(this->m_slots, unstable_index));
        const auto& slot = this->m_slots[unstable_index];

        /* Divide */
        assert(size >= 2);
        const auto mid = std::bit_floor(size - 1);

        /* Conquer */
        I i1 = rehash_recursively(slot.i1, mid, tmp);
        I i2 = rehash_recursively(slot.i2, size - mid, tmp);

        return insert(Slot<I>(i1, i2), tmp);
    }

    /// @brief Depth-first rehash policy for a HashIDMap that stores a collection of perfectly balanced binary trees.
    /// @param new_capacity is the capacity after rehash.
    bool rehash_impl(size_t new_capacity)
    {
        auto tmp = RehashData(new_capacity);

        /* Relocate trees underlying the stable indices */
        m_roots.m_uniqueness.clear();

        auto backup_unstable_indices = std::vector<I> { 0 };

        // Relocate remaining roots.
        for (I stable_index = 1; stable_index < this->m_roots.size(); ++stable_index)
        {
            Slot root = this->m_roots.m_slots[stable_index];
            assert(root.i2 > 0);  // Ensure nonempty.

            backup_unstable_indices.push_back(root.i1);

            if (!tmp.has_capacity_for(root.i2))
            {
                /* Rollback rehash */
                for (I stable_index_2 = 1; stable_index_2 < backup_unstable_indices.size(); ++stable_index_2)
                {
                    this->m_roots.m_slots[stable_index].i1 = backup_unstable_indices[stable_index_2];
                }
                m_roots.m_uniqueness.clear();
                for (I stable_index_2 = 1; stable_index_2 < this->m_roots.size(); ++stable_index_2)
                {
                    this->m_roots.m_uniqueness.emplace(stable_index);
                }
                return false;
            }

            I unstable_index = rehash_recursively(root.i1, root.i2, tmp);

            this->m_roots.m_slots[stable_index] = Slot(unstable_index, root.i2);
            this->m_roots.m_uniqueness.emplace(stable_index);
        }

        if (tmp.size > new_capacity)
            throw std::runtime_error("Encountered insufficient capacity during rehash due to changed structural sharing.");

        std::swap(this->m_slots, tmp.slots);
        std::swap(this->m_controls, tmp.controls);
        this->m_size = tmp.size;
        this->m_capacity = tmp.capacity;

        return true;
    }

    using Base = HashIDMap<TreeHashIDMap<I, Hash, EqualTo, InitialCapacity>, Slot<I>, I, Hash, EqualTo, InitialCapacity>;

    friend Base;

public:
    explicit TreeHashIDMap() : Base(), m_roots()
    {
        this->m_roots.insert(get_empty_slot<I>());  // root representing empty sequence
        this->m_stable_leaves.push_back(false);
    }

    void rehash()
    {
        using clock = std::chrono::high_resolution_clock;

        auto start = clock::now();  // Start timing

        ++this->m_statistics.m_num_rehashes;

        size_t new_capacity = this->capacity();

        while (true)
        {
            new_capacity = (new_capacity << 1) | 1;
            assert(absl::container_internal::IsValidCapacity(new_capacity));

            if (rehash_impl(new_capacity))
                break;
        }

        auto end = clock::now();
        this->m_statistics.m_total_rehash_time += std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    }

    I insert_root(const Slot<I>& slot) { return m_roots.insert(slot); }

    I insert_internal(const Slot<I>& slot) { return Base::insert(slot); }

    const Slot<I>& lookup_root(I pos) const { return m_roots[pos]; }

    const Slot<I>& lookup_internal(I pos) const { return this->m_slots[pos]; }

    size_t num_internals() const { return Base::size(); }
    size_t num_roots() const { return m_roots.size(); }
    size_t num_slots() const { return num_internals() + num_roots(); }

    size_t mem_usage() const
    {
        size_t usage = 0;
        usage += m_roots.mem_usage();
        usage += this->m_slots.capacity() * sizeof(Slot<I>);
        usage += this->m_controls.capacity() * sizeof(absl::container_internal::ctrl_t);
        return usage;
    }

    friend std::ostream& operator<<(std::ostream& os, const TreeHashIDMap& el)
    {
        os << el.m_slots << std::endl;
        return os;
    }
};
}

#endif
