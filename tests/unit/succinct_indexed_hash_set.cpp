/*
 * Copyright (C) 2023 Dominik Drexler
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

#include <gtest/gtest.h>
#include <valla/succinct_indexed_hash_set.hpp>

namespace valla::tests
{
TEST(VallaTests, SuccinctIndexedHashSetTest)
{
    auto set = SuccinctIndexedHashSet<Slot<uint32_t>, uint32_t> {};

    const auto s0 = set.insert(Slot<uint32_t>(0, 1));
    EXPECT_EQ(s0, 0);
    EXPECT_EQ(set.size(), 1);

    const auto s1 = set.insert(Slot<uint32_t>(4, 5));
    EXPECT_EQ(s1, 1);
    EXPECT_EQ(set.size(), 2);

    const auto s2 = set.insert(Slot<uint32_t>(0, 1));
    EXPECT_EQ(s2, 0);
    EXPECT_EQ(set.size(), 2);
}
}
