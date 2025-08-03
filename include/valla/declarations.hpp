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

    constexpr uint8_t bit_width() const { return 2 * std::max(std::bit_width(i1), std::bit_width(i2)); }

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
 * EqualTo
 */

template<typename T>
struct EqualTo
{
    bool operator()(const T& lhs, const T& rhs) const { return lhs == rhs; }
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
};

template<typename T, std::unsigned_integral I, typename Hash = Hash<T>>
struct IndexReferencedHash
{
    const std::vector<T>& vec;
    Hash hash;

    IndexReferencedHash(const std::vector<T>& vec) : vec(vec), hash() {}

    size_t operator()(I el) const
    {
        assert(el < vec.size());
        return hash(vec[el]);
    }
};

template<typename T, std::unsigned_integral I, typename EqualTo = EqualTo<T>>
struct IndexReferencedEqualTo
{
    const std::vector<T>& vec;
    EqualTo equal_to;

    IndexReferencedEqualTo(const std::vector<T>& vec) : vec(vec), equal_to() {}

    size_t operator()(I lhs, I rhs) const
    {
        assert(lhs < vec.size());
        assert(rhs < vec.size());
        return equal_to(vec[lhs], vec[rhs]);
    }
};

template<typename T>
concept IsUint64tCodable = requires(T a, uint64_t v) {
    requires std::is_standard_layout_v<T>;
    requires(sizeof(T) < sizeof(uint64_t));

    { a.bit_width() } -> std::same_as<uint8_t>;
    { a.to_uint64_t() } -> std::same_as<uint64_t>;
    { T::from_uint64_t(v) } -> std::same_as<T>;
};
}

#endif