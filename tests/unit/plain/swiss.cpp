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
#include <valla/plain/swiss.hpp>

namespace valla::tests
{
namespace v = valla::plain::swiss;

/**
 * Plain
 */

TEST(VallaTests, PlainUintSwissTest)
{
    auto root_table = IndexedHashSet<Slot<uint32_t>, uint32_t>();
    auto inner_table = IndexedHashSet<Slot<uint32_t>, uint32_t>();
    auto leaf_table = IndexedHashSet<double, uint32_t>();

    auto index_list = std::vector<uint32_t>();
    auto double_list = std::vector<double>();

    /* uint32_t */
    {
        const auto s0 = std::vector<uint32_t> { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
        const auto s0_slot = v::insert(s0, inner_table);
        const auto s0_root = root_table.insert(s0_slot);

        EXPECT_EQ(inner_table.size(), 8);

        // Created new state!
        EXPECT_EQ(s0_root, 0);

        v::read_state(s0_slot, inner_table, index_list);
        EXPECT_EQ(index_list, s0);
    }

    {
        const auto s1 = std::vector<uint32_t> { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
        const auto s1_slot = v::insert(s1, inner_table);
        const auto s1_root = root_table.insert(s1_slot);

        EXPECT_EQ(inner_table.size(), 9);

        // Created new state!
        EXPECT_EQ(s1_root, 1);

        v::read_state(s1_slot, inner_table, index_list);
        EXPECT_EQ(index_list, s1);
    }

    {
        const auto s2 = std::vector<uint32_t> { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 };
        const auto s2_slot = v::insert(s2, inner_table);
        const auto s2_root = root_table.insert(s2_slot);

        EXPECT_EQ(inner_table.size(), 11);

        // Created new state!
        EXPECT_EQ(s2_root, 2);

        v::read_state(s2_slot, inner_table, index_list);
        EXPECT_EQ(index_list, s2);
    }

    {
        const auto s3 = std::vector<uint32_t> { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 };
        const auto s3_slot = v::insert(s3, inner_table);
        const auto s3_root = root_table.insert(s3_slot);

        EXPECT_EQ(inner_table.size(), 11);

        // std::vector<uint32_t> already exists!
        EXPECT_EQ(s3_root, 2);

        v::read_state(s3_slot, inner_table, index_list);
        EXPECT_EQ(index_list, s3);
    }

    /* double*/

    {
        const auto s0 = std::vector<double> { 0., 1., 2., 3., 4., 5., 6., 7., 8., 9., 10., 11., 12., 13., 14., 15. };
        const auto s0_slot = v::insert(s0, inner_table, leaf_table);
        const auto s0_root = root_table.insert(s0_slot);

        EXPECT_EQ(inner_table.size(), 11);
        EXPECT_EQ(leaf_table.size(), 16);

        // Created new state!
        EXPECT_EQ(s0_root, 0);

        v::read_state(s0_slot, inner_table, leaf_table, double_list);
        EXPECT_EQ(double_list, s0);
    }

    {
        const auto s1 = std::vector<double> { 0., 1., 2., 3., 4., 5., 6., 7., 8., 9., 10., 11., 12., 13., 14., 15., 16. };
        const auto s1_slot = v::insert(s1, inner_table, leaf_table);
        const auto s1_root = root_table.insert(s1_slot);

        EXPECT_EQ(inner_table.size(), 11);
        EXPECT_EQ(leaf_table.size(), 17);

        // Created new state!
        EXPECT_EQ(s1_root, 1);

        v::read_state(s1_slot, inner_table, leaf_table, double_list);
        EXPECT_EQ(double_list, s1);
    }

    {
        const auto s2 = std::vector<double> { 0., 1., 2., 3., 4., 5., 6., 7., 8., 9., 10., 11., 12., 13., 14., 15., 16., 17. };
        const auto s2_slot = v::insert(s2, inner_table, leaf_table);
        const auto s2_root = root_table.insert(s2_slot);

        EXPECT_EQ(inner_table.size(), 11);
        EXPECT_EQ(leaf_table.size(), 18);

        // Created new state!
        EXPECT_EQ(s2_root, 2);

        v::read_state(s2_slot, inner_table, leaf_table, double_list);
        EXPECT_EQ(double_list, s2);
    }

    {
        const auto s3 = std::vector<double> { 0., 1., 2., 3., 4., 5., 6., 7., 8., 9., 10., 11., 12., 13., 14., 15., 16., 17. };
        const auto s3_slot = v::insert(s3, inner_table, leaf_table);
        const auto s3_root = root_table.insert(s3_slot);

        EXPECT_EQ(inner_table.size(), 11);
        EXPECT_EQ(leaf_table.size(), 18);

        // std::vector<double> already exists!
        EXPECT_EQ(s3_root, 2);

        v::read_state(s3_slot, inner_table, leaf_table, double_list);
        EXPECT_EQ(double_list, s3);
    }
}

TEST(VallaTests, PlainUintSwissEdgeCasesTest)
{
    auto root_table = IndexedHashSet<Slot<uint32_t>, uint32_t>();
    auto inner_table = IndexedHashSet<Slot<uint32_t>, uint32_t>();
    auto leaf_table = IndexedHashSet<double, uint32_t>();

    auto index_list = std::vector<uint32_t>();
    auto double_list = std::vector<double>();

    {
        const auto s0 = std::vector<uint32_t> {};
        const auto s0_slot = v::insert(s0, inner_table);
        const auto s0_root = root_table.insert(s0_slot);

        EXPECT_EQ(inner_table.size(), 0);

        // Created new state!
        EXPECT_EQ(s0_root, 0);

        v::read_state(s0_slot, inner_table, index_list);
        EXPECT_EQ(index_list, s0);
    }

    {
        const auto s1 = std::vector<uint32_t> { 0 };
        const auto s1_slot = v::insert(s1, inner_table);
        const auto s1_root = root_table.insert(s1_slot);

        EXPECT_EQ(inner_table.size(), 0);

        // Created new state!
        EXPECT_EQ(s1_root, 1);

        v::read_state(s1_slot, inner_table, index_list);
        EXPECT_EQ(index_list, s1);
    }

    {
        const auto s0 = std::vector<double> {};
        const auto s0_slot = v::insert(s0, inner_table, leaf_table);
        const auto s0_root = root_table.insert(s0_slot);

        EXPECT_EQ(inner_table.size(), 0);
        EXPECT_EQ(leaf_table.size(), 0);

        // Created new state!
        EXPECT_EQ(s0_root, 0);

        v::read_state(s0_slot, inner_table, leaf_table, double_list);
        EXPECT_EQ(double_list, s0);
    }

    {
        const auto s1 = std::vector<double> { 0 };
        const auto s1_slot = v::insert(s1, inner_table, leaf_table);
        const auto s1_root = root_table.insert(s1_slot);

        EXPECT_EQ(inner_table.size(), 0);
        EXPECT_EQ(leaf_table.size(), 1);

        // Created new state!
        EXPECT_EQ(s1_root, 1);

        v::read_state(s1_slot, inner_table, leaf_table, double_list);
        EXPECT_EQ(double_list, s1);
    }
}

TEST(VallaTests, PlainUintSwissIteratorTest)
{
    auto inner_table = IndexedHashSet<Slot<uint32_t>, uint32_t>();
    auto leaf_table = IndexedHashSet<double, uint32_t>();

    auto index_list = std::vector<uint32_t>();
    auto double_list = std::vector<double>();

    {
        const auto s0 = std::vector<uint32_t> { 1, 2, 4, 5, 6 };
        const auto s0_idx = v::insert(s0, inner_table);

        EXPECT_EQ(s0, std::vector<uint32_t>(v::begin(s0_idx, inner_table), v::end<uint32_t>()));
    }

    {
        const auto s0 = std::vector<uint32_t> {};
        EXPECT_EQ(s0, std::vector<uint32_t>(v::begin(get_empty_slot<uint32_t>(), inner_table), v::end<uint32_t>()));
    }

    {
        const auto s0 = std::vector<double> { 1, 2, 4, 5, 6 };
        const auto s0_idx = v::insert(s0, inner_table, leaf_table);

        EXPECT_EQ(s0, std::vector<double>(v::begin(s0_idx, inner_table, leaf_table), v::end(leaf_table)));
    }

    {
        const auto s0 = std::vector<double> {};
        EXPECT_EQ(s0, std::vector<double>(v::begin(get_empty_slot<uint32_t>(), inner_table, leaf_table), v::end(leaf_table)));
    }
}

TEST(VallaTests, PlainUintSwissExhaustiveTest)
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

        std::vector<uint32_t> index_list = ils[i - 1];
        for (size_t j = 0; j < num_changes; ++j)
            index_list[pos_dist(rng)] = index_dist(rng);
        ils.push_back(std::move(index_list));

        std::vector<double> double_list = dls[i - 1];
        for (size_t j = 0; j < num_changes; ++j)
            double_list[pos_dist(rng)] = double_dist(rng);
        dls.push_back(std::move(double_list));
    }

    auto inner_table = IndexedHashSet<Slot<uint32_t>, uint32_t>();
    auto leaf_table = IndexedHashSet<double, uint32_t>();

    auto out_il = std::vector<uint32_t> {};
    auto out_dl = std::vector<double> {};

    auto irs = std::vector<Slot<uint32_t>> {};
    auto drs = std::vector<Slot<uint32_t>> {};

    for (size_t i = 0; i < ils.size(); ++i)
    {
        const auto& il = ils[i];
        const auto& dl = dls[i];

        auto ir = v::insert(il, inner_table);
        auto dr = v::insert(dl, inner_table, leaf_table);

        irs.push_back(ir);
        drs.push_back(dr);

        /* Ensure newly inserted index sequence is readable*/
        v::read_state(ir, inner_table, out_il);
        EXPECT_EQ(il, out_il);

        out_il.clear();
        out_il.insert(out_il.end(), v::begin(ir, inner_table), v::end<uint32_t>());
        EXPECT_EQ(il, out_il);

        out_il.clear();
        for (const auto x : v::range(ir, inner_table))
            out_il.push_back(x);
        EXPECT_EQ(il, out_il);

        /* Ensure newly inserted double sequence is readable*/
        v::read_state(dr, inner_table, leaf_table, out_dl);
        EXPECT_EQ(dl, out_dl);

        out_dl.clear();
        out_dl.insert(out_dl.end(), v::begin(dr, inner_table, leaf_table), v::end(leaf_table));
        EXPECT_EQ(dl, out_dl);

        out_dl.clear();
        for (const auto x : v::range(dr, inner_table, leaf_table))
            out_dl.push_back(x);
        EXPECT_EQ(dl, out_dl);

        /* Ensure that all index sequences are readable after rehash. */
        for (size_t j = 0; j <= i; ++j)
        {
            const auto& il_2 = ils[j];
            const auto& ir_2 = irs[j];

            v::read_state(ir_2, inner_table, out_il);
            EXPECT_EQ(il_2, out_il);

            out_il.clear();
            out_il.insert(out_il.end(), v::begin(ir_2, inner_table), v::end<uint32_t>());
            EXPECT_EQ(il_2, out_il);

            out_il.clear();
            for (const auto x : v::range(ir_2, inner_table))
                out_il.push_back(x);
            EXPECT_EQ(il_2, out_il);
        }

        /* Ensure that all double sequences are readable after rehash. */
        for (size_t j = 0; j <= i; ++j)
        {
            const auto& dl_2 = dls[j];
            const auto& dr_2 = drs[j];

            v::read_state(dr_2, inner_table, leaf_table, out_dl);
            EXPECT_EQ(dl_2, out_dl);

            out_dl.clear();
            out_dl.insert(out_dl.end(), v::begin(dr_2, inner_table, leaf_table), v::end(leaf_table));
            EXPECT_EQ(dl_2, out_dl);

            out_dl.clear();
            for (const auto x : v::range(dr_2, inner_table, leaf_table))
                out_dl.push_back(x);
            EXPECT_EQ(dl_2, out_dl);
        }
    }
}

/**
 * Succinct
 */

TEST(VallaTests, SuccinctUintSwissTest)
{
    auto root_table = SuccinctIndexedHashSet<Slot<uint32_t>, uint32_t>();
    auto inner_table = SuccinctIndexedHashSet<Slot<uint32_t>, uint32_t>();
    auto leaf_table = IndexedHashSet<double, uint32_t>();

    auto index_list = std::vector<uint32_t>();
    auto double_list = std::vector<double>();

    /* uint32_t */
    {
        const auto s0 = std::vector<uint32_t> { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
        const auto s0_slot = v::insert(s0, inner_table);
        const auto s0_root = root_table.insert(s0_slot);

        EXPECT_EQ(inner_table.size(), 8);

        // Created new state!
        EXPECT_EQ(s0_root, 0);

        v::read_state(s0_slot, inner_table, index_list);
        EXPECT_EQ(index_list, s0);
    }

    {
        const auto s1 = std::vector<uint32_t> { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
        const auto s1_slot = v::insert(s1, inner_table);
        const auto s1_root = root_table.insert(s1_slot);

        EXPECT_EQ(inner_table.size(), 9);

        // Created new state!
        EXPECT_EQ(s1_root, 1);

        v::read_state(s1_slot, inner_table, index_list);
        EXPECT_EQ(index_list, s1);
    }

    {
        const auto s2 = std::vector<uint32_t> { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 };
        const auto s2_slot = v::insert(s2, inner_table);
        const auto s2_root = root_table.insert(s2_slot);

        EXPECT_EQ(inner_table.size(), 11);

        // Created new state!
        EXPECT_EQ(s2_root, 2);

        v::read_state(s2_slot, inner_table, index_list);
        EXPECT_EQ(index_list, s2);
    }

    {
        const auto s3 = std::vector<uint32_t> { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 };
        const auto s3_slot = v::insert(s3, inner_table);
        const auto s3_root = root_table.insert(s3_slot);

        EXPECT_EQ(inner_table.size(), 11);

        // std::vector<uint32_t> already exists!
        EXPECT_EQ(s3_root, 2);

        v::read_state(s3_slot, inner_table, index_list);
        EXPECT_EQ(index_list, s3);
    }

    /* double*/

    {
        const auto s0 = std::vector<double> { 0., 1., 2., 3., 4., 5., 6., 7., 8., 9., 10., 11., 12., 13., 14., 15. };
        const auto s0_slot = v::insert(s0, inner_table, leaf_table);
        const auto s0_root = root_table.insert(s0_slot);

        EXPECT_EQ(inner_table.size(), 11);
        EXPECT_EQ(leaf_table.size(), 16);

        // Created new state!
        EXPECT_EQ(s0_root, 0);

        v::read_state(s0_slot, inner_table, leaf_table, double_list);
        EXPECT_EQ(double_list, s0);
    }

    {
        const auto s1 = std::vector<double> { 0., 1., 2., 3., 4., 5., 6., 7., 8., 9., 10., 11., 12., 13., 14., 15., 16. };
        const auto s1_slot = v::insert(s1, inner_table, leaf_table);
        const auto s1_root = root_table.insert(s1_slot);

        EXPECT_EQ(inner_table.size(), 11);
        EXPECT_EQ(leaf_table.size(), 17);

        // Created new state!
        EXPECT_EQ(s1_root, 1);

        v::read_state(s1_slot, inner_table, leaf_table, double_list);
        EXPECT_EQ(double_list, s1);
    }

    {
        const auto s2 = std::vector<double> { 0., 1., 2., 3., 4., 5., 6., 7., 8., 9., 10., 11., 12., 13., 14., 15., 16., 17. };
        const auto s2_slot = v::insert(s2, inner_table, leaf_table);
        const auto s2_root = root_table.insert(s2_slot);

        EXPECT_EQ(inner_table.size(), 11);
        EXPECT_EQ(leaf_table.size(), 18);

        // Created new state!
        EXPECT_EQ(s2_root, 2);

        v::read_state(s2_slot, inner_table, leaf_table, double_list);
        EXPECT_EQ(double_list, s2);
    }

    {
        const auto s3 = std::vector<double> { 0., 1., 2., 3., 4., 5., 6., 7., 8., 9., 10., 11., 12., 13., 14., 15., 16., 17. };
        const auto s3_slot = v::insert(s3, inner_table, leaf_table);
        const auto s3_root = root_table.insert(s3_slot);

        EXPECT_EQ(inner_table.size(), 11);
        EXPECT_EQ(leaf_table.size(), 18);

        // std::vector<double> already exists!
        EXPECT_EQ(s3_root, 2);

        v::read_state(s3_slot, inner_table, leaf_table, double_list);
        EXPECT_EQ(double_list, s3);
    }
}

}
