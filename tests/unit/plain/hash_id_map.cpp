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
#include <valla/plain/hash_id_map.hpp>

namespace valla::tests
{
namespace v = valla::plain::uint::hash_id_map;

TEST(VallaTests, PlainUintHashIDMapTest)
{
    auto table = TreeHashIDMap<uint32_t>();
    auto leaf_table = IndexedHashSet<double, uint32_t>();

    auto index_list = std::vector<uint32_t>();
    auto double_list = std::vector<double>();

    /* uint32_t */
    {
        const auto s0 = std::vector<uint32_t> { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
        const auto s0_idx = v::insert(s0, table);

        EXPECT_EQ(table.size(), 15);

        // Created new state!
        EXPECT_EQ(s0_idx, 1);

        v::read_state(s0_idx, table, index_list);
        EXPECT_EQ(index_list, s0);
    }

    {
        const auto s1 = std::vector<uint32_t> { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
        const auto s1_idx = v::insert(s1, table);

        EXPECT_EQ(table.size(), 16);

        // Created new state!
        EXPECT_EQ(s1_idx, 2);

        v::read_state(s1_idx, table, index_list);
        EXPECT_EQ(index_list, s1);
    }

    {
        const auto s2 = std::vector<uint32_t> { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 };
        const auto s2_idx = v::insert(s2, table);

        EXPECT_EQ(table.size(), 18);

        // Created new state!
        EXPECT_EQ(s2_idx, 3);

        v::read_state(s2_idx, table, index_list);
        EXPECT_EQ(index_list, s2);
    }

    {
        const auto s3 = std::vector<uint32_t> { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 };
        const auto s3_idx = v::insert(s3, table);

        EXPECT_EQ(table.size(), 18);

        // std::vector<uint32_t> already exists!
        EXPECT_EQ(s3_idx, 3);

        v::read_state(s3_idx, table, index_list);
        EXPECT_EQ(index_list, s3);
    }

    /* double*/

    {
        const auto s0 = std::vector<double> { 0., 1., 2., 3., 4., 5., 6., 7., 8., 9., 10., 11., 12., 13., 14., 15. };
        const auto s0_idx = v::insert(s0, table, leaf_table);

        EXPECT_EQ(table.size(), 18);
        EXPECT_EQ(leaf_table.size(), 16);

        // Created new state!
        EXPECT_EQ(s0_idx, 1);

        v::read_state(s0_idx, table, leaf_table, double_list);
        EXPECT_EQ(double_list, s0);
    }

    {
        const auto s1 = std::vector<double> { 0., 1., 2., 3., 4., 5., 6., 7., 8., 9., 10., 11., 12., 13., 14., 15., 16. };
        const auto s1_idx = v::insert(s1, table, leaf_table);

        EXPECT_EQ(table.size(), 18);
        EXPECT_EQ(leaf_table.size(), 17);

        // Created new state!
        EXPECT_EQ(s1_idx, 2);

        v::read_state(s1_idx, table, leaf_table, double_list);
        EXPECT_EQ(double_list, s1);
    }

    {
        const auto s2 = std::vector<double> { 0., 1., 2., 3., 4., 5., 6., 7., 8., 9., 10., 11., 12., 13., 14., 15., 16., 17. };
        const auto s2_idx = v::insert(s2, table, leaf_table);

        EXPECT_EQ(table.size(), 18);
        EXPECT_EQ(leaf_table.size(), 18);

        // Created new state!
        EXPECT_EQ(s2_idx, 3);

        v::read_state(s2_idx, table, leaf_table, double_list);
        EXPECT_EQ(double_list, s2);
    }

    {
        const auto s3 = std::vector<double> { 0., 1., 2., 3., 4., 5., 6., 7., 8., 9., 10., 11., 12., 13., 14., 15., 16., 17. };
        const auto s3_idx = v::insert(s3, table, leaf_table);

        EXPECT_EQ(table.size(), 18);
        EXPECT_EQ(leaf_table.size(), 18);

        // std::vector<double> already exists!
        EXPECT_EQ(s3_idx, 3);

        v::read_state(s3_idx, table, leaf_table, double_list);
        EXPECT_EQ(double_list, s3);
    }
}

TEST(VallaTests, PlainUintHashIDMapEdgeCasesTest)
{
    auto table = TreeHashIDMap<uint32_t>();
    auto leaf_table = IndexedHashSet<double, uint32_t>();

    auto index_list = std::vector<uint32_t>();
    auto double_list = std::vector<double>();

    {
        const auto s0 = std::vector<uint32_t> {};
        const auto s0_idx = v::insert(s0, table);

        EXPECT_EQ(table.size(), 0);

        // Created new state!
        EXPECT_EQ(s0_idx, 0);

        v::read_state(s0_idx, table, index_list);
        EXPECT_EQ(index_list, s0);
    }

    {
        const auto s1 = std::vector<uint32_t> { 0 };
        const auto s1_idx = v::insert(s1, table);

        EXPECT_EQ(table.size(), 0);

        // Created new state!
        EXPECT_EQ(s1_idx, 1);

        v::read_state(s1_idx, table, index_list);
        EXPECT_EQ(index_list, s1);
    }

    {
        const auto s0 = std::vector<double> {};
        const auto s0_idx = v::insert(s0, table, leaf_table);

        EXPECT_EQ(table.size(), 0);
        EXPECT_EQ(leaf_table.size(), 0);

        // Created new state!
        EXPECT_EQ(s0_idx, 0);

        v::read_state(s0_idx, table, leaf_table, double_list);
        EXPECT_EQ(double_list, s0);
    }

    {
        const auto s1 = std::vector<double> { 0 };
        const auto s1_idx = v::insert(s1, table, leaf_table);

        EXPECT_EQ(table.size(), 0);
        EXPECT_EQ(leaf_table.size(), 1);

        // Created new state!
        EXPECT_EQ(s1_idx, 1);

        v::read_state(s1_idx, table, leaf_table, double_list);
        EXPECT_EQ(double_list, s1);
    }
}

TEST(VallaTests, PlainUintHashIDMapIteratorTest)
{
    auto table = TreeHashIDMap<uint32_t>();
    auto leaf_table = IndexedHashSet<double, uint32_t>();

    auto index_list = std::vector<uint32_t>();
    auto double_list = std::vector<double>();

    {
        const auto s0 = std::vector<uint32_t> { 1, 2, 4, 5, 6 };
        const auto s0_idx = v::insert(s0, table);

        EXPECT_EQ(s0, std::vector<uint32_t>(v::begin(s0_idx, table), v::end(table)));
    }

    {
        const auto s0 = std::vector<uint32_t> {};
        EXPECT_EQ(s0, std::vector<uint32_t>(v::begin(uint32_t(0), table), v::end(table)));
    }

    {
        const auto s0 = std::vector<double> { 1, 2, 4, 5, 6 };
        const auto s0_idx = v::insert(s0, table, leaf_table);

        EXPECT_EQ(s0, std::vector<double>(v::begin(s0_idx, table, leaf_table), v::end(table, leaf_table)));
    }

    {
        const auto s0 = std::vector<double> {};
        EXPECT_EQ(s0, std::vector<double>(v::begin(uint32_t(0), table, leaf_table), v::end(table, leaf_table)));
    }
}

TEST(VallaTests, PlainUintHashIDMapExhaustiveTest)
{
    const size_t num_sequences = static_cast<size_t>(1000);  // number of states
    const size_t sequence_size = static_cast<size_t>(29);    // size of each state

    /* Create random sequences */

    std::mt19937 rng(42);  // fixed seed for reproducibility
    std::uniform_int_distribution<uint32_t> index_dist(0, 1000);
    std::uniform_real_distribution<double> double_dist(0, 1000);
    std::uniform_int_distribution<size_t> changes_dist(1, 5);
    std::uniform_int_distribution<size_t> pos_dist(0, sequence_size - 1);

    std::vector<std::vector<uint32_t>> index_lists;
    index_lists.reserve(num_sequences);
    std::vector<std::vector<double>> double_lists;
    double_lists.reserve(num_sequences);

    std::vector<uint32_t> start_index_list(sequence_size);
    for (auto& v : start_index_list)
        v = index_dist(rng);
    index_lists.push_back(start_index_list);

    std::vector<double> start_double_list(sequence_size);
    for (auto& v : start_double_list)
        v = double_dist(rng);
    double_lists.push_back(start_double_list);

    // Generate sorted random states
    for (size_t i = 1; i < num_sequences; ++i)
    {
        size_t num_changes = changes_dist(rng);

        std::vector<uint32_t> index_list = index_lists[i - 1];
        for (size_t j = 0; j < num_changes; ++j)
            index_list[pos_dist(rng)] = index_dist(rng);
        index_lists.push_back(std::move(index_list));

        std::vector<double> double_list = double_lists[i - 1];
        for (size_t j = 0; j < num_changes; ++j)
            double_list[pos_dist(rng)] = double_dist(rng);
        double_lists.push_back(std::move(double_list));
    }

    auto table = TreeHashIDMap<uint32_t>();
    auto leaf_table = IndexedHashSet<double, uint32_t>();

    auto out_index_list = std::vector<uint32_t> {};
    auto out_double_list = std::vector<uint32_t> {};

    auto index_list_roots = std::vector<uint32_t> {};
    auto double_list_roots = std::vector<uint32_t> {};

    for (size_t i = 0; i < index_lists.size(); ++i)
    {
        const auto& s1 = index_lists[i];

        auto root = v::insert(s1, table);

        v::read_state(root, table, out_index_list);
        EXPECT_EQ(s1, out_index_list);

        out_index_list.clear();
        out_index_list.insert(out_index_list.end(), v::begin(root, table), v::end(table));
        EXPECT_EQ(s1, out_index_list);

        out_index_list.clear();
        for (const auto x : v::range(root, table))
            out_index_list.push_back(x);
        EXPECT_EQ(s1, out_index_list);

        index_list_roots.push_back(root);

        for (size_t j = 0; j <= i; ++j)
        {
            const auto& s2 = index_lists[j];
            const auto& root = index_list_roots[j];

            v::read_state(root, table, out_index_list);
            EXPECT_EQ(s2, out_index_list);

            out_index_list.clear();
            out_index_list.insert(out_index_list.end(), v::begin(root, table), v::end(table));
            EXPECT_EQ(s2, out_index_list);

            out_index_list.clear();
            for (const auto x : v::range(root, table))
                out_index_list.push_back(x);
            EXPECT_EQ(s2, out_index_list);
        }
    }
}

}
