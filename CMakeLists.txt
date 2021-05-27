cmake_minimum_required(VERSION 3.16)

if (NOT DEFINED FUN_TIMES_BOOST_VERSION)
    set(FUN_TIMES_BOOST_VERSION "1.67.0" CACHE STRING "Boost Version")
endif ()
set(boost_components system thread chrono date_time atomic)

include(FetchContent)
include(cmake/RequireBoost.cmake)
include(cmake/BuildCMakeContent.cmake)

project(beast_fun_times)

option(ENABLE_TESTING "" ON)

if (NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17)
endif ()

list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_BINARY_DIR}/dependencies/install)
if (NOT DEFINED BOOST_ROOT)
    #    RequireBoost(PREFIX ${CMAKE_CURRENT_BINARY_DIR}/dependencies/install VERSION 1.73.0 COMPONENTS system)
    list(APPEND boost_components container)
    RequireBoost(PREFIX "${CMAKE_CURRENT_BINARY_DIR}/dependencies/install/boost-${FUN_TIMES_BOOST_VERSION}" VERSION "${FUN_TIMES_BOOST_VERSION}"
            COMPONENTS
            ${boost_components})
else()
    message(STATUS "[dependencies] BOOST_ROOT=${BOOST_ROOT}")
endif ()

if (ENABLE_TESTING)
    FetchContent_Declare(catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2.git
            GIT_TAG v2.12.1)
    BuildCmakeContent(catch2 Catch2 CMAKE_ARGS
            "-DCATCH_BUILD_TESTING=OFF"
            "-DCATCH_INSTALL_DOCS=OFF"
            "-DCATCH_INSTALL_HELPERS=OFF")
    find_package(Catch2 CONFIG)
endif ()

set(Boost_USE_STATIC_LIBS ON)
if(FUN_TIMES_BOOST_VERSION VERSION_LESS 1.70.0)
    find_package(Boost ${FUN_TIMES_BOOST_VERSION} EXACT REQUIRED COMPONENTS ${boost_components})
else()
    find_package(Boost CONFIG REQUIRED COMPONENTS ${boost_components})
endif()
find_package(Threads)

add_subdirectory(decorator)
