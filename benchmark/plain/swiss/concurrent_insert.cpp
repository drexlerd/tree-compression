/*
 * Copyright (C) 2023 Dominik Drexler and Simon Stahlberg
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

#include <benchmark/benchmark.h>
#include <oneapi/tbb.h>
#include <valla/valla.hpp>

namespace valla::benchmarks
{

/// @brief In this benchmark, we evaluate the performance of accessing data in sequence
static void BM_PlainUintSwissConcurrentInsert(benchmark::State& state)
{
    const size_t state_num = static_cast<size_t>(state.range(0));    // number of states
    const size_t state_size = static_cast<size_t>(state.range(1));   // size of each state
    const size_t repetitions = static_cast<size_t>(state.range(2));  // reinsertions (for deduplication effect)

    std::mt19937 rng(42);  // fixed seed for reproducibility
    std::uniform_int_distribution<uint32_t> dist(0, 10'000);

    std::vector<std::vector<uint32_t>> all_states;
    all_states.reserve(state_num);

    // Generate sorted random states
    for (size_t i = 0; i < state_num; ++i)
    {
        std::vector<uint32_t> s(state_size);
        for (auto& v : s)
            v = dist(rng);

        std::sort(s.begin(), s.end());
        all_states.push_back(std::move(s));
    }

    // const int threads = 24;
    // tbb::global_control gc(tbb::global_control::max_allowed_parallelism, threads);

    for (auto _ : state)
    {
        IndexedHashSet<Slot<uint32_t>, uint32_t> tree_table;
        IndexedHashSet<Slot<uint32_t>, uint32_t> root_table;

        for (size_t rep = 0; rep < repetitions; ++rep)
        {
            tbb::parallel_for(tbb::blocked_range<size_t>(0, all_states.size(), /*grain*/ 4096),
                              [&](const tbb::blocked_range<size_t>& r)
                              {
                                  for (size_t k = r.begin(); k != r.end(); ++k)
                                  {
                                      assert(k < all_states.size());
                                      benchmark::DoNotOptimize(root_table.insert(insert_sequence(all_states[k], tree_table)));
                                  }
                              });
        }

        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(state.iterations() * state_num * repetitions);
}

}

BENCHMARK(valla::benchmarks::BM_PlainUintSwissConcurrentInsert)->Args({ 100, 50, 1 });     // 100 unique states, 50 entries each, no repetition
BENCHMARK(valla::benchmarks::BM_PlainUintSwissConcurrentInsert)->Args({ 100, 50, 5 });     // same but with 5 insertions each
BENCHMARK(valla::benchmarks::BM_PlainUintSwissConcurrentInsert)->Args({ 1000, 100, 1 });   // larger states
BENCHMARK(valla::benchmarks::BM_PlainUintSwissConcurrentInsert)->Args({ 1000, 100, 5 });   // reinsertions (stress hashing)
BENCHMARK(valla::benchmarks::BM_PlainUintSwissConcurrentInsert)->Args({ 10000, 200, 1 });  // large-scale
BENCHMARK(valla::benchmarks::BM_PlainUintSwissConcurrentInsert)->Args({ 10000, 200, 5 });  // heavy deduplication pressure

BENCHMARK_MAIN();
