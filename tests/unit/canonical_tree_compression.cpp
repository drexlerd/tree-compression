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
#include <valla/canonical_tree_compression.hpp>

namespace valla::tests
{

TEST(VallaTests, CanonicalTreeCompressionTest)
{
    auto tree_table = IndexedHashSet();
    auto pool = BitsetPool();
    auto root_table = RootIndexedHashSet(pool);

    auto tmp_state = State();

    {
        const auto s0 = State { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
        const auto s0_idx = canonical::insert(s0, tree_table, root_table, pool).first->second;

        EXPECT_EQ(tree_table.size(), 8);
        EXPECT_EQ(root_table.size(), 1);

        // Created new state!
        EXPECT_EQ(s0_idx, 0);

        // plain::read_state(s0_idx, tree_table, root_table, tmp_state);
        // EXPECT_EQ(tmp_state, s0);
    }
}

}
