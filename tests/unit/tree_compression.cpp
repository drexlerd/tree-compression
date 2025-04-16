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
#include <valla/tree_compression.hpp>

namespace valla::tests
{
TEST(VallaTests, TreeCompressionTest)
{
    auto tree_table = IndexedHashSet();
    auto root_table = IndexedHashSet();
    auto tmp_state = State();

    {
        const auto s0 = State { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
        const auto s0_idx = plain::insert(s0, tree_table, root_table).first->second;

        EXPECT_EQ(tree_table.size(), 8);
        EXPECT_EQ(root_table.size(), 1);

        // Created new state!
        EXPECT_EQ(s0_idx, 0);

        plain::read_state(s0_idx, tree_table, root_table, tmp_state);
        EXPECT_EQ(tmp_state, s0);
    }

    {
        const auto s1 = State { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
        const auto s1_idx = plain::insert(s1, tree_table, root_table).first->second;

        EXPECT_EQ(tree_table.size(), 9);
        EXPECT_EQ(root_table.size(), 2);

        // Created new state!
        EXPECT_EQ(s1_idx, 1);

        plain::read_state(s1_idx, tree_table, root_table, tmp_state);
        EXPECT_EQ(tmp_state, s1);
    }

    {
        const auto s2 = State { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 };
        const auto s2_idx = plain::insert(s2, tree_table, root_table).first->second;

        EXPECT_EQ(tree_table.size(), 11);
        EXPECT_EQ(root_table.size(), 3);

        // Created new state!
        EXPECT_EQ(s2_idx, 2);

        plain::read_state(s2_idx, tree_table, root_table, tmp_state);
        EXPECT_EQ(tmp_state, s2);
    }

    {
        const auto s3 = State { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 };
        const auto s3_idx = plain::insert(s3, tree_table, root_table).first->second;

        EXPECT_EQ(tree_table.size(), 11);
        EXPECT_EQ(root_table.size(), 3);

        // State already exists!
        EXPECT_EQ(s3_idx, 2);

        plain::read_state(s3_idx, tree_table, root_table, tmp_state);
        EXPECT_EQ(tmp_state, s3);
    }
}

TEST(VallaTests, TreeCompressionEdgeCasesTest)
{
    auto tree_table = IndexedHashSet();
    auto root_table = IndexedHashSet();
    auto tmp_state = State();

    {
        const auto s0 = State {};
        const auto s0_idx = plain::insert(s0, tree_table, root_table).first->second;

        EXPECT_EQ(tree_table.size(), 0);
        EXPECT_EQ(root_table.size(), 1);

        // Created new state!
        EXPECT_EQ(s0_idx, 0);

        plain::read_state(s0_idx, tree_table, root_table, tmp_state);
        EXPECT_EQ(tmp_state, s0);
    }

    {
        const auto s1 = State { 0 };
        const auto s1_idx = plain::insert(s1, tree_table, root_table).first->second;

        EXPECT_EQ(tree_table.size(), 0);
        EXPECT_EQ(root_table.size(), 2);

        // Created new state!
        EXPECT_EQ(s1_idx, 1);

        plain::read_state(s1_idx, tree_table, root_table, tmp_state);
        EXPECT_EQ(tmp_state, s1);
    }
}

TEST(VallaTests, TreeCompressionIteratorTest)
{
    auto tree_table = IndexedHashSet();
    auto root_table = IndexedHashSet();
    auto tmp_state = State();

    {
        const auto s0 = State { 1, 2, 4, 5, 6 };
        const auto s0_idx = plain::insert(s0, tree_table, root_table).first->second;
        const auto s0_root = root_table.get_slot(s0_idx);

        EXPECT_EQ(s0, State(plain::begin(s0_root, tree_table), plain::end()));
    }

    {
        const auto s0 = State {};
        EXPECT_EQ(s0, State(plain::begin(Slot(), tree_table), plain::end()));
    }
}

}
