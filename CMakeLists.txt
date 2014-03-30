cmake_minimum_required(VERSION 2.8)

project(muddyplains)

set(CMAKE_C_FLAGS "-std=c99 -Wall -Werror -DROOT_DIR=\"${CMAKE_SOURCE_DIR}\"")

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} -ggdb3 -O0")

add_subdirectory(src tests)

enable_testing()
