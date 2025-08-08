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

#ifndef VALLA_INCLUDE_HASH_HPP_
#define VALLA_INCLUDE_HASH_HPP_

#include "valla/slot.hpp"
#include "valla/uint64tcoder.hpp"

#include <absl/hash/hash.h>
#include <concepts>
#include <utility>

namespace valla
{

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
 * Bijective hash function h : [b] -> [b] where [b] represents the set {0,...,2^b-1}.
 */

// Precompute masks for b = 0..64
constexpr std::array<uint64_t, 65> MASKS = []
{
    std::array<uint64_t, 65> m {};
    m[0] = 0;  // unused in our API, but defined
    for (int b = 1; b < 64; ++b)
        m[b] = (1ULL << b) - 1;
    m[64] = ~0ULL;
    return m;
}();

constexpr uint64_t mask(uint8_t b)
{
    assert(b >= 1 && b <= 64);
    return MASKS[b];
}

constexpr uint64_t CONSTANT = 0x9E3779B97F4A7C15ULL;

static_assert((CONSTANT & 1ULL) == 1ULL, "CONSTANT must be odd");

// constexpr 6-step inverse modulo 2^64
constexpr uint64_t inv_mod_2pow64(uint64_t a)
{
    uint64_t x = 1;
    x *= 2 - a * x;
    x *= 2 - a * x;
    x *= 2 - a * x;
    x *= 2 - a * x;
    x *= 2 - a * x;
    x *= 2 - a * x;
    return x;
}
constexpr uint64_t CONSTANT_INV = inv_mod_2pow64(CONSTANT);

static_assert(uint64_t(CONSTANT * CONSTANT_INV) == 1ULL, "CONSTANT_INV must satisfy CONSTANT*CONSTANT_INV == 1 mod 2^64");

/// @brief Masked LARGE_ODD_CONSTANT_INVERSE
constexpr std::array<uint64_t, 65> MASKS_INV = []
{
    std::array<uint64_t, 65> a {};
    for (int b = 0; b <= 64; ++b)
        a[b] = CONSTANT_INV & MASKS[b];
    return a;
}();

constexpr uint64_t mask_inv(uint8_t b)
{
    assert(b >= 1 && b <= 64);
    return MASKS_INV[b];
}

inline uint64_t hash(uint64_t x, uint8_t b) { return (x * CONSTANT) & mask(b); }

inline uint64_t inverse_hash(uint64_t h, uint8_t b) { return (h * mask_inv(b)) & mask(b); }

}

#endif