cmake_minimum_required(VERSION 3.10)
project(poker_server)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find package for JSON library
find_package(nlohmann_json QUIET)
if(NOT nlohmann_json_FOUND)
    # If not found, download and include
    include(FetchContent)
    FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.2  # Use a specific version
    )
    FetchContent_MakeAvailable(nlohmann_json)
endif()

# Server library source files
set(SERVER_SOURCES
    src/server.cpp
    src/client.cpp
    src/poker_server.cpp
)

# Add the server library
add_library(poker_server SHARED ${SERVER_SOURCES})

# Set include directories
target_include_directories(poker_server
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

# Link libraries
target_link_libraries(poker_server
    PUBLIC
        nlohmann_json::nlohmann_json
        poker_engine
)

# Add compile options
target_compile_options(poker_server PRIVATE
    -Wall
    -Wextra
    -O0
    -g
)

# Set output directories
set_target_properties(poker_server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib
)

# Create server example executable
add_executable(server_example examples/server_example.cpp)
target_link_libraries(server_example poker_server)

# Create client example executable
add_executable(client_example examples/client_example.cpp)
target_link_libraries(client_example poker_server)

# Create poker server example executable
add_executable(poker_server_main examples/poker_server_main.cpp)
target_link_libraries(poker_server_main poker_server)

# Install targets
install(TARGETS poker_server
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/
    DESTINATION include/poker_server
    FILES_MATCHING PATTERN "*.h"
)

install(TARGETS server_example client_example poker_server_main
    RUNTIME DESTINATION bin
) 