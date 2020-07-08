cmake_minimum_required(VERSION 2.6)
project(libnet)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

set(CXX_FLAGS
      -g
      -O0
      -Wall
      -Wextra
      -Werror
      -Wconversion
      -Wno-unused-parameter
      -Wold-style-cast
      -Woverloaded-virtual
      -Wpointer-arith
      -Wshadow
      -Wwrite-strings
      -march=native
      -std=c++17
      -rdynamic)

string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

set(EXECUTABLE_OUTPUT_PATH  ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH     ${PROJECT_BINARY_DIR}/lib)

include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(libnet)
