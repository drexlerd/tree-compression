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
#include <valla/valla.hpp>

namespace valla::tests
{

TEST(VallaTests, PlainUintHashIDMapTest)
{
    auto table = TreeHashIDMap<uint32_t>();
    auto leaf_table = IndexedHashSet<double, uint32_t>();

    auto index_list = std::vector<uint32_t>();
    auto double_list = std::vector<double>();

    /* uint32_t */
    {
        const auto s0 = std::vector<uint32_t> { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
        const auto s0_idx = insert(s0, table);

        EXPECT_EQ(table.size(), 15);

        // Created new state!
        EXPECT_EQ(s0_idx, 1);

        read_state(s0_idx, table, index_list);
        EXPECT_EQ(index_list, s0);
    }

    {
        const auto s1 = std::vector<uint32_t> { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
        const auto s1_idx = insert(s1, table);

        EXPECT_EQ(table.size(), 16);

        // Created new state!
        EXPECT_EQ(s1_idx, 2);

        read_state(s1_idx, table, index_list);
        EXPECT_EQ(index_list, s1);
    }

    {
        const auto s2 = std::vector<uint32_t> { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 };
        const auto s2_idx = insert(s2, table);

        EXPECT_EQ(table.size(), 18);

        // Created new state!
        EXPECT_EQ(s2_idx, 3);

        read_state(s2_idx, table, index_list);
        EXPECT_EQ(index_list, s2);
    }

    {
        const auto s3 = std::vector<uint32_t> { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 };
        const auto s3_idx = insert(s3, table);

        EXPECT_EQ(table.size(), 18);

        // std::vector<uint32_t> already exists!
        EXPECT_EQ(s3_idx, 3);

        read_state(s3_idx, table, index_list);
        EXPECT_EQ(index_list, s3);
    }

    /* double*/

    {
        const auto s0 = std::vector<double> { 0., 1., 2., 3., 4., 5., 6., 7., 8., 9., 10., 11., 12., 13., 14., 15. };
        const auto s0_idx = insert(s0, table, leaf_table);

        EXPECT_EQ(table.size(), 18);
        EXPECT_EQ(leaf_table.size(), 16);

        // Created new state!
        EXPECT_EQ(s0_idx, 1);

        read_state(s0_idx, table, leaf_table, double_list);
        EXPECT_EQ(double_list, s0);
    }

    {
        const auto s1 = std::vector<double> { 0., 1., 2., 3., 4., 5., 6., 7., 8., 9., 10., 11., 12., 13., 14., 15., 16. };
        const auto s1_idx = insert(s1, table, leaf_table);

        EXPECT_EQ(table.size(), 18);
        EXPECT_EQ(leaf_table.size(), 17);

        // Created new state!
        EXPECT_EQ(s1_idx, 2);

        read_state(s1_idx, table, leaf_table, double_list);
        EXPECT_EQ(double_list, s1);
    }

    {
        const auto s2 = std::vector<double> { 0., 1., 2., 3., 4., 5., 6., 7., 8., 9., 10., 11., 12., 13., 14., 15., 16., 17. };
        const auto s2_idx = insert(s2, table, leaf_table);

        EXPECT_EQ(table.size(), 18);
        EXPECT_EQ(leaf_table.size(), 18);

        // Created new state!
        EXPECT_EQ(s2_idx, 3);

        read_state(s2_idx, table, leaf_table, double_list);
        EXPECT_EQ(double_list, s2);
    }

    {
        const auto s3 = std::vector<double> { 0., 1., 2., 3., 4., 5., 6., 7., 8., 9., 10., 11., 12., 13., 14., 15., 16., 17. };
        const auto s3_idx = insert(s3, table, leaf_table);

        EXPECT_EQ(table.size(), 18);
        EXPECT_EQ(leaf_table.size(), 18);

        // std::vector<double> already exists!
        EXPECT_EQ(s3_idx, 3);

        read_state(s3_idx, table, leaf_table, double_list);
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
        const auto s0_idx = insert(s0, table);

        EXPECT_EQ(table.size(), 0);

        // Created new state!
        EXPECT_EQ(s0_idx, 0);

        read_state(s0_idx, table, index_list);
        EXPECT_EQ(index_list, s0);
    }

    {
        const auto s1 = std::vector<uint32_t> { 0 };
        const auto s1_idx = insert(s1, table);

        EXPECT_EQ(table.size(), 0);

        // Created new state!
        EXPECT_EQ(s1_idx, 1);

        read_state(s1_idx, table, index_list);
        EXPECT_EQ(index_list, s1);
    }

    {
        const auto s0 = std::vector<double> {};
        const auto s0_idx = insert(s0, table, leaf_table);

        EXPECT_EQ(table.size(), 0);
        EXPECT_EQ(leaf_table.size(), 0);

        // Created new state!
        EXPECT_EQ(s0_idx, 0);

        read_state(s0_idx, table, leaf_table, double_list);
        EXPECT_EQ(double_list, s0);
    }

    {
        const auto s1 = std::vector<double> { 0 };
        const auto s1_idx = insert(s1, table, leaf_table);

        EXPECT_EQ(table.size(), 0);
        EXPECT_EQ(leaf_table.size(), 1);

        // Created new state!
        EXPECT_EQ(s1_idx, 1);

        read_state(s1_idx, table, leaf_table, double_list);
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
        const auto s0_idx = insert(s0, table);

        EXPECT_EQ(s0, std::vector<uint32_t>(begin(s0_idx, table), end(table)));
    }

    {
        const auto s0 = std::vector<uint32_t> {};
        EXPECT_EQ(s0, std::vector<uint32_t>(begin(uint32_t(0), table), end(table)));
    }

    {
        const auto s0 = std::vector<double> { 1, 2, 4, 5, 6 };
        const auto s0_idx = insert(s0, table, leaf_table);

        EXPECT_EQ(s0, std::vector<double>(begin(s0_idx, table, leaf_table), end(table, leaf_table)));
    }

    {
        const auto s0 = std::vector<double> {};
        EXPECT_EQ(s0, std::vector<double>(begin(uint32_t(0), table, leaf_table), end(table, leaf_table)));
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

    std::vector<std::vector<uint32_t>> ils;
    ils.reserve(num_sequences);
    std::vector<std::vector<double>> dls;
    dls.reserve(num_sequences);

    std::vector<uint32_t> start_il(sequence_size);
    for (auto& v : start_il)
        v = index_dist(rng);
    ils.push_back(start_il);

    std::vector<double> start_dl(sequence_size);
    for (auto& v : start_dl)
        v = double_dist(rng);
    dls.push_back(start_dl);

    // Generate sorted random states
    for (size_t i = 1; i < num_sequences; ++i)
    {
        size_t num_changes = changes_dist(rng);

        std::vector<uint32_t> il = ils[i - 1];
        for (size_t j = 0; j < num_changes; ++j)
            il[pos_dist(rng)] = index_dist(rng);
        ils.push_back(std::move(il));

        std::vector<double> dl = dls[i - 1];
        for (size_t j = 0; j < num_changes; ++j)
            dl[pos_dist(rng)] = double_dist(rng);
        dls.push_back(std::move(dl));
    }

    auto table = TreeHashIDMap<uint32_t>();
    auto leaf_table = IndexedHashSet<double, uint32_t>();

    auto out_il = std::vector<uint32_t> {};
    auto out_dl = std::vector<double> {};

    auto irs = std::vector<uint32_t> {};
    auto drs = std::vector<uint32_t> {};

    for (size_t i = 0; i < ils.size(); ++i)
    {
        const auto& il = ils[i];
        const auto& dl = dls[i];

        auto ir = insert(il, table);
        auto dr = insert(dl, table, leaf_table);

        irs.push_back(ir);
        drs.push_back(dr);

        /* Ensure newly inserted index sequence is readable*/
        read_state(ir, table, out_il);
        EXPECT_EQ(il, out_il);

        out_il.clear();
        out_il.insert(out_il.end(), begin(ir, table), end(table));
        EXPECT_EQ(il, out_il);

        out_il.clear();
        for (const auto x : range(ir, table))
            out_il.push_back(x);
        EXPECT_EQ(il, out_il);

        /* Ensure newly inserted double sequence is readable*/
        read_state(dr, table, leaf_table, out_dl);
        EXPECT_EQ(dl, out_dl);

        out_dl.clear();
        out_dl.insert(out_dl.end(), begin(dr, table, leaf_table), end(table, leaf_table));
        EXPECT_EQ(dl, out_dl);

        out_dl.clear();
        for (const auto x : range(dr, table, leaf_table))
            out_dl.push_back(x);
        EXPECT_EQ(dl, out_dl);

        /* Ensure that all index sequences are readable after rehash. */
        for (size_t j = 0; j <= i; ++j)
        {
            const auto& il_2 = ils[j];
            const auto& ir_2 = irs[j];

            read_state(ir_2, table, out_il);
            EXPECT_EQ(il_2, out_il);

            out_il.clear();
            out_il.insert(out_il.end(), begin(ir_2, table), end(table));
            EXPECT_EQ(il_2, out_il);

            out_il.clear();
            for (const auto x : range(ir_2, table))
                out_il.push_back(x);
            EXPECT_EQ(il_2, out_il);
        }

        /* Ensure that all double sequences are readable after rehash. */
        for (size_t j = 0; j <= i; ++j)
        {
            const auto& dl_2 = dls[j];
            const auto& dr_2 = drs[j];

            read_state(dr_2, table, leaf_table, out_dl);
            EXPECT_EQ(dl_2, out_dl);

            out_dl.clear();
            out_dl.insert(out_dl.end(), begin(dr_2, table, leaf_table), end(table, leaf_table));
            EXPECT_EQ(dl_2, out_dl);

            out_dl.clear();
            for (const auto x : range(dr_2, table, leaf_table))
                out_dl.push_back(x);
            EXPECT_EQ(dl_2, out_dl);
        }
    }
}

}
