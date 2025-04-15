# Tree-Compression

This repository aims to be a playground around tree compression methods.

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

## Available State Compression Methods

1. Chain of Perfectly Balanced Binary Trees (CPBBT)

The following table shows the properties of our implementations.

| Configuration     |     Variable-length | Multi-core      |
| :---------------- | ------------------: | --------------: | 
| CPBBT             |                 YES |              NO |

### Benchmarking

The benchmark framework depends on GoogleBenchmark and requires the additional compile flag `-DBUILD_PROFILING=ON` to be set in the cmake configure step. The results from the GitHub action can be viewed (here)[https://drexlerd.github.io/tree-compression/dev/bench/].


## TODOs

- add automated performance benchmarking with some artificial benchmarks to compare and visualize upon commit.
- add implementation of clearly table: https://ieeexplore.ieee.org/document/1676499
