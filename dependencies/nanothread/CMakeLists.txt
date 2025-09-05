cmake_minimum_required(VERSION 3.21)
project(InstallNanothread)

include(ExternalProject)

message(STATUS "Dependency \"nanothread\"...")

ExternalProject_Add(
    nanothread
    GIT_REPOSITORY https://github.com/mitsuba-renderer/nanothread.git
    GIT_TAG 244a8d837d34a4ec7500fb3e3444993b7dcf6b02
    
    # header-only -> no configure/build
    CONFIGURE_COMMAND ""
    BUILD_COMMAND     ""

    # where we want headers copied to
    INSTALL_DIR       ${CMAKE_INSTALL_PREFIX}

    # do the copy as the "install" step
    INSTALL_COMMAND
        ${CMAKE_COMMAND} -E make_directory <INSTALL_DIR>/include
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                <SOURCE_DIR>/include
                <INSTALL_DIR>/include

    UPDATE_DISCONNECTED 1
)