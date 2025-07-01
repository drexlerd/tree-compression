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
    tdb::TreeDatabase<SlotHash, std::equal_to<Slot>, 8> db(2);

    std::cout << db << std::endl << std::endl;

    auto s1 = IndexList { 0, 1, 2, 3 };
    auto s1_root = db.insert(s1);

    EXPECT_EQ(IndexList(db.begin(s1_root), db.end()), s1);

    std::cout << db << std::endl << std::endl;

    auto s2 = IndexList { 1, 2, 3, 4, 5, 6, 7, 8 };
    auto s2_root = db.insert(s2);

    EXPECT_EQ(IndexList(db.begin(s2_root), db.end()), s2);

    std::cout << db << std::endl << std::endl;
}

TEST(VallaTests, HashIdMap2Test)
{
    tdb::TreeDatabase<SlotHash, std::equal_to<Slot>, 8> db(2);

    std::cout << db << std::endl << std::endl;

    auto s1 = IndexList { 1, 2, 3, 4, 5, 6, 7, 8 };
    auto s1_root = db.insert(s1);

    EXPECT_EQ(IndexList(db.begin(s1_root), db.end()), s1);

    std::cout << db << std::endl << std::endl;

    auto s2 = IndexList { 1, 2, 3, 4, 5, 6, 7, 8 };
    auto s2_root = db.insert(s2);

    EXPECT_EQ(IndexList(db.begin(s2_root), db.end()), s2);

    std::cout << db << std::endl << std::endl;
}

}
