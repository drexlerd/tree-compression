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

// Some ChaptGpt generated hash from the Dynamic path decomposed tries paper text. have to finalize it.
template<std::unsigned_integral U = uint64_t>
struct BijectiveTransform
{
    // parameters: bit-width z in [1, digits(U)], shift a > floor(z/2), odd multiplier p
    const uint8_t z;
    const uint8_t a;
    const U p;
    const U mask;   // (1<<z)-1 (or all-ones if z==digits)
    const U p_inv;  // modular inverse of p modulo 2^z

    // ---- helpers ----
    static constexpr U make_mask(uint8_t zbits)
    {
        const uint8_t D = std::numeric_limits<U>::digits;
        assert(zbits >= 1 && zbits <= D);
        if (zbits == D)
            return ~U { 0 };
        return (U { 1 } << zbits) - U { 1 };
    }

    // multiplicative inverse modulo 2^z (p must be odd). Newton iteration; evaluated at compile time here.
    static constexpr U inv_mod_2pow(U a, uint8_t zbits)
    {
        assert((a & 1u) == 1u);
        U x = U { 1 };
        // Each iteration doubles the number of correct low bits. 6 suffices for 64-bit.
        for (int i = 0; i < 6; ++i)
        {
            x = x * (U { 2 } - a * x);
        }
        // keep only zbits
        const U m = make_mask(zbits);
        return x & m;
    }

    // constructor checks & precomputes
    constexpr BijectiveTransform(uint8_t zbits, uint8_t ashift, U mult) : z(zbits), a(ashift), p(mult), mask(make_mask(z)), p_inv(inv_mod_2pow(mult, z))
    {
        const uint8_t D = std::numeric_limits<U>::digits;
        assert(z >= 1 && z <= D);
        assert(a > (z >> 1));    // a > floor(z/2)  => h1 is self-inverse
        assert((p & 1u) == 1u);  // p must be odd to be invertible mod 2^z
    }

    // h1: x ^ (x >> a), masked to z bits
    static constexpr U h1(U x, uint8_t a, U m) { return (x ^ (x >> a)) & m; }

    // h2: x * p mod 2^z
    static constexpr U h2(U x, U p, U m) { return (x * p) & m; }

    // The hash: h = h1 ∘ h2
    constexpr U hash(U x) const
    {
        x &= mask;
        x = h2(x, p, mask);
        x = h1(x, a, mask);
        return x;
    }

    // Inverse: h^{-1} = h2^{-1} ∘ h1^{-1};  h1 is self-inverse when a > z/2
    constexpr U invert(U y) const
    {
        y &= mask;
        y = h1(y, a, mask);      // self-inverse under our 'a' constraint
        y = h2(y, p_inv, mask);  // multiply by modular inverse
        return y;
    }
};

}

#endif