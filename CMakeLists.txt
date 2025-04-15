cmake_minimum_required(VERSION 3.21)

##############################################################
# Language setup
##############################################################

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

##############################################################
# Establish project
##############################################################

project(valla VERSION 0.1 LANGUAGES C CXX)

# Compilation flags, some configuration-specific
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra -pedantic -fPIC")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -fomit-frame-pointer")
set(CMAKE_CXX_FLAGS_DEBUG "-O3 -DDEBUG")

# Set a default build type if none was specified
set(default_build_type "Debug")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to '${default_build_type}', as none was specified.")
    set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
        STRING "Choose the type of build." FORCE)
endif()

option(ENABLE_TESTING "Enables compilation of tests." OFF)
if (ENABLE_TESTING)
    message("Building tests enabled.")
else()
    message("Building tests disabled.")
endif()

##############################################################
# Dependency Handling
##############################################################

find_package(fmt REQUIRED PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)
if(fmt_FOUND)
  include_directories(${fmt_INCLUDE_DIRS})
  message(STATUS "Found fmt: ${fmt_DIR} (found version ${fmt_VERSION})")
endif()


find_package(absl CONFIG REQUIRED PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)
if(absl_FOUND)
  include_directories(${absl_INCLUDE_DIRS})
  message(STATUS "Found absl: ${absl_DIR} (found version ${absl_VERSION})")
endif()

##############################################################
# Add library and executable targets
##############################################################

add_subdirectory(src)

add_subdirectory(tests)





###########
# Install #
###########

# Install header files
install(DIRECTORY "${PROJECT_SOURCE_DIR}/include/valla"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")


###########
# Exports #
###########

# https://cmake.org/cmake/help/latest/guide/importing-exporting/index.html

include(CMakePackageConfigHelpers)

# Generate the version file for the config file
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/vallaConfigVersion.cmake"
    VERSION ${valla_VERSION}
    COMPATIBILITY ExactVersion
)

# Create config file
# https://cmake.org/cmake/help/book/mastering-cmake/cmake/Help/guide/importing-exporting/index.html
# https://cmake.org/cmake/help/latest/module/CMakePackageConfigHelpers.html#generating-a-package-configuration-file
configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/vallaConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/valla"
    NO_CHECK_REQUIRED_COMPONENTS_MACRO
)
