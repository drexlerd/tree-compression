# Tree-Compression

This repository contains implementations of two variants of tree compression: plain and delta. The plain tree compression inserts values of an input sequence as given, while the delta tree compression inserts the differences between adjacent values. Both implementations assume sorted input sequences, which often results in better sharing capabilities, and therefore, lower memory footprint.

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

### Benchmarking

The benchmark framework depends on GoogleBenchmark and requires the additional compile flag `-DBUILD_PROFILING=ON` to be set in the cmake configure step. The results from the GitHub action can be viewed [here](https://drexlerd.github.io/tree-compression/dev/bench/).
