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
#include <valla/double_tree_compression.hpp>

namespace valla::tests
{

TEST(VallaTests, DoubleTreeCompressionTest)
{
    auto tree_table = IndexedHashSet<Slot>();
    auto double_table = IndexedHashSet<double>();
    auto root_table = IndexedHashSet<Slot>();
    auto tmp_state = DoubleList();

    {
        const auto s0 = DoubleList { 0., 1., 2., 3., 4., 5., 6., 7., 8., 9., 10., 11., 12., 13., 14., 15. };
        const auto [s0_result, success] = root_table.insert(doubles::plain::insert(s0, tree_table, double_table));
        const auto& s0_root = s0_result->first;
        const auto& s0_idx = s0_result->second;

        EXPECT_EQ(tree_table.size(), 15);
        EXPECT_EQ(root_table.size(), 1);

        // Created new state!
        EXPECT_EQ(s0_idx, 0);

        doubles::plain::read_state(s0_root, tree_table, double_table, tmp_state);
        EXPECT_EQ(tmp_state, s0);

        EXPECT_EQ(doubles::plain::read_value(s0_root, 0, tree_table, double_table), 0);
        EXPECT_EQ(doubles::plain::read_value(s0_root, 7, tree_table, double_table), 7);
        EXPECT_EQ(doubles::plain::read_value(s0_root, 8, tree_table, double_table), 8);
        EXPECT_EQ(doubles::plain::read_value(s0_root, 15, tree_table, double_table), 15);
    }

    {
        const auto s1 = DoubleList { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
        const auto [s1_result, success] = root_table.insert(doubles::plain::insert(s1, tree_table, double_table));
        const auto& s1_root = s1_result->first;
        const auto& s1_idx = s1_result->second;

        EXPECT_EQ(tree_table.size(), 16);
        EXPECT_EQ(root_table.size(), 2);

        // Created new state!
        EXPECT_EQ(s1_idx, 1);

        doubles::plain::read_state(s1_root, tree_table, double_table, tmp_state);
        EXPECT_EQ(tmp_state, s1);
    }

    {
        const auto s2 = DoubleList { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 };
        const auto [s2_result, success] = root_table.insert(doubles::plain::insert(s2, tree_table, double_table));
        const auto& s2_root = s2_result->first;
        const auto& s2_idx = s2_result->second;

        EXPECT_EQ(tree_table.size(), 17);
        EXPECT_EQ(root_table.size(), 3);

        // Created new state!
        EXPECT_EQ(s2_idx, 2);

        doubles::plain::read_state(s2_root, tree_table, double_table, tmp_state);
        EXPECT_EQ(tmp_state, s2);
    }

    {
        const auto s3 = DoubleList { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 };
        const auto [s3_result, success] = root_table.insert(doubles::plain::insert(s3, tree_table, double_table));
        const auto& s3_root = s3_result->first;
        const auto& s3_idx = s3_result->second;

        EXPECT_EQ(tree_table.size(), 17);
        EXPECT_EQ(root_table.size(), 3);

        // DoubleList already exists!
        EXPECT_EQ(s3_idx, 2);

        doubles::plain::read_state(s3_root, tree_table, double_table, tmp_state);
        EXPECT_EQ(tmp_state, s3);
    }
}

TEST(VallaTests, DoubleTreeCompressionEdgeCasesTest)
{
    auto tree_table = IndexedHashSet<Slot>();
    auto double_table = IndexedHashSet<double>();
    auto root_table = IndexedHashSet<Slot>();
    auto tmp_state = DoubleList();

    {
        const auto s0 = DoubleList {};
        const auto [s0_result, success] = root_table.insert(doubles::plain::insert(s0, tree_table, double_table));
        const auto& s0_root = s0_result->first;
        const auto& s0_idx = s0_result->second;

        EXPECT_EQ(tree_table.size(), 0);
        EXPECT_EQ(root_table.size(), 1);

        // Created new state!
        EXPECT_EQ(s0_idx, 0);

        doubles::plain::read_state(s0_root, tree_table, double_table, tmp_state);
        EXPECT_EQ(tmp_state, s0);
    }

    {
        const auto s1 = DoubleList { 0 };
        const auto [s1_result, success] = root_table.insert(doubles::plain::insert(s1, tree_table, double_table));
        const auto& s1_root = s1_result->first;
        const auto& s1_idx = s1_result->second;

        EXPECT_EQ(tree_table.size(), 0);
        EXPECT_EQ(root_table.size(), 2);

        // Created new state!
        EXPECT_EQ(s1_idx, 1);

        doubles::plain::read_state(s1_root, tree_table, double_table, tmp_state);
        EXPECT_EQ(tmp_state, s1);
    }
}

TEST(VallaTests, DoubleTreeCompressionIteratorTest)
{
    auto tree_table = IndexedHashSet<Slot>();
    auto double_table = IndexedHashSet<double>();
    auto root_table = IndexedHashSet<Slot>();
    auto tmp_state = DoubleList();

    {
        const auto s0 = DoubleList { 1, 2, 4, 5, 6 };
        const auto [s0_result, success] = root_table.insert(doubles::plain::insert(s0, tree_table, double_table));
        const auto& s0_root = s0_result->first;
        const auto& s0_idx = s0_result->second;

        EXPECT_EQ(s0, DoubleList(doubles::plain::begin(s0_root, tree_table, double_table), doubles::plain::end()));
    }

    {
        const auto s0 = DoubleList {};
        EXPECT_EQ(s0, DoubleList(doubles::plain::begin(Slot(), tree_table, double_table), doubles::plain::end()));
    }
}

}
