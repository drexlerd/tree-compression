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

#ifndef VALLA_INCLUDE_DECLARATIONS_HPP_
#define VALLA_INCLUDE_DECLARATIONS_HPP_

#include "valla/config.hpp"
//
#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/container/node_hash_map.h>
#include <absl/container/node_hash_set.h>
#include <cassert>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <ostream>
#include <ranges>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace valla
{

template<typename Container>
inline bool is_within_bounds(const Container& container, size_t index)
{
    return index < container.size();
}

/**
 * Slot
 */

template<std::unsigned_integral I>
struct Slot
{
    I i1;
    I i2;

    constexpr Slot() : i1(0), i2(0) {}
    constexpr Slot(I i1, I i2) : i1(i1), i2(i2) {}

    constexpr friend bool operator==(const Slot& lhs, const Slot& rhs) { return lhs.i1 == rhs.i1 && lhs.i2 == rhs.i2; }

    friend std::ostream& operator<<(std::ostream& os, const Slot& slot)
    {
        os << "<" << slot.i1 << ", " << slot.i2 << ">";
        return os;
    }
};

template<std::unsigned_integral I>
constexpr inline Slot<I> get_empty_slot()
{
    return Slot<I>();
}

/**
 * Uint64tCoder
 */

template<typename T>
struct Uint64tCoder
{
};

template<std::unsigned_integral I>
struct Uint64tCoder<Slot<I>>
{
    constexpr static uint8_t bit_width(const Slot<I>& el) { return std::max(1, 2 * std::max(std::bit_width(el.i1), std::bit_width(el.i2))); }

    constexpr static uint64_t to_uint64_t(const Slot<I>& el, uint8_t bit_width)
    {
        uint8_t half = bit_width / 2;
        return (uint64_t(el.i1) << half) | el.i2;
    }

    constexpr static Slot<I> from_uint64_t(uint64_t packed, uint8_t bit_width)
    {
        uint8_t half = bit_width / 2;
        uint64_t mask = (uint64_t(1) << half) - 1;
        I i2 = static_cast<I>(packed & mask);
        I i1 = static_cast<I>(packed >> half);
        return Slot(i1, i2);
    }
};

template<std::unsigned_integral T>
struct Uint64tCoder<T>
{
    constexpr static uint8_t bit_width(const T& el) { return std::max(1, std::bit_width(el)); }

    constexpr static uint64_t to_uint64_t(const T& el, uint8_t) { return static_cast<uint64_t>(el); }

    constexpr static T from_uint64_t(uint64_t packed, uint8_t) { return static_cast<T>(packed); }
};

/**
 * Concepts
 */

template<typename T>
concept IsUint64tCodable = requires(T a, uint64_t p, uint8_t b) {
    requires std::is_standard_layout_v<T>;
    requires(sizeof(T) <= sizeof(uint64_t));

    { Uint64tCoder<T>::bit_width(a) } -> std::same_as<uint8_t>;
    { Uint64tCoder<T>::to_uint64_t(a, b) } -> std::same_as<uint64_t>;
    { Uint64tCoder<T>::from_uint64_t(p, b) } -> std::same_as<T>;
};

static_assert(IsUint64tCodable<uint16_t>);
static_assert(IsUint64tCodable<uint32_t>);
static_assert(IsUint64tCodable<uint64_t>);

static_assert(IsUint64tCodable<Slot<uint16_t>>);
static_assert(IsUint64tCodable<Slot<uint32_t>>);
static_assert(!IsUint64tCodable<Slot<uint64_t>>);

template<typename Set>
concept IsStable = Set::is_stable;

template<typename T>
concept IsIndexedHashSet = requires(T a, typename T::value_type v, typename T::index_type i) {
    typename T::value_type;
    typename T::index_type;

    requires std::unsigned_integral<typename T::index_type>;

    { a.insert(v) } -> std::same_as<typename T::index_type>;
    { a[i] } -> std::convertible_to<typename T::value_type>;
};

template<typename Set1, typename Set2, typename V = typename Set2::value_type>
concept AreCompatibleIndexedHashSets = IsIndexedHashSet<Set1>                                                 //
                                       && IsIndexedHashSet<Set2>                                              //
                                       && std::same_as<V, typename Set2::value_type>                          //
                                       && std::same_as<typename Set1::index_type, typename Set2::index_type>  //
                                       && std::same_as<typename Set1::value_type, Slot<typename Set1::index_type>>;

/**
 * Iterator
 */

template<std::unsigned_integral I>
struct Entry
{
    I m_index;
    I m_size;

    Entry(I index, I size) : m_index(index), m_size(size) {}
};

template<typename T>
inline void copy_object(const std::vector<T>& src, std::vector<T>& dst)
{
    dst.clear();
    dst.insert(dst.end(), src.begin(), src.end());
}

/**
 * Printing
 */

template<typename T>
inline std::ostream& operator<<(std::ostream& out, const std::vector<T>& vec)
{
    out << "[";
    for (const auto x : vec)
    {
        out << x << ", ";
    }
    out << "]";

    return out;
}

inline std::ostream& operator<<(std::ostream& out, const std::vector<uint8_t>& vec)
{
    out << "[";
    for (const auto x : vec)
    {
        out << static_cast<uint32_t>(x) << ", ";
    }
    out << "]";

    return out;
}

/**
 * Statistics
 */

struct HashSetStatistics
{
    size_t m_num_rehashes = 0;
    std::chrono::milliseconds m_total_rehash_time = std::chrono::milliseconds::zero();
    size_t m_num_probes = 0;
    size_t m_sum_probe_lengths = 0;

    HashSetStatistics& operator+=(const HashSetStatistics& rhs)
    {
        m_num_rehashes += rhs.m_num_rehashes;
        m_total_rehash_time += rhs.m_total_rehash_time;
        m_num_probes += rhs.m_num_probes;
        m_sum_probe_lengths += rhs.m_sum_probe_lengths;
        return *this;
    }
};

/**
 * Hashing
 */

template<typename T>
struct Hash
{
    size_t operator()(const T& el) const { return std::hash<T> {}(el); }
};

template<std::unsigned_integral I>
struct Hash<Slot<I>>
{
    size_t operator()(const Slot<I>& el) const { return absl::HashOf(el.i1, el.i2); }

    template<IsUint64tCodable U = Slot<I>>
    bool operator()(uint64_t el) const
    {
        return Hash<uint64_t> {}(el);
    }
};

/**
 * EqualTo
 */

template<typename T>
struct EqualTo
{
    bool operator()(const T& lhs, const T& rhs) const { return lhs == rhs; }

    template<IsUint64tCodable U = T>
    bool operator()(uint64_t lhs, uint64_t rhs) const
    {
        return lhs == rhs;
    }
};

}

#endif