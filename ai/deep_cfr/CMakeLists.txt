cmake_minimum_required(VERSION 3.10)
project(DeepCFR)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/engine/include
)

# Add required headers
add_definitions(-include random)  # Include the random header for std::mt19937

# Source files
set(SOURCES
    src/deep_cfr.cpp
    src/cfr_network.cpp
    src/memory_buffer.cpp
    src/mytorch/tensor.cpp
)

# Create library
add_library(deep_cfr STATIC ${SOURCES})

# Link against engine
target_link_libraries(deep_cfr PRIVATE
    poker_engine
)

# Set compiler flags
if(MSVC)
    target_compile_options(deep_cfr PRIVATE /W4)
else()
    target_compile_options(deep_cfr PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Installation rules
install(TARGETS deep_cfr
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION include/deep_cfr
    FILES_MATCHING PATTERN "*.hpp"
)

# Optional: Add tests directory
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests)
    enable_testing()
    add_subdirectory(tests)
endif()
