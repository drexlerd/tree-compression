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
#include <valla/plain/double/hash_id_map.hpp>

namespace valla::tests
{
namespace v = valla::plain::dbl::hash_id_map;

TEST(VallaTests, PlainDoubleHashIDMapTest)
{
    auto inner_table = TreeHashIDMap<>();
    auto leaf_table = IndexedHashSet<double>();

    auto tmp_state = DoubleList();

    {
        const auto s0 = DoubleList { 0., 1., 2., 3., 4., 5., 6., 7., 8., 9., 10., 11., 12., 13., 14., 15. };
        const auto s0_idx = v::insert(s0, inner_table, leaf_table);

        EXPECT_EQ(inner_table.size(), 7);
        EXPECT_EQ(leaf_table.size(), 8);

        // Created new state!
        EXPECT_EQ(s0_idx, 1);

        v::read_state(s0_idx, inner_table, leaf_table, tmp_state);
        EXPECT_EQ(tmp_state, s0);
    }

    {
        const auto s1 = DoubleList { 0., 1., 2., 3., 4., 5., 6., 7., 8., 9., 10., 11., 12., 13., 14., 15., 16. };
        const auto s1_idx = v::insert(s1, inner_table, leaf_table);

        EXPECT_EQ(inner_table.size(), 8);
        EXPECT_EQ(leaf_table.size(), 9);

        // Created new state!
        EXPECT_EQ(s1_idx, 2);

        v::read_state(s1_idx, inner_table, leaf_table, tmp_state);
        EXPECT_EQ(tmp_state, s1);
    }

    {
        const auto s2 = DoubleList { 0., 1., 2., 3., 4., 5., 6., 7., 8., 9., 10., 11., 12., 13., 14., 15., 16., 17. };
        const auto s2_idx = v::insert(s2, inner_table, leaf_table);

        EXPECT_EQ(inner_table.size(), 9);
        EXPECT_EQ(leaf_table.size(), 10);

        // Created new state!
        EXPECT_EQ(s2_idx, 3);

        v::read_state(s2_idx, inner_table, leaf_table, tmp_state);
        EXPECT_EQ(tmp_state, s2);
    }

    {
        const auto s3 = DoubleList { 0., 1., 2., 3., 4., 5., 6., 7., 8., 9., 10., 11., 12., 13., 14., 15., 16., 17. };
        const auto s3_idx = v::insert(s3, inner_table, leaf_table);

        EXPECT_EQ(inner_table.size(), 9);
        EXPECT_EQ(leaf_table.size(), 10);

        // DoubleList already exists!
        EXPECT_EQ(s3_idx, 3);

        v::read_state(s3_idx, inner_table, leaf_table, tmp_state);
        EXPECT_EQ(tmp_state, s3);
    }
}

TEST(VallaTests, PlainDoubleHashIDMapEdgeCasesTest)
{
    auto inner_table = TreeHashIDMap<>();
    auto leaf_table = IndexedHashSet<double>();

    auto tmp_state = DoubleList();

    {
        const auto s0 = DoubleList {};
        const auto s0_idx = v::insert(s0, inner_table, leaf_table);

        EXPECT_EQ(inner_table.size(), 0);
        EXPECT_EQ(leaf_table.size(), 0);

        // Created new state!
        EXPECT_EQ(s0_idx, 0);

        v::read_state(s0_idx, inner_table, leaf_table, tmp_state);
        EXPECT_EQ(tmp_state, s0);
    }

    {
        const auto s1 = DoubleList { 0 };
        const auto s1_idx = v::insert(s1, inner_table, leaf_table);

        EXPECT_EQ(inner_table.size(), 0);
        EXPECT_EQ(leaf_table.size(), 1);

        // Created new state!
        EXPECT_EQ(s1_idx, 1);

        v::read_state(s1_idx, inner_table, leaf_table, tmp_state);
        EXPECT_EQ(tmp_state, s1);
    }
}

TEST(VallaTests, PlainDoubleHashIDMapIteratorTest)
{
    auto inner_table = TreeHashIDMap<>();
    auto leaf_table = IndexedHashSet<double>();

    auto tmp_state = DoubleList();

    {
        const auto s0 = DoubleList { 1, 2, 4, 5, 6 };
        const auto s0_idx = v::insert(s0, inner_table, leaf_table);

        EXPECT_EQ(s0, DoubleList(v::begin(s0_idx, inner_table, leaf_table), v::end(inner_table)));
    }

    {
        // const auto s0 = DoubleList {};
        // EXPECT_EQ(s0, DoubleList(v::begin(v::EMPTY_ROOT_SLOT, inner_table, leaf_table), v::end()));
    }
}

TEST(VallaTests, PlainDoubleHashIDMapExhaustiveTest)
{
    const size_t state_num = static_cast<size_t>(1000);  // number of states
    const size_t state_size = static_cast<size_t>(9);    // size of each state

    std::mt19937 rng(42);  // fixed seed for reproducibility
    std::uniform_int_distribution<Index> value_dist(0, 1000);

    std::vector<DoubleList> all_s;
    all_s.reserve(state_num);

    // Generate sorted random states
    for (size_t i = 0; i < state_num; ++i)
    {
        DoubleList s(state_size);
        for (auto& v : s)
            v = value_dist(rng);

        std::sort(s.begin(), s.end());
        all_s.push_back(std::move(s));
    }

    auto inner_table = TreeHashIDMap<>();
    auto leaf_table = IndexedHashSet<double>();

    auto out_state = DoubleList {};

    auto all_roots = IndexList {};

    for (size_t i = 0; i < all_s.size(); ++i)
    {
        const auto& s1 = all_s[i];

        auto root = v::insert(s1, inner_table, leaf_table);

        all_roots.push_back(root);

        for (size_t j = 0; j <= i; ++j)
        {
            const auto& s2 = all_s[j];
            const auto& root = all_roots[j];

            v::read_state(root, inner_table, leaf_table, out_state);
            EXPECT_EQ(s2, out_state);

            out_state.clear();
            out_state.insert(out_state.end(), v::begin(root, inner_table, leaf_table), v::end(inner_table));
            EXPECT_EQ(s2, out_state);

            out_state.clear();
            for (const auto x : v::range(root, inner_table, leaf_table))
                out_state.push_back(x);
            EXPECT_EQ(s2, out_state);
        }
    }
}

}
