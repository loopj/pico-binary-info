# Fetch the Unity test framework
if(NOT unity_FOUND)
  include(FetchContent)
  FetchContent_Declare(Unity GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git)
  FetchContent_MakeAvailable(Unity)
endif()

# Little helper function to create a test
function(add_library_test TEST_NAME)
  add_executable(${TEST_NAME} ${ARGN})
  target_link_libraries(${TEST_NAME} pico_bi unity::framework)
  target_compile_definitions(${TEST_NAME} PRIVATE
    FIXTURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures")
  add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endfunction()

# Add the tests
add_library_test(test_pico_binary_info test_pico_binary_info.c)
