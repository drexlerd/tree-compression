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
#include <valla/static_tree_compression.hpp>

namespace valla::tests
{

TEST(VallaTests, StaticTreeCompressionTest)
{
    auto tree_table = IndexedHashSet();
    auto root_table = RootIndexedHashSet();
    auto tmp_state = State();

    {
        const auto s0 = State { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
        const auto s0_size = std::distance(s0.begin(), s0.end());
        const auto s0_idx = static_tree::insert(s0, tree_table, root_table).first;

        // The tree table has inserted |s0|/2 = 8 slots.
        EXPECT_EQ(tree_table.size(), 8);
        // The root table has a single state insertion.
        EXPECT_EQ(root_table.size(), 1);

        // Created new state!
        EXPECT_EQ(s0_idx, 0);

        tmp_state.reserve(s0_size);
        static_tree::read_state(s0_idx, s0_size, tree_table, root_table, tmp_state);
        EXPECT_EQ(tmp_state, s0);
    }

    {
        const auto s1 = State { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 16, 17 };
        const auto s1_size = std::distance(s1.begin(), s1.end());
        const auto s1_idx = static_tree::insert(s1, tree_table, root_table).first;

        // The tree table has additional log2(|s1/2|)+1 = 4 slots. As only one entry is added.
        EXPECT_EQ(tree_table.size(), 12);
        EXPECT_EQ(root_table.size(), 2);

        // Created new state!
        EXPECT_EQ(s1_idx, 1);

        tmp_state.reserve(s1_size);
        static_tree::read_state(s1_idx, s1_size, tree_table, root_table, tmp_state);
        EXPECT_EQ(tmp_state, s1);
    }

    {
        const auto s2 = State { 8, 9, 10, 11, 12, 13, 16, 17, 0, 1, 2, 3, 4, 5, 6, 7 };
        const auto s2_size = std::distance(s2.begin(), s2.end());
        const auto s2_idx = static_tree::insert(s2, tree_table, root_table).first;

        EXPECT_EQ(tree_table.size(), 13);
        EXPECT_EQ(root_table.size(), 3);

        // Created new state!
        EXPECT_EQ(s2_idx, 2);

        tmp_state.reserve(s2_size);
        static_tree::read_state(s2_idx, s2_size, tree_table, root_table, tmp_state);
        EXPECT_EQ(tmp_state, s2);
    }

    {
        const auto s3 = State { 8, 9, 10, 11, 12, 13, 16, 17, 0, 1, 2, 3, 4, 5, 6, 7 };
        static_tree::insert(s3, tree_table, root_table).first;

        // State already exists!
        EXPECT_EQ(tree_table.size(), 13);
        EXPECT_EQ(root_table.size(), 3);
    }
}

TEST(VallaTests, StaticTreeCompressionEdgeCasesTest)
{
    auto tree_table = IndexedHashSet();
    auto root_table = RootIndexedHashSet();
    auto tmp_state = State();

    {
        const auto s0 = State {};
        const auto s0_size = std::distance(s0.begin(), s0.end());
        const auto s0_idx = static_tree::insert(s0, tree_table, root_table).first;

        EXPECT_EQ(tree_table.size(), 0);
        EXPECT_EQ(root_table.size(), 0);

        // Created new state!
        EXPECT_EQ(s0_idx, 0);

        tmp_state.reserve(s0_size);
        static_tree::read_state(s0_idx, s0_size, tree_table, root_table, tmp_state);
        EXPECT_EQ(tmp_state, s0);
    }

    {
        const auto s1 = State { 0 };
        const auto s1_size = std::distance(s1.begin(), s1.end());
        const auto s1_idx = static_tree::insert(s1, tree_table, root_table).first;

        EXPECT_EQ(tree_table.size(), 1);
        EXPECT_EQ(root_table.size(), 1);

        // Created new state!
        EXPECT_EQ(s1_idx, 0);

        tmp_state.reserve(s1_size);
        static_tree::read_state(s1_idx, s1_size, tree_table, root_table, tmp_state);
        EXPECT_EQ(tmp_state, s1);
    }
}

TEST(VallaTests, StaticTreeCompressionIteratorTest)
{
    auto tree_table = IndexedHashSet();
    auto root_table = RootIndexedHashSet();
    auto tmp_state = State();

    {
        const auto s0 = State { 1, 2, 4, 5, 6 };
        const auto s0_size = std::distance(s0.begin(), s0.end());
        const auto s0_idx = static_tree::insert(s0, tree_table, root_table).first;
        const auto s0_root = root_table.get_slot(s0_idx);

        EXPECT_EQ(s0, State(static_tree::begin(s0_root, s0_size, tree_table), static_tree::end()));
    }
}

}
