# Tree-Compression

This repository aims to be a playground around tree compression methods.

## Getting Started

1. Configure

```console
cmake -S . -B build
```

2. Build
```console
cmake --build build -j$(nproc)
```

3. Run test

```console
./build/tests/valla_tests
```

## TODOs

- add automated performance benchmarking with some artificial benchmarks to compare and visualize upon commit.
- add implementation of clearly table: https://ieeexplore.ieee.org/document/1676499
