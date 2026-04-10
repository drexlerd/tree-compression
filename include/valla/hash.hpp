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

#include <concepts>
#include <gtl/phmap.hpp>
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
    size_t operator()(const Slot<I>& el) const { return gtl::HashState().combine(el.i1, el.i2); }
};

template<std::floating_point T>
struct Hash<T>
{
    size_t operator()(const T& el) const
    {
        if (std::isnan(el))
            return 0x9e3779b97f4a7c15ULL;  // any fixed salt

        return std::hash<T> {}(el);
    }
};

}

#endif