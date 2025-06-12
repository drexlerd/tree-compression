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
#include <valla/canonical_delta_tree_compression.hpp>

namespace valla::tests
{

TEST(VallaTests, CanonicalDeltaTreeCompressionTest)
{
    auto tree_table = IndexedHashSet();
    auto pool = BitsetPool();
    auto repo = BitsetRepository(pool);
    auto root_table = RootIndexedHashSet(repo);

    auto tmp_state = State();

    {
        const auto s0 = State { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
        const auto [s0_result, success] = root_table.insert(canonical_delta::insert(s0, tree_table, pool, repo));
        const auto& s0_root = s0_result->first;
        const auto& s0_idx = s0_result->second;

        EXPECT_EQ(tree_table.size(), 2);
        EXPECT_EQ(root_table.size(), 2);

        // Created new state!
        EXPECT_EQ(s0_idx, 1);

        canonical_delta::read_state(s0_root, tree_table, tmp_state);
        EXPECT_EQ(tmp_state, s0);
    }

    {
        const auto s1 = State { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
        const auto [s1_result, success] = root_table.insert(canonical_delta::insert(s1, tree_table, pool, repo));
        const auto& s1_root = s1_result->first;
        const auto& s1_idx = s1_result->second;

        EXPECT_EQ(tree_table.size(), 2);
        EXPECT_EQ(root_table.size(), 3);

        // Created new state!
        EXPECT_EQ(s1_idx, 2);

        canonical_delta::read_state(s1_root, tree_table, tmp_state);
        EXPECT_EQ(tmp_state, s1);
    }

    {
        const auto s2 = State { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 };
        const auto [s2_result, success] = root_table.insert(canonical_delta::insert(s2, tree_table, pool, repo));
        const auto& s2_root = s2_result->first;
        const auto& s2_idx = s2_result->second;

        EXPECT_EQ(tree_table.size(), 2);
        EXPECT_EQ(root_table.size(), 4);

        // Created new state!
        EXPECT_EQ(s2_idx, 3);

        canonical_delta::read_state(s2_root, tree_table, tmp_state);
        EXPECT_EQ(tmp_state, s2);
    }

    {
        const auto s3 = State { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 };
        const auto [s3_result, success] = root_table.insert(canonical_delta::insert(s3, tree_table, pool, repo));
        const auto& s3_root = s3_result->first;
        const auto& s3_idx = s3_result->second;

        EXPECT_EQ(tree_table.size(), 2);
        EXPECT_EQ(root_table.size(), 4);

        // State already exists!
        EXPECT_EQ(s3_idx, 3);

        canonical_delta::read_state(s3_root, tree_table, tmp_state);
        EXPECT_EQ(tmp_state, s3);
    }
}

TEST(VallaTests, CanonicalDeltaTreeCompression2Test)
{
    auto tree_table = IndexedHashSet();
    auto pool = BitsetPool();
    auto repo = BitsetRepository(pool);
    auto root_table = RootIndexedHashSet(repo);

    auto tmp_state = State();

    {
        const auto s0 = State { 0, 2, 4, 6, 8, 10, 12, 14 };
        const auto [s0_result, success] = root_table.insert(canonical_delta::insert(s0, tree_table, pool, repo));
        const auto& s0_root = s0_result->first;
        const auto& s0_idx = s0_result->second;

        EXPECT_EQ(tree_table.size(), 5);
        EXPECT_EQ(root_table.size(), 2);

        // Created new state!
        EXPECT_EQ(s0_idx, 1);

        canonical_delta::read_state(s0_root, tree_table, tmp_state);
        EXPECT_EQ(tmp_state, s0);

        tmp_state.clear();
        EXPECT_EQ(s0, State(canonical_delta::begin(s0_root, tree_table), canonical_delta::end()));
    }

    {
        const auto s1 = State { 3, 4, 8, 10, 12, 14 };
        const auto [s1_result, success] = root_table.insert(canonical_delta::insert(s1, tree_table, pool, repo));
        const auto& s1_root = s1_result->first;
        const auto& s1_idx = s1_result->second;

        EXPECT_EQ(tree_table.size(), 9);
        EXPECT_EQ(root_table.size(), 3);

        // Created new state!
        EXPECT_EQ(s1_idx, 2);

        canonical_delta::read_state(s1_root, tree_table, tmp_state);
        EXPECT_EQ(tmp_state, s1);

        tmp_state.clear();
        EXPECT_EQ(s1, State(canonical_delta::begin(s1_root, tree_table), canonical_delta::end()));
    }
}

TEST(VallaTests, CanonicalDeltaTreeCompressionRandomTest)
{
    auto tree_table = IndexedHashSet();
    auto pool = BitsetPool();
    auto repo = BitsetRepository(pool);
    auto root_table = RootIndexedHashSet(repo);

    std::mt19937 rng(42);  // fixed seed for reproducibility
    std::uniform_int_distribution<Index> dist(0, 10'000);
    const size_t state_num = 1000;                                     // number of states
    const std::vector<size_t> state_sizes = { 10, 20, 50, 100, 200 };  // size of each state

    std::vector<State> all_states;
    all_states.reserve(state_num);

    // Generate sorted random states
    for (size_t i = 0; i < state_num; ++i)
    {
        for (const auto state_size : state_sizes)
        {
            State s(state_size);
            for (auto& v : s)
                v = dist(rng);

            s.erase(std::unique(s.begin(), s.end()), s.end());
            std::sort(s.begin(), s.end());
            all_states.push_back(std::move(s));
        }
    }

    auto tmp_state = State();

    {
        for (const auto& s : all_states)
        {
            auto [s_result, success] = root_table.insert(canonical_delta::insert(s, tree_table, pool, repo));
            const auto& s_root = s_result->first;
            const auto& s_idx = s_result->second;

            canonical_delta::read_state(s_root, tree_table, tmp_state);
            EXPECT_EQ(tmp_state, s);
            assert(tmp_state == s);

            tmp_state.clear();
            EXPECT_EQ(s, State(canonical_delta::begin(s_root, tree_table), canonical_delta::end()));
            assert(s == State(canonical_delta::begin(s_root, tree_table), canonical_delta::end()));
        }
    }
}

TEST(VallaTests, CanonicalDeltaTreeCompressionExhaustiveTest)
{
    auto tree_table = IndexedHashSet();
    auto pool = BitsetPool();
    auto repo = BitsetRepository(pool);
    auto root_table = RootIndexedHashSet(repo);

    auto tmp_state = State();

    const size_t num_atoms = 10;  // number of atoms

    std::vector<State> all_states;
    for (Index a = 0; a < num_atoms; ++a)
        for (Index b = a; b < num_atoms; ++b)
            for (Index c = b; c < num_atoms; ++c)
                for (Index d = c; d < num_atoms; ++d)
                    for (Index e = d; e < num_atoms; ++e)
                        all_states.push_back(State { a, b, c, d, e });

    {
        for (const auto& s : all_states)
        {
            auto [s_result, success] = root_table.insert(canonical_delta::insert(s, tree_table, pool, repo));
            const auto& s_root = s_result->first;
            const auto& s_idx = s_result->second;

            canonical_delta::read_state(s_root, tree_table, tmp_state);
            EXPECT_EQ(tmp_state, s);
            assert(tmp_state == s);

            tmp_state.clear();
            EXPECT_EQ(s, State(canonical_delta::begin(s_root, tree_table), canonical_delta::end()));
        }
    }
}

TEST(VallaTests, CanonicalDeltaTreeCompressionEdgeCasesTest)
{
    auto tree_table = IndexedHashSet();
    auto pool = BitsetPool();
    auto repo = BitsetRepository(pool);
    auto root_table = RootIndexedHashSet(repo);

    auto tmp_state = State();

    {
        const auto s0 = State {};
        const auto [s0_result, success] = root_table.insert(canonical_delta::insert(s0, tree_table, pool, repo));
        const auto& s0_root = s0_result->first;
        const auto& s0_idx = s0_result->second;

        EXPECT_EQ(tree_table.size(), 0);
        EXPECT_EQ(root_table.size(), 1);

        // Created new state!
        EXPECT_EQ(s0_idx, 0);

        canonical_delta::read_state(s0_root, tree_table, tmp_state);
        EXPECT_EQ(tmp_state, s0);
    }

    {
        const auto s1 = State { 0 };
        const auto [s1_result, success] = root_table.insert(canonical_delta::insert(s1, tree_table, pool, repo));
        const auto& s1_root = s1_result->first;
        const auto& s1_idx = s1_result->second;

        EXPECT_EQ(tree_table.size(), 0);
        EXPECT_EQ(root_table.size(), 2);

        // Created new state!
        EXPECT_EQ(s1_idx, 1);

        canonical_delta::read_state(s1_root, tree_table, tmp_state);
        EXPECT_EQ(tmp_state, s1);
    }
}

TEST(VallaTests, CanonicalDeltaTreeCompressionIteratorTest)
{
    auto tree_table = IndexedHashSet();
    auto pool = BitsetPool();
    auto repo = BitsetRepository(pool);
    auto root_table = RootIndexedHashSet(repo);

    auto tmp_state = State();

    {
        const auto s0 = State { 1, 2, 4, 5, 6 };
        const auto [s0_result, success] = root_table.insert(canonical_delta::insert(s0, tree_table, pool, repo));
        const auto& s0_root = s0_result->first;
        const auto& s0_idx = s0_result->second;

        EXPECT_EQ(s0, State(canonical_delta::begin(s0_root, tree_table), canonical_delta::end()));
    }

    {
        const auto s0 = State {};
        EXPECT_EQ(s0, State(canonical_delta::begin(root_table.get_empty_root(), tree_table), canonical_delta::end()));
    }
}
}
