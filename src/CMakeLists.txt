cmake_minimum_required (VERSION 3.1)

project(mkit)

include(CheckIncludeFile)
include(CheckSymbolExists)

# Force build type to Debug if none was specified.
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif(NOT CMAKE_BUILD_TYPE)

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    add_definitions( -DDEBUG )
endif(CMAKE_BUILD_TYPE MATCHES "Debug")

find_package(Git)
if(GIT_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --long --tags --dirty --always
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_VERSION
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
        COMMAND ${GIT_EXECUTABLE} log -1 --date=format:"%Y/%m/%d %T" --format=%ad
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_DATE
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
else()
    set(GIT_VERSION "Unknown")
    set(GIT_DATE "__DATE__")
endif()
message( STATUS "GIT_VERSION: ${GIT_VERSION}")
message( STATUS "GIT_DATE: ${GIT_DATE}")
 
set(LIBRARY_OUTPUT_PATH    "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
set(ARCHIVE_OUTPUT_PATH    "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}")

check_symbol_exists(strcasecmp "strings.h" HAVE_STRCASECMP)
check_include_file("getopt.h" HAVE_GETOPT_H)

if(NOT HAVE_GETOPT_H)
    add_library(getopt STATIC externals/getopt/getopt.c externals/getopt/getopt1.c externals/getopt/getopt.h)
    target_include_directories(getopt PUBLIC externals/getopt/)
endif()

if(HAVE_STRCASECMP)
    add_definitions(-DHAVE_STRCASECMP)
endif()

add_subdirectory(huc)
add_subdirectory(mkit/as)