
cmake_minimum_required(VERSION 3.10)
project(PokerAI)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the engine subdirectory
add_subdirectory("engine")
add_subdirectory(ai/deep_cfr)

# Create your executable
add_executable(poker_ai main.cpp)

# Link against the engine library and get its include directories
target_link_libraries(poker_ai PRIVATE
    deep_cfr
    poker_engine
)

set_target_properties(poker_ai PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib
)

# Add the -Wno-unused-variable flag
# target_compile_options(poker_ai PUBLIC -g)

# # Enable testing
# enable_testing()

# # Add Google Test
# include(FetchContent)
# FetchContent_Declare(
#     googletest
#     GIT_REPOSITORY https://github.com/google/googletest.git
#     GIT_TAG release-1.11.0
)
# FetchContent_MakeAvailable(googletest)

# # Create test executable
# add_executable(engine_tests
#     tests/engine/game_test.cpp
#     # Add other test files here
)

# # Link against gtest and your engine library
# target_link_libraries(engine_tests
#     PRIVATE
#     gtest
#     gtest_main
#     poker_engine  # Your engine library
)