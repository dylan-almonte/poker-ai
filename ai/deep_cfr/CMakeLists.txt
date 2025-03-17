cmake_minimum_required(VERSION 3.10)
project(deep_cfr)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find LibTorch package - specify path for M1 Mac if needed
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "/opt/homebrew/libtorch")
find_package(Torch REQUIRED)
find_package(PythonLibs REQUIRED)


# Collect all source files
set(SOURCES
    src/deep_cfr.cpp
    src/info_state.cpp
    src/cfr_neural_net.cpp
)

# Create the library
add_library(deep_cfr SHARED ${SOURCES})

# Set include directories for the library
target_include_directories(deep_cfr 
    PUBLIC 
        # This makes the include path available to any target that links deep_cfr
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        include
        ${PYTHON_INCLUDE_DIRS}
)

# Link against LibTorch and poker_engine
target_link_libraries(deep_cfr 
    PUBLIC 
        ${TORCH_LIBRARIES}
        ${PYTHON_LIBRARIES}
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
    TORCH_INDUCTOR_CPP_WRAPPER
) 

# Add M1-specific compile definitions if needed
if(APPLE AND CMAKE_SYSTEM_PROCESSOR MATCHES "arm")
    message(STATUS "Configuring for Apple Silicon (M1/ARM)")
    target_compile_definitions(deep_cfr PRIVATE
        APPLE_SILICON=1
    )
endif()

# set_property(TARGET deep_cfr PROPERTY CXX_STANDARD 17)

