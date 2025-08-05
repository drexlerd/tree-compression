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

#ifndef VALLA_INCLUDE_STATISTICS_HPP_
#define VALLA_INCLUDE_STATISTICS_HPP_

#include <chrono>
#include <cstddef>

namespace valla
{

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

}

#endif