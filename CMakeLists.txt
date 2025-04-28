cmake_minimum_required(VERSION 3.10)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # For clang-tidy.
set(BUILD_SHARED_LIBS OFF) # We expect external libraries to be linked statically.
set(CMAKE_CXX_STANDARD 17) # Compile as C++17.
set(CMAKE_CXX_STANDARD_REQUIRED ON) # Require C++17 support.

project(PokerAI
        VERSION 2023.1
        DESCRIPTION "Poker AI"
)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
        message(STATUS "Setting build type to `Debug` as none was specified.")
        set(CMAKE_BUILD_TYPE "Debug")
endif()

if(EMSCRIPTEN)
        add_compile_options(-fexceptions)
        add_link_options(-fexceptions)

        # Memory configuration
        add_compile_options(-sALLOW_MEMORY_GROWTH=1)
        add_compile_options(-sMAXIMUM_MEMORY=64MB)
        add_link_options(-sALLOW_MEMORY_GROWTH=1)
        add_link_options(-sMAXIMUM_MEMORY=64MB)
endif()

# People keep running CMake in the wrong folder, completely nuking their project or creating weird bugs.
# This checks if you're running CMake from a folder that already has CMakeLists.txt.
# Importantly, this catches the common case of running it from the root directory.
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" PATH_TO_CMAKELISTS_TXT)

if(EXISTS "${PATH_TO_CMAKELISTS_TXT}")
        message(FATAL_ERROR "Run CMake from a build subdirectory! \"mkdir build ; cd build ; cmake ..\" \
    Some junk files were created in this folder (CMakeCache.txt, CMakeFiles); you should delete those.")
endif()

# Expected directory structure.
set(POKERAI_BUILD_SUPPORT_DIR "${CMAKE_SOURCE_DIR}/build_support")
set(POKERAI_CLANG_SEARCH_PATH "/usr/local/bin" "/usr/bin" "/usr/local/opt/llvm/bin" "/usr/local/opt/llvm@14/bin"
        "/opt/homebrew/opt/llvm@14/bin/")

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        if(CMAKE_CXX_COMPILER_VERSION MATCHES "^14.")
                message(STATUS "You're using ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
        else()
                message(WARNING "!! We recommend that you use clang-14 for developing PokerAI. You're using ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}, a different version.")
        endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
        message(STATUS "You're using ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
else()
        message(WARNING "!! We recommend that you use clang-14 for developing PokerAI. You're using ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}, which is not clang.")
endif()

# #####################################################################################################################
# DEPENDENCIES
# #####################################################################################################################

# CTest
enable_testing()

# clang-format

# attempt to find the binary if user did not specify
find_program(CLANG_FORMAT_BIN
        NAMES clang-format clang-format-14
        HINTS ${POKERAI_CLANG_SEARCH_PATH})

if("${CLANG_FORMAT_BIN}" STREQUAL "CLANG_FORMAT_BIN-NOTFOUND")
        message(WARNING "PokerAI/main couldn't find clang-format.")
else()
        message(STATUS "PokerAI/main found clang-format at ${CLANG_FORMAT_BIN}")
endif()

# attempt to find the binary if user did not specify
find_program(CLANG_TIDY_BIN
        NAMES clang-tidy clang-tidy-14
        HINTS ${PokerAI_CLANG_SEARCH_PATH})

if("${CLANG_TIDY_BIN}" STREQUAL "CLANG_TIDY_BIN-NOTFOUND")
        message(WARNING "PokerAI/main couldn't find clang-tidy.")
else()
        # Output compile_commands.json
        set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
        message(STATUS "PokerAI/main found clang-tidy at ${CLANG_TIDY_BIN}")
endif()

find_program(CLANG_APPLY_REPLACEMENTS_BIN
        NAMES clang-apply-replacements clang-apply-replacements-14
        HINTS ${PokerAI_CLANG_SEARCH_PATH})

if("${CLANG_APPLY_REPLACEMENTS_BIN}" STREQUAL "CLANG_APPLY_REPLACEMENTS_BIN-NOTFOUND")
        message(WARNING "PokerAI/main couldn't find clang-apply-replacements.")
else()
        # Output compile_commands.json
        set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
        message(STATUS "PokerAI/main found clang-apply-replacements at ${CLANG_APPLY_REPLACEMENTS_BIN}")
endif()

# cpplint
find_program(CPPLINT_BIN
        NAMES cpplint cpplint.py
        HINTS "${POKERAI_BUILD_SUPPORT_DIR}")

if("${CPPLINT_BIN}" STREQUAL "CPPLINT_BIN-NOTFOUND")
        message(WARNING "PokerAI/main couldn't find cpplint.")
else()
        message(STATUS "PokerAI/main found cpplint at ${CPPLINT_BIN}")
endif()

# #####################################################################################################################
# COMPILER SETUP
# #####################################################################################################################
if(NOT DEFINED POKERAI_SANITIZER)
        set(POKERAI_SANITIZER address)
endif()

message("Build mode: ${CMAKE_BUILD_TYPE}")
message("${POKERAI_SANITIZER} sanitizer will be enabled in debug mode.")

# Compiler flags.
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -Werror")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wno-unused-parameter -Wno-attributes") # TODO: remove
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -ggdb -fsanitize=${POKERAI_SANITIZER} -fno-omit-frame-pointer -fno-optimize-sibling-calls")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS_DEBUG: ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CMAKE_EXE_LINKER_FLAGS: ${CMAKE_EXE_LINKER_FLAGS}")
message(STATUS "CMAKE_SHARED_LINKER_FLAGS: ${CMAKE_SHARED_LINKER_FLAGS}")

