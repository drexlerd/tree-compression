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

#include "../include/valla/tree_compression.hpp"

#include <gtest/gtest.h>

namespace valla::tests
{
TEST(VallaTests, TreeCompressionTest)
{
    auto tree_table = IndexedHashSet();
    auto root_table = IndexedHashSet();

    {
        const auto s0 = State { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
        const auto [s0_idx, success] = insert(s0, tree_table, root_table);

        EXPECT_EQ(tree_table.size(), 8);
        EXPECT_EQ(root_table.size(), 1);

        EXPECT_EQ(s0_idx, 0);
    }

    {
        const auto s1 = State { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
        const auto [s1_idx, success] = insert(s1, tree_table, root_table);

        EXPECT_EQ(tree_table.size(), 10);
        EXPECT_EQ(root_table.size(), 2);

        EXPECT_EQ(s1_idx, 1);
    }

    {
        const auto s2 = State { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 };
        const auto [s2_idx, success] = insert(s2, tree_table, root_table);

        EXPECT_EQ(tree_table.size(), 12);
        EXPECT_EQ(root_table.size(), 3);

        EXPECT_EQ(s2_idx, 2);
    }

    {
        const auto s3 = State { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 };
        const auto [s3_idx, success] = insert(s3, tree_table, root_table);

        EXPECT_EQ(tree_table.size(), 12);
        EXPECT_EQ(root_table.size(), 3);

        EXPECT_EQ(s3_idx, 2);
    }
}

}
