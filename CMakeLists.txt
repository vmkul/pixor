cmake_minimum_required(VERSION 3.10)
project(PIXOR)
enable_language(CUDA)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -g")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_compile_definitions(BUILD_DIR="${CMAKE_BINARY_DIR}")
add_compile_definitions(LAYOUT_DIR="${CMAKE_BINARY_DIR}/res/layout")

add_subdirectory(src)
