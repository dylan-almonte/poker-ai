cmake_minimum_required(VERSION 3.10)
project(poker_engine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

# Library
add_library(poker_engine
    src/actions.cpp
    src/card.cpp
    src/dealer.cpp
    src/deck.cpp
    src/engine.cpp
    src/evaluator.cpp
    src/lookup_table.cpp
    src/player.cpp
    src/pot.cpp
    src/table.cpp
)

target_include_directories(poker_engine PUBLIC include)

# Add a simple test executable
add_executable(poker_test test/main.cpp)
target_link_libraries(poker_test poker_engine) 