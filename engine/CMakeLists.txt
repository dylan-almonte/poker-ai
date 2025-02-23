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
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Set output directories
set_target_properties(poker_engine PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin
)

# Add compile options if needed
target_compile_options(poker_engine PRIVATE
    -Wall
    -Wextra
)

# Install the library and headers
install(TARGETS poker_engine
    EXPORT poker-engine-targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)

# Export targets
install(EXPORT poker-engine-targets
    FILE poker-engine-targets.cmake
    NAMESPACE poker::
    DESTINATION lib/cmake/poker_engine
)

# Install the config file
install(FILES poker-engine-config.cmake
    DESTINATION lib/cmake/poker_engine
)
