# mkdir build
# TORCH_ROOT=$(pwd)/../extern/libtorch
# cd build
# echo "TORCH_ROOT: $TORCH_ROOT"
# cmake -DCMAKE_PREFIX_PATH=$TORCH_ROOT ..
# cmake --build .
# make
# cd ..
# ./build/dcgan
mkdir -p build
cd build
cmake ..
cmake --build .
./engine_tests