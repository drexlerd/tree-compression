# Dynamic Tree Databases

This repository contains a C++ libary for Dynamic Swiss Tree Databases (DTDB-S) and Dynamic HashID Tree Databases (DTDB-H). 

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
./build/tests/unit/plain_swiss
./build/tests/unit/plain_hash_id_map
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