# Output directory.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Includes.
set(POKERAI_SRC_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src/include)
set(POKERAI_TEST_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/test/include)
set(POKERAI_THIRD_PARTY_INCLUDE_DIR
        ${PROJECT_SOURCE_DIR}/third_party
        ${PROJECT_SOURCE_DIR}/third_party/fmt/include
        ${PROJECT_SOURCE_DIR}/third_party/libpg_query/include
        ${PROJECT_SOURCE_DIR}/third_party/argparse/include
        ${PROJECT_SOURCE_DIR}/third_party/cpp_random_distributions
        ${PROJECT_SOURCE_DIR}/third_party/backward-cpp
)

include_directories(
    ${POKERAI_SRC_INCLUDE_DIR} 
    ${POKERAI_TEST_INCLUDE_DIR} 
    ${POKERAI_THIRD_PARTY_INCLUDE_DIR}
)
include_directories(BEFORE src) # This is needed for gtest.

function(disable_target_warnings NAME)
        target_compile_options(${NAME} PRIVATE "-w")
endfunction()

# #####################################################################################################################
# Other CMake modules
# MUST BE ADDED AFTER CONFIGURING COMPILER PARAMETERS
# #####################################################################################################################
set(CMAKE_MODULE_PATH "${POKERAI_BUILD_SUPPORT_DIR}/cmake;${CMAKE_MODULE_PATH}")
find_package(LibElf)
find_package(LibDwarf)

add_subdirectory(third_party)
add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(tools)


# #####################################################################################################################
# MAKE TARGETS
# #####################################################################################################################

# #########################################
# "make format"
# "make check-format"
# #########################################
string(CONCAT POKERAI_FORMAT_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}/src,"
        "${CMAKE_CURRENT_SOURCE_DIR}/test,"
)

# Runs clang format and updates files in place.
add_custom_target(format ${POKERAI_BUILD_SUPPORT_DIR}/run_clang_format.py
        ${CLANG_FORMAT_BIN}
        ${POKERAI_BUILD_SUPPORT_DIR}/clang_format_exclusions.txt
        --source_dirs
        ${POKERAI_FORMAT_DIRS}
        --fix
        --quiet
)

