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

TEST(VallaTests, TreeDatabaseTest)
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

TEST(VallaTests, TreeDatabase2Test)
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

TEST(VallaTests, TreeDatabaseSpecialCasesTest)
{
    tdb::TreeDatabase<SlotHash, std::equal_to<Slot>, 8> db(2);

    std::cout << db << std::endl << std::endl;

    auto s_empty = IndexList {};
    EXPECT_EQ(IndexList(db.begin(0), db.end()), s_empty);
}

TEST(VallaTests, TreeDatabaseExhaustiveTest)
{
    tdb::TreeDatabase<SlotHash, std::equal_to<Slot>, 4> db(2);

    const size_t state_num = 10000;  // number of states
    const size_t state_size = 999;   // size of each state

    std::mt19937 rng(42);  // fixed seed for reproducibility
    std::uniform_int_distribution<Index> dist(0, 10'000);

    std::vector<IndexList> all_states;
    all_states.reserve(state_num);

    // Generate sorted random states
    for (size_t i = 0; i < state_num; ++i)
    {
        IndexList s(state_size);
        for (auto& v : s)
            v = dist(rng);

        std::sort(s.begin(), s.end());
        s.erase(std::unique(s.begin(), s.end()), s.end());
        all_states.push_back(std::move(s));
    }

    IndexList all_roots;

    for (const auto& s : all_states)
    {
        all_roots.push_back(db.insert(s));
    }

    auto state = IndexList();

    for (size_t i = 0; i < state_num; ++i)
    {
        state.clear();
        state.insert(state.end(), db.begin(all_roots[i]), db.end());

        EXPECT_EQ(state, all_states[i]);
    }
}
}
