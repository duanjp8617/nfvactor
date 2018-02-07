#! /bin/sh

# record current working directory
NFA_DIR=`pwd`

# the current working branch of bess
BESS_BRANCH="c-legacy"

# switch to the deps directory
cd deps

# install grpc and protocbuf
if [ ! -d "./grpc" ]; then

  # build and install grpc
  sudo apt-get install build-essential autoconf libtool
  git clone -b $(curl -L http://grpc.io/release) https://github.com/grpc/grpc
  cd grpc
  git submodule update --init
  make -j`nproc`
  sudo make install

  # go to /third_party/protobuf and install protobuf
  cd ./third_party/protobuf
  make -j`nproc`
  sudo make install
else
  echo "grpc and protobuf have already been installed."
fi

# switch to deps directory
cd $NFA_DIR/deps

# install benchmark
if [ ! -d "./benchmark" ]; then
  git clone https://github.com/google/benchmark.git
  cd benchmark
  mkdir build
  cd build
  cmake ..
  make -j`nproc`
  sudo make install
else
  echo "benchmark has already been installed."
fi

#switch to deps directory
cd $NFA_DIR/deps

# install several packages that are required to build bess
sudo apt-get install libssl-dev libunwind8-dev liblzma-dev libgoogle-glog-dev libgflags-dev libgtest-dev

# download and build bess
if [ ! -d "./bess" ]; then
  git clone -b $BESS_BRANCH https://github.com/NetSys/bess.git
  cd bess
  ./build.py
else
  echo "bess has already been built."
fi

# build google test, no need, bess installs it.
# build google flag, no need, bess installs it.
# build google log, no need, bess installs it.
