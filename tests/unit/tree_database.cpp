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
#include <valla/tree_database.hpp>

namespace valla::tests
{

TEST(VallaTests, HashIdMapTest)
{
    TreeDatabase<SlotHash, std::equal_to<Slot>, 8> map(2);

    std::cout << map << std::endl << std::endl;

    auto state1 = IndexList { 0, 1, 2, 3 };
    map.insert(state1);

    std::cout << map << std::endl << std::endl;

    auto state2 = IndexList { 1, 2, 3, 4, 5, 6, 7, 8 };
    map.insert(state2);

    std::cout << map << std::endl << std::endl;
}

TEST(VallaTests, HashIdMap2Test)
{
    TreeDatabase<SlotHash, std::equal_to<Slot>, 8> map(2);

    std::cout << map << std::endl << std::endl;

    auto state1 = IndexList { 1, 2, 3, 4, 5, 6, 7, 8 };
    map.insert(state1);

    std::cout << map << std::endl << std::endl;

    auto state2 = IndexList { 1, 2, 3, 4, 5, 6, 7, 8 };
    map.insert(state2);

    std::cout << map << std::endl << std::endl;
}

}
