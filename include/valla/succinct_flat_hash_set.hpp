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
template<IsUint64tCodable T, typename Hash = Hash<T>, typename EqualTo = EqualTo<T>, size_t InitialCapacity = 1024>
class succinct_flat_hash_set
{
private:
    static_assert(InitialCapacity % 2 == 0, "InitialCapacity must be a multiple of 2.");
    static_assert(InitialCapacity % absl::container_internal::Group::kWidth == 0, "InitialCapacity must be a multiple of group width.");
    static_assert(InitialCapacity >= MIN_RAW_CAPACITY, "InitialCapacity must be greater than minumum raw capacity.");

    sdsl::int_vector<> m_slots;
    std::vector<absl::container_internal::ctrl_t> m_controls;

public:
};
}

#endif
