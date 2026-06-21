cmake_minimum_required(VERSION 3.20)

project(read_fd C CXX ASM)

set(PICO_BI_ROOT ${CMAKE_CURRENT_LIST_DIR}/../..)
add_subdirectory(${PICO_BI_ROOT} ${CMAKE_BINARY_DIR}/_deps/pico_bi-build)

add_executable(read_fd main.c)

target_link_libraries(read_fd pico_bi)
