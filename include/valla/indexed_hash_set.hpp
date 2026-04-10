/*
 * Copyright (C) 2025 Dominik Drexler
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

#ifndef VALLA_INCLUDE_INDEXED_HASH_SET_HPP_
#define VALLA_INCLUDE_INDEXED_HASH_SET_HPP_

#include "valla/equal_to.hpp"
#include "valla/hash.hpp"
#include "valla/utils.hpp"

#include <concepts>
#include <cstddef>
#include <gtl/phmap.hpp>
#include <memory>
#include <vector>

namespace valla
{

template<typename T, std::unsigned_integral I, typename H = Hash<T>, typename E = EqualTo<T>>
class IndexedHashSet
{
public:
    using value_type = T;
    using index_type = I;

private:
    class IndexableHash;
    class IndexableEqualTo;

    using VectorType = std::vector<T>;

public:
    IndexedHashSet() : m_storage(std::make_unique<VectorType>()), m_set(0, IndexableHash(*m_storage), IndexableEqualTo(*m_storage)) {}

    // Moveable but not copieable
    IndexedHashSet(const IndexedHashSet& other) = delete;
    IndexedHashSet& operator=(const IndexedHashSet& other) = delete;
    IndexedHashSet(IndexedHashSet&& other) : m_storage(std::move(other.m_storage)), m_set(std::move(other.m_set)) {}
    IndexedHashSet& operator=(IndexedHashSet&& other)
    {
        if (this != &other)
        {
            m_storage = std::move(other.m_storage);
            m_set = std::move(other.m_set);
        }
        return *this;
    }

    /// @brief Insert a slot uniquely and return its index.
    /// @param slot
    /// @return
    I insert(T slot)
    {
        if (auto it = m_set.find(slot); it != m_set.end())
            return *it;

        assert(m_storage->size() <= static_cast<size_t>(std::numeric_limits<I>::max())
               && "IndexedHashSet: Index overflow! The maximum number of slots reached.");

        m_storage->push_back(std::move(slot));
        const I index = static_cast<I>(m_storage->size() - 1);
        m_set.emplace(index);

        return index;
    }

    /// @brief Lookup the slot of the given index.
    /// Thread-safe if m_storage is a segmented vector.
    /// @param index
    /// @return
    T lookup(I index) const
    {
        assert(index < m_storage->size() && "Index out of bounds");

        return m_storage->operator[](index);
    }

    size_t size() const { return m_storage->size(); }

    size_t memory_usage() const
    {
        size_t usage = 0;
        usage += m_storage->capacity() * sizeof(T);
        usage += m_set.capacity() * (sizeof(I) + sizeof(gtl::priv::ctrl_t));
        return usage;
    }

private:
    struct IndexableHash
    {
        using is_transparent = void;

        const VectorType* vec;
        H hash;

        IndexableHash() : vec(nullptr), hash() {}
        explicit IndexableHash(const VectorType& vec) : vec(&vec), hash() {}

        size_t operator()(I el) const
        {
            assert(vec);
            assert(is_within_bounds(*vec, el));
            return hash(vec->operator[](el));
        }
        size_t operator()(const T& el) const { return hash(el); }
    };

    struct IndexableEqualTo
    {
        using is_transparent = void;

        const VectorType* vec;
        E equal_to;

        IndexableEqualTo() : vec(nullptr), equal_to() {}
        explicit IndexableEqualTo(const VectorType& vec) : vec(&vec), equal_to() {}

        bool operator()(I lhs, I rhs) const
        {
            assert(vec);
            assert(is_within_bounds(*vec, lhs));
            assert(is_within_bounds(*vec, rhs));
            return equal_to(vec->operator[](lhs), vec->operator[](rhs));
        }
        bool operator()(const T& lhs, I rhs) const
        {
            assert(vec);
            assert(is_within_bounds(*vec, rhs));
            return equal_to(lhs, (*vec)[rhs]);
        }
        bool operator()(I lhs, const T& rhs) const
        {
            assert(vec);
            assert(is_within_bounds(*vec, lhs));
            return equal_to((*vec)[lhs], rhs);
        }
        bool operator()(const T& lhs, const T& rhs) const { return equal_to(lhs, rhs); }
    };

private:
    std::unique_ptr<VectorType> m_storage;
    gtl::flat_hash_set<I, IndexableHash, IndexableEqualTo> m_set;
};

}

#endif
