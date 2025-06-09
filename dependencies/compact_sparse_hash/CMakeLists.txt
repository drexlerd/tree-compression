cmake_minimum_required(VERSION 3.21)
project(InstallCompactSparseHash)

include(ExternalProject)

message(STATUS "Dependency \"compact_sparse_hash\"...")

# Attempt to find the installed compact_sparse_hash library
find_package(compact_sparse_hash QUIET PATHS ${CMAKE_INSTALL_PREFIX} NO_DEFAULT_PATH)

if (NOT compact_sparse_hash_FOUND)


    list(APPEND CMAKE_ARGS
            -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
            -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
            -DCMAKE_PREFIX_PATH=${CMAKE_INSTALL_PREFIX}
            -DCMAKE_POSITION_INDEPENDENT_CODE=ON
    )

    message(STATUS "Preparing external project \"compact_sparse_hash\" with args:")
    foreach(CMAKE_ARG ${CMAKE_ARGS})
        message(STATUS "-- ${CMAKE_ARG}")
    endforeach()
    ExternalProject_Add(
        compact_sparse_hash
        GIT_REPOSITORY https://github.com/tudocomp/compact_sparse_hash.git
        GIT_TAG 3306b1ddb9c6e8d0c0fe8cda05fa26e3de32ca2c
        GIT_SUBMODULES_RECURSE ON
        PREFIX ${CMAKE_BINARY_DIR}/compact_sparse_hash
        CMAKE_ARGS ${CMAKE_ARGS}
    )
else()
    message(STATUS "compact_sparse_hash is already installed.")
endif()