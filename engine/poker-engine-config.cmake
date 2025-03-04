# Get the directory containing this file
get_filename_component(POKER_ENGINE_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)

# Optional: Define version
set(POKER_ENGINE_VERSION "1.0.0")

# Optional: Check for required dependencies
# find_package(SomeOtherLibrary REQUIRED)

# Optional: Define include directories
set(POKER_ENGINE_INCLUDE_DIRS "${POKER_ENGINE_CMAKE_DIR}/include")

# Optional: Define any compile definitions needed
# set(POKER_ENGINE_DEFINITIONS "")

# Import the targets
include("${POKER_ENGINE_CMAKE_DIR}/poker-engine-targets.cmake")

# Optional: Define any additional flags or settings
set(POKER_ENGINE_CXX_FLAGS "")

# Optional: Verify everything was found
if(NOT TARGET poker::poker_engine)
    message(FATAL_ERROR "poker_engine targets not found!")
endif() 