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

#ifndef VALLA_INCLUDE_RAW_HASH_SET_HPP_
#define VALLA_INCLUDE_RAW_HASH_SET_HPP_

#include "../config.hpp"

#include <cstdint>
#include <ostream>
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

inline std::ostream& operator<<(std::ostream& out, const std::vector<ctrl_t>& vec)
{
    out << "[";
    for (const auto x : vec)
    {
        out << static_cast<int32_t>(x) << ", ";
    }
    out << "]";

    return out;
}

alignas(16) inline static const __m128i kEmptyPattern = _mm_set1_epi8(static_cast<signed char>(ctrl_t::kEmpty));

// --------------------------------------------
// SIMD Version
#if VALLA_HAVE_SSE2
struct ControlMatcherSse2
{
    static inline int match_ctrl(const ctrl_t* controls, ctrl_t ctrl)
    {
        __m128i ctrl_block = _mm_loadu_si128(reinterpret_cast<const __m128i*>(controls));
        __m128i broadcast = _mm_set1_epi8(static_cast<signed char>(ctrl));
        __m128i cmp = _mm_cmpeq_epi8(ctrl_block, broadcast);
        return _mm_movemask_epi8(cmp);
    }

    static inline int match_empty(const ctrl_t* controls)
    {
        __m128i ctrl_block = _mm_loadu_si128(reinterpret_cast<const __m128i*>(controls));
        __m128i empty = _mm_cmpeq_epi8(ctrl_block, kEmptyPattern);
        return _mm_movemask_epi8(empty);
    }
};

struct GroupSse2
{
    static constexpr size_t kWidth = 16;  // the number of slots per group
};
#endif

// --------------------------------------------
// Portable fallback
struct ControlMatcherPortable
{
    static inline int match_ctrl(const ctrl_t* controls, ctrl_t ctrl)
    {
        int result = 0;
        for (int j = 0; j < 16; ++j)
            result |= (controls[j] == ctrl) << j;
        return result;
    }

    static inline int match_empty(const ctrl_t* controls)
    {
        int result = 0;
        for (int j = 0; j < 16; ++j)
            result |= (controls[j] == ctrl_t::kEmpty) << j;
        return result;
    }
};

struct GroupPortable
{
    static constexpr size_t kWidth = 8;  // the number of slots per group
};

#if VALLA_HAVE_SSE2
using ProbeImpl = ControlMatcherSse2;
#else
using ProbeImpl = ControlMatcherPortable;
#endif

}

#endif
