cmake_minimum_required(VERSION 3.10)
project(poker_engine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Collect all source files
set(SOURCES
    src/action_type.cpp
    src/card.cpp
    src/deck.cpp
    src/evaluator.cpp
    src/game.cpp
    src/hand_phase.cpp
    src/player_state.cpp
    src/pot.cpp
)

# Create the library
add_library(poker_engine SHARED ${SOURCES})

# Set include directories for the library
target_include_directories(poker_engine 
    PUBLIC 
        # This makes the include path available to any target that links poker_engine
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Add compile options if needed
target_compile_options(poker_engine PRIVATE
    -Wall
    -Wextra
)

set_target_properties(poker_engine PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib
)