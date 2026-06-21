# Set minimum CMake version
cmake_minimum_required(VERSION "3.21")

# Configure project and languages
project(pico_bi LANGUAGES C)

# Define the library
add_library(pico_bi INTERFACE)
target_sources(pico_bi INTERFACE src/pico_binary_info.c)

# Specify the include paths
target_include_directories(pico_bi INTERFACE include)

# Build tests by default when pico_bi is the top-level project
option(PICO_BI_BUILD_TESTS "Build bininfo tests" ${PROJECT_IS_TOP_LEVEL})

# Include tests if enabled
if(PICO_BI_BUILD_TESTS)
  include(CTest)
  add_subdirectory(tests)
endif()
