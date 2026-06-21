cmake_minimum_required(VERSION 3.20)

project(read_in_memory C CXX ASM)

set(PICO_BI_ROOT ${CMAKE_CURRENT_LIST_DIR}/../..)
add_subdirectory(${PICO_BI_ROOT} ${CMAKE_BINARY_DIR}/_deps/pico_bi-build)

add_executable(read_in_memory main.c)

target_link_libraries(read_in_memory pico_bi)
