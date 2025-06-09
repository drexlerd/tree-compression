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
#include <valla/bitset_pool.hpp>

namespace valla::tests
{

TEST(VallaTests, BitsetPoolTest)
{
    auto pool = BitsetPool();

    auto view1 = pool.allocate(70);
    auto view2 = pool.allocate(70);

    EXPECT_EQ(view1.get_segment(), 0);
    EXPECT_EQ(view1.get_offset(), 0);
    EXPECT_EQ(view2.get_segment(), 0);
    EXPECT_EQ(view2.get_offset(), 70);

    EXPECT_FALSE(view1.get(0, pool));
    EXPECT_FALSE(view1.get(63, pool));
    EXPECT_FALSE(view1.get(64, pool));

    view1.set(0, pool);
    view1.set(63, pool);
    view1.set(64, pool);

    EXPECT_TRUE(view1.get(0, pool));
    EXPECT_TRUE(view1.get(63, pool));
    EXPECT_TRUE(view1.get(64, pool));

    EXPECT_FALSE(view2.get(0, pool));
    EXPECT_FALSE(view2.get(63, pool));
    EXPECT_FALSE(view2.get(64, pool));

    view2.set(0, pool);
    view2.set(63, pool);
    view2.set(64, pool);

    EXPECT_TRUE(view2.get(0, pool));
    EXPECT_TRUE(view2.get(63, pool));
    EXPECT_TRUE(view2.get(64, pool));
}

}
