
cmake_minimum_required(VERSION 3.10)

set( CMAKE_C_STANDARD 11 )

project(muddled)

add_subdirectory(src)
add_subdirectory(tests)

add_subdirectory(web)
add_subdirectory(resources)
add_subdirectory(scripts)

