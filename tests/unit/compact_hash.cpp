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
TEST(VallaTests, CompactHashTest)
{
    auto w = uint8_t(4);

    auto hash = CompactHash<uint64_t> {};
    const auto x0 = uint64_t(0);
    const auto h0 = hash.hash(x0, w);
    std::cout << 0 << " " << h0 << std::endl;
    EXPECT_LE(std::bit_width(h0), 4);
    EXPECT_EQ(x0, hash.invert_hash(h0, w));

    const auto x1 = uint64_t(1);
    const auto h1 = hash.hash(x1, 4);
    std::cout << 0 << " " << h1 << std::endl;
    EXPECT_LE(std::bit_width(h1), 4);
    EXPECT_EQ(x1, hash.invert_hash(h1, w));
}

TEST(VallaTests, SimulateCompactHashTest)
{
    const auto w = uint8_t(10);
    const auto b = size_t(std::pow(2, w));
    const auto m = uint64_t(128);

    auto hash = CompactHash<uint64_t> {};

    std::mt19937 rng(42);  // fixed seed for reproducibility
    std::uniform_int_distribution<uint32_t> index_dist(0, std::pow(2, 2 * w));

    std::vector<size_t> buckets(b + 1, 0);
    size_t count = 0;

    while (true)
    {
        const auto i = hash.hash(index_dist(rng), w) % b;

        if (++buckets[i] == m)
            break;

        ++count;
    }

    const double load = double(count) / (double(m) * double(b));
    const double expected = std::max(0.0, 1.0 - std::sqrt((2.0 * std::log(double(b))) / double(m)));

    EXPECT_FLOAT_EQ(load, 0.735283);
    EXPECT_FLOAT_EQ(expected, 0.670904);
}

TEST(VallaTests, SimulateCompactHashOverflowTest)
{
    const auto w = uint8_t(10);
    const auto b = size_t(std::pow(2, w));
    const auto m = uint64_t(64);

    auto hash = CompactHash<uint64_t> {};

    std::mt19937 rng(42);  // fixed seed for reproducibility
    std::uniform_int_distribution<uint32_t> index_dist(0, std::pow(2, 2 * w));

    std::vector<size_t> buckets(b + 1, 0);
    size_t count = 0;
    std::vector<bool> overflow(b, false);
    while (true)
    {
        const auto i = hash.hash(index_dist(rng), w) % b;
        if (buckets[i] == m)
        {
            overflow[i] = true;
            if (++buckets[b] == m)
                break;
        }
        else
        {
            ++buckets[i];
        }

        ++count;
    }

    const auto overflowed = double(std::count(overflow.begin(), overflow.end(), true)) / b;
    const double load = double(count) / (double(m) * double(b));

    EXPECT_FLOAT_EQ(overflowed, 0.026367188);
    EXPECT_FLOAT_EQ(load, 0.837616);
}

}
