git clone -b master --recurse-submodule https://github.com/pytorch/pytorch.git
mkdir pytorch-build
cd pytorch-build 
cmake -DBUILD_SHARED_LIBS:BOOL=ON -DCMAKE_BUILD_TYPE:STRING=Release -DPYTHON_EXECUTABLE:PATH=`which python3` -DCMAKE_INSTALL_PREFIX:PATH=../pytorch-install ../pytorch

cmake --build . --target install

mv pytorch-install libtorch
# wget https://download.pytorch.org/libtorch/cpu/libtorch-macos-2.1.0.zip
# unzip libtorch-macos-2.1.0.zip
# mv libtorch libtorch-build
