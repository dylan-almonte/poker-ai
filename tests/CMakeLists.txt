# Minimum CMake version
cmake_minimum_required(VERSION 3.31)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_subdirectory(
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine_v2
    ${CMAKE_BINARY_DIR}/engine           
)
# Enable testing
enable_testing()

# Add Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.11.0
)
FetchContent_MakeAvailable(googletest)

# Create test executable
add_executable(engine_tests
    engine/game_test.cpp
    # Add other test files here
)

# Link against gtest and your engine library
target_link_libraries(engine_tests
    PRIVATE
    gtest
    gtest_main
    poker_engine  # Your engine library
)
target_compile_options(engine_tests PRIVATE -g -DDEBUG)