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
#include <valla/valla.hpp>

namespace valla::tests
{
TEST(VallaTests, SuccinctIndexedHashSetTest)
{
    auto set = SuccinctIndexedHashSet<Slot<uint32_t>, uint32_t> {};

    // Insert s0
    const auto s0 = set.insert(Slot<uint32_t>(0, 1));
    EXPECT_EQ(s0, 0);
    EXPECT_EQ(set.size(), 1);
    EXPECT_EQ(set.width(), 2);
    EXPECT_EQ(set.uniqueness().width(), 1);
    EXPECT_EQ(set.slots().bit_size(), 2);
    EXPECT_EQ(set.slots().capacity(), 64);
    EXPECT_EQ(set.lookup(s0), Slot<uint32_t>(0, 1));

    // Insert s1 (bit resize from 2 to 6)
    const auto s1 = set.insert(Slot<uint32_t>(4, 5));
    EXPECT_EQ(s1, 1);
    EXPECT_EQ(set.size(), 2);
    EXPECT_EQ(set.width(), 6);
    EXPECT_EQ(set.uniqueness().width(), 1);
    EXPECT_EQ(set.slots().bit_size(), 12);
    EXPECT_EQ(set.slots().capacity(), 64);
    EXPECT_EQ(set.lookup(s1), Slot<uint32_t>(4, 5));

    // Insert s2 (same as s0)
    const auto s2 = set.insert(Slot<uint32_t>(0, 1));
    EXPECT_EQ(s2, 0);
    EXPECT_EQ(set.size(), 2);
    EXPECT_EQ(set.width(), 6);
    EXPECT_EQ(set.uniqueness().width(), 2);
    EXPECT_EQ(set.slots().bit_size(), 24);
    EXPECT_EQ(set.slots().capacity(), 64);
    EXPECT_EQ(set.lookup(s2), Slot<uint32_t>(0, 1));

    // Insert s3
    const auto s3 = set.insert(Slot<uint32_t>(3, 2));
    EXPECT_EQ(s3, 2);
    EXPECT_EQ(set.size(), 3);
    EXPECT_EQ(set.width(), 6);
    EXPECT_EQ(set.uniqueness().width(), 2);
    EXPECT_EQ(set.slots().bit_size(), 24);
    EXPECT_EQ(set.slots().capacity(), 64);
    EXPECT_EQ(set.lookup(s3), Slot<uint32_t>(3, 2));

    // Insert s4
    const auto s4 = set.insert(Slot<uint32_t>(1, 4));
    EXPECT_EQ(s4, 3);
    EXPECT_EQ(set.size(), 4);
    EXPECT_EQ(set.width(), 6);
    EXPECT_EQ(set.uniqueness().width(), 2);
    EXPECT_EQ(set.slots().bit_size(), 24);
    EXPECT_EQ(set.slots().capacity(), 64);
    EXPECT_EQ(set.lookup(s4), Slot<uint32_t>(1, 4));

    // Insert s5
    const auto s5 = set.insert(Slot<uint32_t>(5, 1));
    EXPECT_EQ(s5, 4);
    EXPECT_EQ(set.size(), 5);
    EXPECT_EQ(set.width(), 6);
    EXPECT_EQ(set.uniqueness().width(), 3);
    EXPECT_EQ(set.slots().bit_size(), 48);
    EXPECT_EQ(set.slots().capacity(), 64);
    EXPECT_EQ(set.lookup(s5), Slot<uint32_t>(5, 1));
}
}
