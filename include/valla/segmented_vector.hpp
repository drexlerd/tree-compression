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
 *<
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef VALLA_INCLUDE_SEGMENTED_VECTOR_HPP_
#define VALLA_INCLUDE_SEGMENTED_VECTOR_HPP_

#include <atomic>
#include <bit>
#include <deque>
#include <mutex>
#include <vector>

namespace valla
{
template<typename T>
class SegmentedVector
{
private:
    static constexpr std::size_t kSegmentBytes = 512ull * 1024ull;  // 512 KiB
    static constexpr std::size_t kElemsPerSeg = (kSegmentBytes / sizeof(T));

    static_assert(kElemsPerSeg > 0, "kElemsPerSeg must be >= 1");
    static_assert((kSegmentBytes & (kSegmentBytes - 1)) == 0, "kSegmentBytes must be a power of 2");

    std::vector<std::vector<T>> m_segments;
    std::mutex m_write_mutex;
    size_t m_offset;
    size_t m_capacity;
    std::atomic<size_t> m_size;

    static std::size_t get_index(std::size_t pos) { return pos / kElemsPerSeg; }
    static std::size_t get_offset(std::size_t pos) { return pos % kElemsPerSeg; }

    void resize_to_fit()
    {
        const auto remaining_entries = kElemsPerSeg - m_offset;

        if (remaining_entries == 0)
        {
            m_segments.emplace_back();
            m_segments.back().reserve(kElemsPerSeg);
            m_offset = 0;
            m_capacity += kElemsPerSeg;
        }
    }

public:
    SegmentedVector() : m_segments(), m_offset(kElemsPerSeg), m_capacity(0), m_size(0) {}
    SegmentedVector(const SegmentedVector& other) = delete;
    SegmentedVector& operator=(const SegmentedVector& other) = delete;
    SegmentedVector(SegmentedVector&& other) = default;
    SegmentedVector& operator=(SegmentedVector&& other) = default;

    size_t push_back(const T& element)
    {
        std::lock_guard<std::mutex> lk(m_write_mutex);

        resize_to_fit();

        const std::size_t idx = m_size.load(std::memory_order_relaxed);

        m_segments.back().push_back(element);

        ++m_offset;
        m_size.fetch_add(1, std::memory_order_release);

        return idx;
    }
    size_t push_back(T&& element)
    {
        std::lock_guard<std::mutex> lk(m_write_mutex);

        resize_to_fit();

        const std::size_t idx = m_size.load(std::memory_order_relaxed);

        m_segments.back().push_back(std::move(element));

        ++m_offset;
        m_size.fetch_add(1, std::memory_order_release);

        return idx;
    }

    void pop_back()
    {
        std::lock_guard<std::mutex> lk(m_write_mutex);

        m_segments.back().pop_back();

        --m_offset;
        m_size.fetch_sub(1, std::memory_order_release);
    }

    const T& operator[](size_t pos) const
    {
        const auto index = get_index(pos);
        const auto offset = get_offset(pos);

        assert(index < m_segments.size() && offset < m_segments[index].size());

        return m_segments[index][offset];
    }
    T& operator[](size_t pos)
    {
        const auto index = get_index(pos);
        const auto offset = get_offset(pos);

        assert(index < m_segments.size() && offset < m_segments[index].size());

        return m_segments[index][offset];
    }

    const T& at(size_t pos) const
    {
        const auto n = m_size.load(std::memory_order_acquire);
        if (pos >= n)
            throw std::out_of_range("SegmentedVector::at");

        const auto index = get_index(pos);
        const auto offset = get_offset(pos);

        return m_segments.at(index).at(offset);
    }
    T& at(size_t pos)
    {
        const auto n = m_size.load(std::memory_order_acquire);
        if (pos >= n)
            throw std::out_of_range("SegmentedVector::at");

        const auto index = get_index(pos);
        const auto offset = get_offset(pos);

        return m_segments.at(index).at(offset);
    }

    const T& back() const { return m_segments.back().back(); }
    T& back() { return m_segments.back().back(); }

    size_t capacity() const { return m_capacity; }
    size_t size() const { return m_size.load(std::memory_order_acquire); }
};
}

#endif
