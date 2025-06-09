cmake_minimum_required(VERSION 3.21)
project(InstallCompactSparseHash)

include(ExternalProject)

message(STATUS "Dependency \"glog\"...")

# Attempt to find the installed glog library
find_package(glog QUIET PATHS ${CMAKE_INSTALL_PREFIX} NO_DEFAULT_PATH)

if (NOT glog_FOUND)


    list(APPEND CMAKE_ARGS
            -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
            -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
            -DCMAKE_PREFIX_PATH=${CMAKE_INSTALL_PREFIX}
            -DCMAKE_POSITION_INDEPENDENT_CODE=ON
    )

    message(STATUS "Preparing external project \"glog\" with args:")
    foreach(CMAKE_ARG ${CMAKE_ARGS})
        message(STATUS "-- ${CMAKE_ARG}")
    endforeach()
    ExternalProject_Add(
        glog
        GIT_REPOSITORY https://github.com/google/glog.git
        GIT_TAG v0.4.0
        PREFIX ${CMAKE_BINARY_DIR}/glog
        CMAKE_ARGS ${CMAKE_ARGS}
    )
else()
    message(STATUS "glog is already installed.")
endif()