# Dynamic Tree Databases

A Tree database is a data structure D for representing sequences of elements over a finite alphabet with two operations: 1) insert(z) inserts a sequence z of length k into D and returns a handle i, and 2) lookup(i) returns the sequence z corresponding to the handle i. Tree databases encode a sequence z = z1, z2, ..., zk as perfectly balanced binary trees, where subtrees may be shared among trees. The best-case compression approaches the information-theoretic lower bound of one word, assuming that one variable changes on average from one sequence to the next. Tree databases have successfully been applied to compressing the set of generated states during explicit search in model checking and automated planning. Tree databases were initially proposed as statically sized data structures, requiring the user to estimate the amount of memory needed.

This repository contains a C++ library for dynamic tree databases. Dynamic tree databases allocate memory dynamically as needed. Therefore, they rely on user estimates of the required memory, but have the same desired properties. The available implementations are based on modern Swiss tables and fixed word sizes. However, more memory-efficient implementations based on succinct data structures and compact hashing are possible, but are also significantly more challenging to implement efficiently at runtime.

## API

The following example illustrates 

```cpp
#include <valla/valla.hpp>

/* Common setup */
// Deduplicate root nodes, i.e., pair of uint32_t representing pointer to inner node and sequence length, by bijectively mapping it to uint32_t
auto root_table = IndexedHashSet<Slot<uint32_t>, uint32_t>();
// Deduplicate inner nodes, i.e., pair of uint32_t representing pointer to left and right child node, by bijectively mapping it to uint32_t
auto inner_table = IndexedHashSet<Slot<uint32_t>, uint32_t>();
// Deduplicate doubles by bijectively mapping it to uint32_t
auto leaf_table = IndexedHashSet<double, uint32_t>();

/* Sequence of uint32_t (special case) */
{
    // Create a sequence over uint32_t
    const auto z = std::vector<uint32_t> { 0, 1, 3, 4, 7, 10 };
    // Insert the sequence that satisfies the std::input_range concept
    const auto z_root = insert(z, inner_table);
    // Read the sequence from the given handle z_root.
    auto z_out = std::vector<uint32_t>{};
    read_state(z_root, inner_table, z_out);
    // Iterate over the sequence from the given handle z_root.
    for (const auto element : range(z_root, inner_table)) { }
}

/* Sequence of double (general case) */
{
    // Create a sequence over double (requires the leaf table for deduplication at the leafs)
    const auto z = std::vector<double> { 0, 1, 3, 4, 7, 10 };
    // Insert the sequence that satisfies the std::input_range concept
    const auto z_root = insert(z, inner_table, leaf_table);
    // Read the sequence from the given handle z_root.
    auto z_out = std::vector<double>{};
    read_state(z_root, inner_table, leaf_table, z_out);
    // Iterate over the sequence from the given handle z_root.
    for (const auto element : range(z_root, inner_table, leaf_table)) { }
}
```

## Getting Started

### Installing the Dependencies

1. Configure the dependencies CMake project with the desired installation path:
```console
cmake -S dependencies -B dependencies/build -DCMAKE_INSTALL_PREFIX=dependencies/installs -DCMAKE_PREFIX_PATH=$PWD/dependencies/installs
```
2. Download, build, and install all dependencies:
```console
cmake --build dependencies/build -j$(nproc)
```

### Building the Project

1. Configure

```console
cmake -S . -B build -DCMAKE_PREFIX_PATH=${PWD}/dependencies/installs
```

2. Build
```console
cmake --build build -j$(nproc)
```

3. (Optional) Install the project from the build directory to the desired installation `prefix` directory:
```console
cmake --install build --prefix=<path/to/installation-directory>
```

### Testing 

The testing framework depends on GoogleTest and requires the additional compile flag `-DBUILD_TESTS=ON` to be set in the cmake configure step. The tests can be run by executing the following commands from the root.

```console
./build/tests/unit/dtdb_h
./build/tests/unit/dtdb_s
```

### Benchmarking

The benchmark framework depends on GoogleBenchmark and requires the additional compile flag `-DBUILD_PROFILING=ON` to be set in the cmake configure step. The benchmarks can be run by executing the following commands from the root.

```console
./build/benchmark/plain_swiss_insert
./build/benchmark/plain_swiss_read
./build/benchmark/plain_swiss_iterator
./build/benchmark/plain_hash_id_map_insert
./build/benchmark/plain_hash_id_map_read
./build/benchmark/plain_hash_id_map_iterator
```

The results from the GitHub action can be viewed [here](https://drexlerd.github.io/tree-compression/dev/bench/).
