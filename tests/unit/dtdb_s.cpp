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
        const auto s0_slot = insert_sequence(s0, inner_table);
        const auto s0_root = root_table.insert(s0_slot);

        EXPECT_EQ(inner_table.size(), 8);

        // Created new state!
        EXPECT_EQ(s0_root, 0);

        index_list.clear();
        read_sequence(s0_slot, inner_table, std::back_inserter(index_list));
        EXPECT_EQ(index_list, s0);
    }

    {
        const auto s1 = std::vector<uint32_t> { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
        const auto s1_slot = insert_sequence(s1, inner_table);
        const auto s1_root = root_table.insert(s1_slot);

        EXPECT_EQ(inner_table.size(), 9);

        // Created new state!
        EXPECT_EQ(s1_root, 1);

        index_list.clear();
        read_sequence(s1_slot, inner_table, std::back_inserter(index_list));
        EXPECT_EQ(index_list, s1);
    }

    {
        const auto s2 = std::vector<uint32_t> { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 };
        const auto s2_slot = insert_sequence(s2, inner_table);
        const auto s2_root = root_table.insert(s2_slot);

        EXPECT_EQ(inner_table.size(), 11);

        // Created new state!
        EXPECT_EQ(s2_root, 2);

        index_list.clear();
        read_sequence(s2_slot, inner_table, std::back_inserter(index_list));
        EXPECT_EQ(index_list, s2);
    }

    {
        const auto s3 = std::vector<uint32_t> { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 };
        const auto s3_slot = insert_sequence(s3, inner_table);
        const auto s3_root = root_table.insert(s3_slot);

        EXPECT_EQ(inner_table.size(), 11);

        // std::vector<uint32_t> already exists!
        EXPECT_EQ(s3_root, 2);

        index_list.clear();
        read_sequence(s3_slot, inner_table, std::back_inserter(index_list));
        EXPECT_EQ(index_list, s3);
    }

    /* double*/

    {
        const auto s0 = std::vector<double> { 0., 1., 2., 3., 4., 5., 6., 7., 8., 9., 10., 11., 12., 13., 14., 15. };
        index_list.clear();
        encode_as_unsigned_integrals(s0, leaf_table, std::back_inserter(index_list));
        const auto s0_slot = insert_sequence(index_list, inner_table);
        const auto s0_root = root_table.insert(s0_slot);

        EXPECT_EQ(inner_table.size(), 11);
        EXPECT_EQ(leaf_table.size(), 16);

        // Created new state!
        EXPECT_EQ(s0_root, 0);

        index_list.clear();
        read_sequence(s0_slot, inner_table, std::back_inserter(index_list));
        double_list.clear();
        decode_from_unsigned_integrals(index_list, leaf_table, std::back_inserter(double_list));
        EXPECT_EQ(double_list, s0);
    }

    {
        const auto s1 = std::vector<double> { 0., 1., 2., 3., 4., 5., 6., 7., 8., 9., 10., 11., 12., 13., 14., 15., 16. };
        index_list.clear();
        encode_as_unsigned_integrals(s1, leaf_table, std::back_inserter(index_list));
        const auto s1_slot = insert_sequence(index_list, inner_table);
        const auto s1_root = root_table.insert(s1_slot);

        EXPECT_EQ(inner_table.size(), 11);
        EXPECT_EQ(leaf_table.size(), 17);

        // Created new state!
        EXPECT_EQ(s1_root, 1);

        index_list.clear();
        read_sequence(s1_slot, inner_table, std::back_inserter(index_list));
        double_list.clear();
        decode_from_unsigned_integrals(index_list, leaf_table, std::back_inserter(double_list));
        EXPECT_EQ(double_list, s1);
    }

    {
        const auto s2 = std::vector<double> { 0., 1., 2., 3., 4., 5., 6., 7., 8., 9., 10., 11., 12., 13., 14., 15., 16., 17. };
        index_list.clear();
        encode_as_unsigned_integrals(s2, leaf_table, std::back_inserter(index_list));
        const auto s2_slot = insert_sequence(index_list, inner_table);
        const auto s2_root = root_table.insert(s2_slot);

        EXPECT_EQ(inner_table.size(), 11);
        EXPECT_EQ(leaf_table.size(), 18);

        // Created new state!
        EXPECT_EQ(s2_root, 2);

        index_list.clear();
        read_sequence(s2_slot, inner_table, std::back_inserter(index_list));
        double_list.clear();
        decode_from_unsigned_integrals(index_list, leaf_table, std::back_inserter(double_list));
        EXPECT_EQ(double_list, s2);
    }

    {
        const auto s3 = std::vector<double> { 0., 1., 2., 3., 4., 5., 6., 7., 8., 9., 10., 11., 12., 13., 14., 15., 16., 17. };
        index_list.clear();
        encode_as_unsigned_integrals(s3, leaf_table, std::back_inserter(index_list));
        const auto s3_slot = insert_sequence(index_list, inner_table);
        const auto s3_root = root_table.insert(s3_slot);

        EXPECT_EQ(inner_table.size(), 11);
        EXPECT_EQ(leaf_table.size(), 18);

        // std::vector<double> already exists!
        EXPECT_EQ(s3_root, 2);

        index_list.clear();
        read_sequence(s3_slot, inner_table, std::back_inserter(index_list));
        double_list.clear();
        decode_from_unsigned_integrals(index_list, leaf_table, std::back_inserter(double_list));
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
        const auto s0_slot = insert_sequence(s0, inner_table);
        const auto s0_root = root_table.insert(s0_slot);

        EXPECT_EQ(inner_table.size(), 0);

        // Created new state!
        EXPECT_EQ(s0_root, 0);

        index_list.clear();
        read_sequence(s0_slot, inner_table, std::back_inserter(index_list));
        EXPECT_EQ(index_list, s0);
    }

    {
        const auto s1 = std::vector<uint32_t> { 0 };
        const auto s1_slot = insert_sequence(s1, inner_table);
        const auto s1_root = root_table.insert(s1_slot);

        EXPECT_EQ(inner_table.size(), 0);

        // Created new state!
        EXPECT_EQ(s1_root, 1);

        index_list.clear();
        read_sequence(s1_slot, inner_table, std::back_inserter(index_list));
        EXPECT_EQ(index_list, s1);
    }

    {
        const auto s0 = std::vector<double> {};
        index_list.clear();
        encode_as_unsigned_integrals(s0, leaf_table, std::back_inserter(index_list));
        const auto s0_slot = insert_sequence(index_list, inner_table);
        const auto s0_root = root_table.insert(s0_slot);

        EXPECT_EQ(inner_table.size(), 0);
        EXPECT_EQ(leaf_table.size(), 0);

        // Created new state!
        EXPECT_EQ(s0_root, 0);

        index_list.clear();
        read_sequence(s0_slot, inner_table, std::back_inserter(index_list));
        double_list.clear();
        decode_from_unsigned_integrals(index_list, leaf_table, std::back_inserter(double_list));
        EXPECT_EQ(double_list, s0);
    }

    {
        const auto s1 = std::vector<double> { 0 };
        index_list.clear();
        encode_as_unsigned_integrals(s1, leaf_table, std::back_inserter(index_list));
        const auto s1_slot = insert_sequence(index_list, inner_table);
        const auto s1_root = root_table.insert(s1_slot);

        EXPECT_EQ(inner_table.size(), 0);
        EXPECT_EQ(leaf_table.size(), 1);

        // Created new state!
        EXPECT_EQ(s1_root, 1);

        index_list.clear();
        read_sequence(s1_slot, inner_table, std::back_inserter(index_list));
        double_list.clear();
        decode_from_unsigned_integrals(index_list, leaf_table, std::back_inserter(double_list));
        EXPECT_EQ(double_list, s1);
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

    auto tmp_il = std::vector<uint32_t> {};

    for (size_t i = 0; i < ils.size(); ++i)
    {
        const auto& il = ils[i];
        const auto& dl = dls[i];

        auto ir = insert_sequence(il, inner_table);
        tmp_il.clear();
        encode_as_unsigned_integrals(dl, leaf_table, std::back_inserter(tmp_il));
        auto dr = insert_sequence(tmp_il, inner_table);

        irs.push_back(ir);
        drs.push_back(dr);

        /* Ensure newly inserted index sequence is readable*/
        out_il.clear();
        read_sequence(ir, inner_table, std::back_inserter(out_il));
        EXPECT_EQ(il, out_il);

        /* Ensure newly inserted double sequence is readable*/
        tmp_il.clear();
        read_sequence(dr, inner_table, std::back_inserter(tmp_il));
        out_dl.clear();
        decode_from_unsigned_integrals(tmp_il, leaf_table, std::back_inserter(out_dl));
        EXPECT_EQ(dl, out_dl);

        /* Ensure that all index sequences are readable after rehash. */
        for (size_t j = 0; j <= i; ++j)
        {
            const auto& il_2 = ils[j];
            const auto& ir_2 = irs[j];

            out_il.clear();
            read_sequence(ir_2, inner_table, std::back_inserter(out_il));
            EXPECT_EQ(il_2, out_il);
        }

        /* Ensure that all double sequences are readable after rehash. */
        for (size_t j = 0; j <= i; ++j)
        {
            const auto& dl_2 = dls[j];
            const auto& dr_2 = drs[j];

            tmp_il.clear();
            read_sequence(dr_2, inner_table, std::back_inserter(tmp_il));
            out_dl.clear();
            decode_from_unsigned_integrals(tmp_il, leaf_table, std::back_inserter(out_dl));
            EXPECT_EQ(dl_2, out_dl);
        }
    }
}

/**
 * Succinct
 */

TEST(VallaTests, SuccinctUintSwissTest)
{
    auto inner_table = SuccinctIndexedHashSet<Slot<uint32_t>, uint32_t>();
    auto leaf_table = IndexedHashSet<double, uint32_t>();

    auto index_root_table = SuccinctIndexedHashSet<Slot<uint32_t>, uint32_t>();
    auto index_list = std::vector<uint32_t>();
    auto double_root_table = SuccinctIndexedHashSet<Slot<uint32_t>, uint32_t>();
    auto double_list = std::vector<double>();

    /* uint32_t */
    {
        const auto s0 = std::vector<uint32_t> { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
        const auto s0_slot = insert_sequence(s0, inner_table);
        const auto s0_root = index_root_table.insert(s0_slot);

        EXPECT_EQ(inner_table.size(), 8);

        // Created new state!
        EXPECT_EQ(s0_root, 0);

        index_list.clear();
        read_sequence(s0_slot, inner_table, std::back_inserter(index_list));
        EXPECT_EQ(index_list, s0);
    }

    {
        const auto s1 = std::vector<uint32_t> { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
        const auto s1_slot = insert_sequence(s1, inner_table);
        const auto s1_root = index_root_table.insert(s1_slot);

        EXPECT_EQ(inner_table.size(), 9);

        // Created new state!
        EXPECT_EQ(s1_root, 1);

        index_list.clear();
        read_sequence(s1_slot, inner_table, std::back_inserter(index_list));
        EXPECT_EQ(index_list, s1);
    }

    {
        const auto s2 = std::vector<uint32_t> { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 };
        const auto s2_slot = insert_sequence(s2, inner_table);
        const auto s2_root = index_root_table.insert(s2_slot);

        EXPECT_EQ(inner_table.size(), 11);

        // Created new state!
        EXPECT_EQ(s2_root, 2);

        index_list.clear();
        read_sequence(s2_slot, inner_table, std::back_inserter(index_list));
        EXPECT_EQ(index_list, s2);
    }

    {
        const auto s3 = std::vector<uint32_t> { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 };
        const auto s3_slot = insert_sequence(s3, inner_table);
        const auto s3_root = index_root_table.insert(s3_slot);

        EXPECT_EQ(inner_table.size(), 11);

        // std::vector<uint32_t> already exists!
        EXPECT_EQ(s3_root, 2);

        index_list.clear();
        read_sequence(s3_slot, inner_table, std::back_inserter(index_list));
        EXPECT_EQ(index_list, s3);
    }

    /* double*/

    {
        const auto s0 = std::vector<double> { 0., 1., 2., 3., 4., 5., 6., 7., 8., 9., 10., 11., 12., 13., 14., 15. };
        index_list.clear();
        encode_as_unsigned_integrals(s0, leaf_table, std::back_inserter(index_list));
        const auto s0_slot = insert_sequence(index_list, inner_table);
        const auto s0_root = double_root_table.insert(s0_slot);

        EXPECT_EQ(inner_table.size(), 11);
        EXPECT_EQ(leaf_table.size(), 16);

        // Created new state!
        EXPECT_EQ(s0_root, 0);

        index_list.clear();
        read_sequence(s0_slot, inner_table, std::back_inserter(index_list));
        double_list.clear();
        decode_from_unsigned_integrals(index_list, leaf_table, std::back_inserter(double_list));
        EXPECT_EQ(double_list, s0);
    }

    {
        const auto s1 = std::vector<double> { 0., 1., 2., 3., 4., 5., 6., 7., 8., 9., 10., 11., 12., 13., 14., 15., 16. };
        index_list.clear();
        encode_as_unsigned_integrals(s1, leaf_table, std::back_inserter(index_list));
        const auto s1_slot = insert_sequence(index_list, inner_table);
        const auto s1_root = double_root_table.insert(s1_slot);

        EXPECT_EQ(inner_table.size(), 11);
        EXPECT_EQ(leaf_table.size(), 17);

        // Created new state!
        EXPECT_EQ(s1_root, 1);

        index_list.clear();
        read_sequence(s1_slot, inner_table, std::back_inserter(index_list));
        double_list.clear();
        decode_from_unsigned_integrals(index_list, leaf_table, std::back_inserter(double_list));
        EXPECT_EQ(double_list, s1);
    }

    {
        const auto s2 = std::vector<double> { 0., 1., 2., 3., 4., 5., 6., 7., 8., 9., 10., 11., 12., 13., 14., 15., 16., 17. };
        index_list.clear();
        encode_as_unsigned_integrals(s2, leaf_table, std::back_inserter(index_list));
        const auto s2_slot = insert_sequence(index_list, inner_table);
        const auto s2_root = double_root_table.insert(s2_slot);

        EXPECT_EQ(inner_table.size(), 11);
        EXPECT_EQ(leaf_table.size(), 18);

        // Created new state!
        EXPECT_EQ(s2_root, 2);

        index_list.clear();
        read_sequence(s2_slot, inner_table, std::back_inserter(index_list));
        double_list.clear();
        decode_from_unsigned_integrals(index_list, leaf_table, std::back_inserter(double_list));
        EXPECT_EQ(double_list, s2);
    }

    {
        const auto s3 = std::vector<double> { 0., 1., 2., 3., 4., 5., 6., 7., 8., 9., 10., 11., 12., 13., 14., 15., 16., 17. };
        index_list.clear();
        encode_as_unsigned_integrals(s3, leaf_table, std::back_inserter(index_list));
        const auto s3_slot = insert_sequence(index_list, inner_table);
        const auto s3_root = double_root_table.insert(s3_slot);

        EXPECT_EQ(inner_table.size(), 11);
        EXPECT_EQ(leaf_table.size(), 18);

        // std::vector<double> already exists!
        EXPECT_EQ(s3_root, 2);

        index_list.clear();
        read_sequence(s3_slot, inner_table, std::back_inserter(index_list));
        double_list.clear();
        decode_from_unsigned_integrals(index_list, leaf_table, std::back_inserter(double_list));
        EXPECT_EQ(double_list, s3);
    }
}

TEST(VallaTests, SuccinctUintSwissExhaustiveTest)
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

    auto inner_table = SuccinctIndexedHashSet<Slot<uint32_t>, uint32_t>();
    auto leaf_table = IndexedHashSet<double, uint32_t>();

    auto out_il = std::vector<uint32_t> {};
    auto out_dl = std::vector<double> {};

    auto irs = std::vector<Slot<uint32_t>> {};
    auto drs = std::vector<Slot<uint32_t>> {};

    auto tmp_il = std::vector<uint32_t> {};

    for (size_t i = 0; i < ils.size(); ++i)
    {
        const auto& il = ils[i];
        const auto& dl = dls[i];

        auto ir = insert_sequence(il, inner_table);
        tmp_il.clear();
        encode_as_unsigned_integrals(dl, leaf_table, std::back_inserter(tmp_il));
        auto dr = insert_sequence(tmp_il, inner_table);

        irs.push_back(ir);
        drs.push_back(dr);

        /* Ensure newly inserted index sequence is readable*/
        out_il.clear();
        read_sequence(ir, inner_table, std::back_inserter(out_il));
        EXPECT_EQ(il, out_il);

        /* Ensure newly inserted double sequence is readable*/
        tmp_il.clear();
        read_sequence(dr, inner_table, std::back_inserter(tmp_il));
        out_dl.clear();
        decode_from_unsigned_integrals(tmp_il, leaf_table, std::back_inserter(out_dl));
        EXPECT_EQ(dl, out_dl);

        /* Ensure that all index sequences are readable after rehash. */
        for (size_t j = 0; j <= i; ++j)
        {
            const auto& il_2 = ils[j];
            const auto& ir_2 = irs[j];

            out_il.clear();
            read_sequence(ir_2, inner_table, std::back_inserter(out_il));
            EXPECT_EQ(il_2, out_il);
        }

        /* Ensure that all double sequences are readable after rehash. */
        for (size_t j = 0; j <= i; ++j)
        {
            const auto& dl_2 = dls[j];
            const auto& dr_2 = drs[j];

            tmp_il.clear();
            read_sequence(dr_2, inner_table, std::back_inserter(tmp_il));
            out_dl.clear();
            decode_from_unsigned_integrals(tmp_il, leaf_table, std::back_inserter(out_dl));
            EXPECT_EQ(dl_2, out_dl);
        }
    }
}

}
