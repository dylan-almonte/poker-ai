cmake_minimum_required(VERSION 3.10)
project(deep_cfr)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find LibTorch package
find_package(Torch REQUIRED)

# Collect all source files
set(SOURCES
    src/deep_cfr.cpp
    src/info_state.cpp
    src/neural_net.cpp
)

# Create the library
add_library(deep_cfr SHARED ${SOURCES})

# Set include directories for the library
target_include_directories(deep_cfr 
    PUBLIC 
        # This makes the include path available to any target that links deep_cfr
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Link against LibTorch and poker_engine
target_link_libraries(deep_cfr 
    PUBLIC 
        ${TORCH_LIBRARIES}
        poker_engine
)

# Add compile options if needed
target_compile_options(deep_cfr PRIVATE
    -Wall
    -Wextra
)

# Set output directories
set_target_properties(deep_cfr PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib
)

# Add compile definitions for LibTorch
target_compile_definitions(deep_cfr PRIVATE
    TORCH_API_INCLUDE_EXTENSION_H
) 