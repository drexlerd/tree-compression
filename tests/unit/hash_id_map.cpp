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
#include <valla/hash_id_map.hpp>

namespace valla::tests
{

TEST(VallaTests, HashIdMapTest)
{
    HashIdMap<SlotHash> map(2);

    auto i0 = map.insert(Slot());

    EXPECT_EQ(i0, 0);
    EXPECT_EQ(map.size(), 1);

    auto i1 = map.insert(Slot(0, 1));
    EXPECT_EQ(i0, 0);
}

}
