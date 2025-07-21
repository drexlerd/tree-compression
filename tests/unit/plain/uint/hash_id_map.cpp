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
    auto root_table = IndexedHashSet<Index>();
    auto tree_table = TreeHashIDMap<>(root_table);
    auto tmp_state = IndexList();

    {
        const auto s0 = IndexList { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
        const auto s0_idx = root_table.insert(v::insert(s0, tree_table));
        const auto& s0_root = root_table[s0_idx];

        EXPECT_EQ(tree_table.size(), 15);
        EXPECT_EQ(root_table.size(), 1);

        // Created new state!
        EXPECT_EQ(s0_idx, 0);

        v::read_state(s0_root, tree_table, tmp_state);
        EXPECT_EQ(tmp_state, s0);
    }

    {
        const auto s1 = IndexList { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
        const auto s1_idx = root_table.insert(v::insert(s1, tree_table));
        const auto& s1_root = root_table[s1_idx];

        EXPECT_EQ(tree_table.size(), 16);
        EXPECT_EQ(root_table.size(), 2);

        // Created new state!
        EXPECT_EQ(s1_idx, 1);

        v::read_state(s1_root, tree_table, tmp_state);
        EXPECT_EQ(tmp_state, s1);
    }

    {
        const auto s2 = IndexList { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 };
        const auto s2_idx = root_table.insert(v::insert(s2, tree_table));
        const auto& s2_root = root_table[s2_idx];

        EXPECT_EQ(tree_table.size(), 18);
        EXPECT_EQ(root_table.size(), 3);

        // Created new state!
        EXPECT_EQ(s2_idx, 2);

        v::read_state(s2_root, tree_table, tmp_state);
        EXPECT_EQ(tmp_state, s2);
    }

    {
        const auto s3 = IndexList { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 };
        const auto s3_idx = root_table.insert(v::insert(s3, tree_table));
        const auto& s3_root = root_table[s3_idx];

        EXPECT_EQ(tree_table.size(), 18);
        EXPECT_EQ(root_table.size(), 3);

        // IndexList already exists!
        EXPECT_EQ(s3_idx, 2);

        v::read_state(s3_root, tree_table, tmp_state);
        EXPECT_EQ(tmp_state, s3);
    }
}

TEST(VallaTests, PlainUintHashIDMapEdgeCasesTest)
{
    auto root_table = IndexedHashSet<Index>();
    auto tree_table = TreeHashIDMap<>(root_table);
    auto tmp_state = IndexList();

    {
        const auto s0 = IndexList {};
        const auto s0_idx = root_table.insert(v::insert(s0, tree_table));
        const auto& s0_root = root_table[s0_idx];

        EXPECT_EQ(tree_table.size(), 0);
        EXPECT_EQ(root_table.size(), 1);

        // Created new state!
        EXPECT_EQ(s0_idx, 0);

        v::read_state(s0_root, tree_table, tmp_state);
        EXPECT_EQ(tmp_state, s0);
    }

    {
        const auto s1 = IndexList { 0 };
        const auto s1_idx = root_table.insert(v::insert(s1, tree_table));
        const auto& s1_root = root_table[s1_idx];

        EXPECT_EQ(tree_table.size(), 0);
        EXPECT_EQ(root_table.size(), 2);

        // Created new state!
        EXPECT_EQ(s1_idx, 1);

        v::read_state(s1_root, tree_table, tmp_state);
        EXPECT_EQ(tmp_state, s1);
    }
}

TEST(VallaTests, PlainUintHashIDMapIteratorTest)
{
    auto root_table = IndexedHashSet<Index>();
    auto tree_table = TreeHashIDMap<>(root_table);
    auto tmp_state = IndexList();

    {
        const auto s0 = IndexList { 1, 2, 4, 5, 6 };
        const auto s0_idx = root_table.insert(v::insert(s0, tree_table));
        const auto& s0_root = root_table[s0_idx];

        EXPECT_EQ(s0, IndexList(v::begin(s0_root, tree_table), v::end()));
    }

    {
        const auto s0 = IndexList {};
        EXPECT_EQ(s0, IndexList(v::begin(EMPTY_ROOT_SLOT, tree_table), v::end()));
    }
}

}
