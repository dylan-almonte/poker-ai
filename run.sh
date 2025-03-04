#!/bin/bash
set -e  # Exit on error

# Set up environment variables
export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:$(pwd)/extern/libtorch/lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)/extern/libtorch/lib
# Make a backup of the original file
cp extern/libtorch/include/torch/csrc/python_headers.h extern/libtorch/include/torch/csrc/python_headers.h.bak

# Edit the file to comment out the Python.h include
# sed -i.bak 's/#include <Python.h>/\/\/ #include <Python.h>/' extern/libtorch/include/torch/csrc/python_headers.h
export TORCH_ROOT=$(pwd)/extern/libtorch

# Clean build directory
rm -rf build
mkdir -p build

# Configure and build
cd build
# cmake .. -DCMAKE_VERBOSE_MAKEFILE=ON
cmake -DCMAKE_PREFIX_PATH=$TORCH_ROOT ..
cmake --build .

# Go back to project root and run
cd ..
./bin/deep_cfr