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
#include <valla/plain/uint/hash_id_map.hpp>

namespace valla::tests
{
namespace v = valla::plain::uint::hash_id_map;

TEST(VallaTests, PlainUintHashIDMapTest)
{
    auto table = TreeHashIDMap<>();
    auto tmp_state = IndexList();

    {
        const auto s0 = IndexList { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
        const auto s0_idx = v::insert(s0, table);

        EXPECT_EQ(table.size(), 15);
        EXPECT_EQ(table.num_roots(), 2);

        // Created new state!
        EXPECT_EQ(s0_idx, 1);

        v::read_state(s0_idx, table, tmp_state);
        EXPECT_EQ(tmp_state, s0);
    }

    {
        const auto s1 = IndexList { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
        const auto s1_idx = v::insert(s1, table);

        EXPECT_EQ(table.size(), 16);
        EXPECT_EQ(table.num_roots(), 3);

        // Created new state!
        EXPECT_EQ(s1_idx, 2);

        v::read_state(s1_idx, table, tmp_state);
        EXPECT_EQ(tmp_state, s1);
    }

    {
        const auto s2 = IndexList { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 };
        const auto s2_idx = v::insert(s2, table);

        EXPECT_EQ(table.size(), 18);
        EXPECT_EQ(table.num_roots(), 4);

        // Created new state!
        EXPECT_EQ(s2_idx, 3);

        v::read_state(s2_idx, table, tmp_state);
        EXPECT_EQ(tmp_state, s2);
    }

    {
        const auto s3 = IndexList { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 };
        const auto s3_idx = v::insert(s3, table);

        EXPECT_EQ(table.size(), 18);
        EXPECT_EQ(table.num_roots(), 4);

        // IndexList already exists!
        EXPECT_EQ(s3_idx, 3);

        v::read_state(s3_idx, table, tmp_state);
        EXPECT_EQ(tmp_state, s3);
    }
}

TEST(VallaTests, PlainUintHashIDMapEdgeCasesTest)
{
    auto table = TreeHashIDMap<>();
    auto tmp_state = IndexList();

    {
        const auto s0 = IndexList {};
        const auto s0_idx = v::insert(s0, table);

        EXPECT_EQ(table.size(), 0);
        EXPECT_EQ(table.num_roots(), 1);

        // Created new state!
        EXPECT_EQ(s0_idx, 0);

        v::read_state(s0_idx, table, tmp_state);
        EXPECT_EQ(tmp_state, s0);
    }

    {
        const auto s1 = IndexList { 0 };
        const auto s1_idx = v::insert(s1, table);

        EXPECT_EQ(table.size(), 0);
        EXPECT_EQ(table.num_roots(), 2);

        // Created new state!
        EXPECT_EQ(s1_idx, 1);

        v::read_state(s1_idx, table, tmp_state);
        EXPECT_EQ(tmp_state, s1);
    }
}

TEST(VallaTests, PlainUintHashIDMapIteratorTest)
{
    auto table = TreeHashIDMap<>();
    auto tmp_state = IndexList();

    {
        const auto s0 = IndexList { 1, 2, 4, 5, 6 };
        const auto s0_idx = v::insert(s0, table);

        EXPECT_EQ(s0, IndexList(v::begin(s0_idx, table), v::end(table)));
    }

    {
        const auto s0 = IndexList {};
        EXPECT_EQ(s0, IndexList(v::begin(0, table), v::end(table)));
    }
}

TEST(VallaTests, PlainUintHashIDMapExhaustiveTest)
{
    const size_t state_num = static_cast<size_t>(50);  // number of states
    const size_t state_size = static_cast<size_t>(6);  // size of each state

    std::mt19937 rng(42);  // fixed seed for reproducibility
    std::uniform_int_distribution<Index> dist(0, 10000);

    std::vector<IndexList> all_states;
    all_states.reserve(state_num);

    // Generate sorted random states
    for (size_t i = 0; i < state_num; ++i)
    {
        IndexList s(state_size);
        for (auto& v : s)
            v = dist(rng);

        std::sort(s.begin(), s.end());
        all_states.push_back(std::move(s));
    }

    TreeHashIDMap<Hasher<Slot<Index>>, std::equal_to<Slot<Index>>, size_t(64)> table;

    for (const auto& s : all_states)
    {
        v::insert(s, table);
    }
}

}