# Runs clang format and exits with a non-zero exit code if any files need to be reformatted
add_custom_target(check-format ${POKERAI_BUILD_SUPPORT_DIR}/run_clang_format.py
        ${CLANG_FORMAT_BIN}
        ${POKERAI_BUILD_SUPPORT_DIR}/clang_format_exclusions.txt
        --source_dirs
        ${POKERAI_FORMAT_DIRS}
        --quiet
)

# #########################################
# "make check-lint"
# #########################################
file(GLOB_RECURSE POKERAI_LINT_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/test/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp"
)

# Balancing act: cpplint.py takes a non-trivial time to launch,
# so process 12 files per invocation, while still ensuring parallelism
add_custom_target(check-lint echo '${POKERAI_LINT_FILES}' | xargs -n12 -P8
        ${CPPLINT_BIN}
        --verbose=2 --quiet
        --linelength=120
        --filter=-legal/copyright,-build/header_guard,-runtime/references # https://github.com/cpplint/cpplint/issues/148
)

# ##########################################################
# "make check-clang-tidy" target
# ##########################################################
# runs clang-tidy and exits with a non-zero exit code if any errors are found.
# note that clang-tidy automatically looks for a .clang-tidy file in parent directories
add_custom_target(check-clang-tidy
        ${POKERAI_BUILD_SUPPORT_DIR}/run_clang_tidy.py # run LLVM's clang-tidy script
        -clang-tidy-binary ${CLANG_TIDY_BIN} # using our clang-tidy binary
        -p ${CMAKE_BINARY_DIR} # using cmake's generated compile commands
)
add_custom_target(fix-clang-tidy
        ${POKERAI_BUILD_SUPPORT_DIR}/run_clang_tidy.py # run LLVM's clang-tidy script
        -clang-tidy-binary ${CLANG_TIDY_BIN} # using our clang-tidy binary
        -p ${CMAKE_BINARY_DIR} # using cmake's generated compile commands
        -clang-apply-replacements-binary ${CLANG_APPLY_REPLACEMENTS_BIN} # using our clang-apply-replacements binary
        -fix # apply suggested changes generated by clang-tidy
)
add_custom_target(check-clang-tidy-diff
        ${POKERAI_BUILD_SUPPORT_DIR}/run_clang_tidy.py # run LLVM's clang-tidy script
        -clang-tidy-binary ${CLANG_TIDY_BIN} # using our clang-tidy binary
        -p ${CMAKE_BINARY_DIR} # using cmake's generated compile commands
        -only-diff # only check diff files to master
)
add_custom_target(fix-clang-tidy-diff
        ${POKERAI_BUILD_SUPPORT_DIR}/run_clang_tidy.py # run LLVM's clang-tidy script
        -clang-tidy-binary ${CLANG_TIDY_BIN} # using our clang-tidy binary
        -p ${CMAKE_BINARY_DIR} # using cmake's generated compile commands
        -clang-apply-replacements-binary ${CLANG_APPLY_REPLACEMENTS_BIN} # using our clang-apply-replacements binary
        -fix # apply suggested changes generated by clang-tidy
        -only-diff # only check diff files to master
)

# ##########################################################
# "make check-clang-tidy" target for projects
# check clang-tidy on the whole projects is slow, so we
# hardcode some files to check here for each project.
# ##########################################################
set(ENGINE_FILES
        "src/include/engine/engine.h"
        "src/engine/engine.cpp"
)

add_custom_target(check-clang-tidy-engine
        ${PokerAI_BUILD_SUPPORT_DIR}/run_clang_tidy.py # run LLVM's clang-tidy script
        -clang-tidy-binary ${CLANG_TIDY_BIN} # using our clang-tidy binary
        -p ${CMAKE_BINARY_DIR} # using cmake's generated compile commands
        ${ENGINE_FILES}
)


add_dependencies(check-clang-tidy gtest PokerAI) # needs gtest headers, compile_commands.json