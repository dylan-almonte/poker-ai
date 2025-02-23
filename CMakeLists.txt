cmake_minimum_required(VERSION 3.10)
project(poker_tests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Specify paths
set(POKER_ENGINE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/engine)
set(POKER_ENGINE_INCLUDE_DIR ${POKER_ENGINE_DIR}/include)
set(POKER_ENGINE_SRC_DIR ${POKER_ENGINE_DIR}/src)
set(POKER_ENGINE_LIB_DIR ${POKER_ENGINE_DIR}/bin)

# Add includes
include_directories(${POKER_ENGINE_INCLUDE_DIR})

# Link directories
link_directories(${POKER_ENGINE_LIB_DIR})

# Add the test executable
add_executable(poker_tests test/main.cpp)

set_target_properties(poker_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin
)

# Link against the poker_engine library
target_link_libraries(poker_tests
    poker_engine
)

# Google Test setup
enable_testing()
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(poker_unit_tests
    test/unit_tests.cpp
)

target_link_libraries(poker_unit_tests
    poker_engine
    ${GTEST_LIBRARIES}
    pthread
)

add_test(NAME poker_unit_tests COMMAND poker_unit_tests)