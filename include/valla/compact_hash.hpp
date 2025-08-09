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

#ifndef VALLA_INCLUDE_COMPACT_HASH_HPP_
#define VALLA_INCLUDE_COMPACT_HASH_HPP_

#include "valla/slot.hpp"
#include "valla/uint64tcoder.hpp"

#include <array>
#include <cassert>
#include <concepts>
#include <cstdint>
#include <limits>
#include <utility>

namespace valla
{

/**
 * Bijective hash function h : [w] -> [w] where [w] represents the set {0,...,2^w-1}.
 */

// Precompute masks for w = 0..64
template<std::unsigned_integral T>
consteval auto make_masks()
{
    constexpr uint8_t Bits = std::numeric_limits<T>::digits;

    std::array<T, Bits + 1> m {};
    m[0] = T { 0 };
    for (int w = 1; w < Bits; ++w)
    {
        m[w] = (T(1) << w) - T(1);
    }
    m[Bits] = ~T { 0 };
    return m;
}

template<std::unsigned_integral T>
inline constexpr auto MASKS = make_masks<T>();

template<std::unsigned_integral T>
constexpr T mask(uint8_t w)
{
    [[maybe_unused]] constexpr uint8_t Bits = std::numeric_limits<T>::digits;
    assert(w >= 1 && w <= Bits);
    return MASKS<T>[w];
}

// ---------------- invertible ops on w bits ----------------
template<std::unsigned_integral T>
constexpr T inv_mod_2pow(T a)
{
    // Newton iteration for inverse mod 2^w (a must be odd)
    T x = T { 1 };
    for (int i = 0; i < 6; ++i)
        x *= T(2) - a * x;
    return x;
}

template<std::unsigned_integral T>
constexpr T mul_b(T x, T c, uint8_t w)
{
    return (x * c) & mask<T>(w);
}

template<std::unsigned_integral T>
constexpr T mul_inv_b(T x, T c, uint8_t w)
{
    const T ci = inv_mod_2pow(c) & mask<T>(w);
    return (x * ci) & mask<T>(w);
}

// Invertible right-xor-shift limited to w bits:
// y = x ^ (x >> s);  inverse via expanding dependencies.
template<std::unsigned_integral T>
constexpr T xorshr(T x, uint8_t s, uint8_t w)
{
    return (x ^ (x >> s)) & mask<T>(w);
}
template<std::unsigned_integral T>
constexpr T inv_xorshr(T x, uint8_t s, uint8_t w)
{
    const T m = mask<T>(w);
    // apply with doubling window until covering w bits
    for (uint8_t k = s; k < w; k <<= 1)
        x ^= (x >> k) & m;
    return x & m;
}

// Invertible left-xor-shift limited to w bits:
// y = x ^ ((x << s) & m); inverse by doubling window.
template<std::unsigned_integral T>
constexpr T xorshl(T x, uint8_t s, uint8_t w)
{
    const T m = mask<T>(w);
    return (x ^ ((x << s) & m)) & m;
}
template<std::unsigned_integral T>
constexpr T inv_xorshl(T x, uint8_t s, uint8_t w)
{
    const T m = mask<T>(w);
    for (uint8_t k = s; k < w; k <<= 1)
        x ^= ((x << k) & m);
    return x & m;
}

// ---------------- bijective mixer ----------------
// Two odd multipliers with good bit diffusion (SplitMix64 constants)
template<std::unsigned_integral T>
constexpr T C1 = T { 0x9E3779B97F4A7C15ull };  // odd
template<std::unsigned_integral T>
constexpr T C2 = T { 0xBF58476D1CE4E5B9ull };  // odd

template<IsUint64tCodable T>
struct CompactHash
{
    // choose shifts relative to w so it works well for small w too
    static constexpr uint8_t s1(uint8_t w) { return w >= 6 ? (uint8_t) (w / 2) : 1; }
    static constexpr uint8_t s2(uint8_t w) { return w >= 7 ? (uint8_t) (w / 3 + 1) : 1; }
    static constexpr uint8_t s3(uint8_t w) { return w >= 8 ? (uint8_t) (w / 2 + 1) : 1; }

    T hash(T x, uint8_t w) const
    {
        x &= mask<T>(w);
        x = xorshr(x, s1(w), w);
        x = mul_b(x, C1<T>, w);
        x = xorshl(x, s2(w), w);
        x = mul_b(x, C2<T>, w);
        x = xorshr(x, s3(w), w);
        return x & mask<T>(w);
    }

    T invert_hash(T h, uint8_t w) const
    {
        h &= mask<T>(w);
        h = inv_xorshr(h, s3(w), w);
        h = mul_inv_b(h, C2<T>, w);
        h = inv_xorshl(h, s2(w), w);
        h = mul_inv_b(h, C1<T>, w);
        h = inv_xorshr(h, s1(w), w);
        return h & mask<T>(w);
    }
};

}

#endif