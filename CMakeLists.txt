cmake_minimum_required(VERSION 2.8)

project(muddyplains)

set(CMAKE_C_FLAGS "-std=c99")

add_subdirectory(src tests)

enable_testing()
