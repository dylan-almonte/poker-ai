cmake_minimum_required(VERSION 3.31)
project(poker_engine)
include_directories(include)
set(CMAKE_CXX_STANDARD 17)

add_library(poker_engine SHARED
    # src/actions.cpp
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