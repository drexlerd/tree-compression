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
TEST(VallaTests, CompactFlatHashSetUint32Test)
{
    auto set = compact_flat_hash_set<uint32_t> {};

    // Insert s0
    const auto [s0, s0_success] = set.insert(uint32_t(0));
    EXPECT_EQ(set[s0], uint32_t(0));
    EXPECT_EQ(set.size(), 1);
    EXPECT_EQ(set.width(), 1);

    // Insert s1 (bit resize from 2 to 6)
    const auto [s1, s1_success] = set.insert(uint32_t(1));
    EXPECT_EQ(set[s1], uint32_t(1));
    EXPECT_EQ(set.size(), 2);
    EXPECT_EQ(set.width(), 1);

    // Insert s2 (same as s0)
    const auto [s2, s2_success] = set.insert(uint32_t(0));
    EXPECT_EQ(set[s2], uint32_t(0));
    EXPECT_EQ(set.size(), 2);
    EXPECT_EQ(set.width(), 1);
    // Insert s3
    const auto [s3, s3_success] = set.insert(uint32_t(2));
    EXPECT_EQ(set[s3], uint32_t(2));
    EXPECT_EQ(set.size(), 3);
    EXPECT_EQ(set.width(), 2);
}

TEST(VallaTests, CompactFlatHashSetSlotTest)
{
    auto set = compact_flat_hash_set<Slot<uint32_t>> {};

    // Insert s0
    const auto [s0, s0_success] = set.insert(Slot<uint32_t>(0, 1));
    EXPECT_EQ(set[s0], Slot<uint32_t>(0, 1));
    EXPECT_EQ(set.size(), 1);
    EXPECT_EQ(set.width(), 2);

    // Insert s1 (bit resize from 2 to 6)
    const auto [s1, s1_success] = set.insert(Slot<uint32_t>(4, 5));
    EXPECT_EQ(set[s1], Slot<uint32_t>(4, 5));
    EXPECT_EQ(set.size(), 2);
    EXPECT_EQ(set.width(), 6);

    // Insert s2 (same as s0)
    const auto [s2, s2_success] = set.insert(Slot<uint32_t>(0, 1));
    EXPECT_EQ(set[s2], Slot<uint32_t>(0, 1));
    EXPECT_EQ(set.size(), 2);
    EXPECT_EQ(set.width(), 6);
    // Insert s3
    const auto [s3, s3_success] = set.insert(Slot<uint32_t>(3, 2));
    EXPECT_EQ(set[s3], Slot<uint32_t>(3, 2));
    EXPECT_EQ(set.size(), 3);
    EXPECT_EQ(set.width(), 6);

    // Insert s4
    const auto [s4, s4_success] = set.insert(Slot<uint32_t>(1, 4));
    EXPECT_EQ(set[s4], Slot<uint32_t>(1, 4));
    EXPECT_EQ(set.size(), 4);
    EXPECT_EQ(set.width(), 6);

    // Insert s5
    const auto [s5, s5_success] = set.insert(Slot<uint32_t>(5, 1));
    EXPECT_EQ(set[s5], Slot<uint32_t>(5, 1));
    EXPECT_EQ(set.size(), 5);
    EXPECT_EQ(set.width(), 6);
}
}
