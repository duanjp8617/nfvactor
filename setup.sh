#! /bin/sh

# Follow https://grpc.io/docs/quickstart/python.html to install python grpc first.

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

  # First build protobuf.
  cd ./third_party/protobuf
  sudo apt-get install autoconf automake libtool curl make g++ unzip
  ./autogen.sh
  ./configure
  make -j
  sudo make install
  sudo ldconfig

  # Then build grpc, in case that the build fail on Ubuntu16.04,
  # please modify the following line in Makefile from :
  # HOST_LDLIBS_PROTOC += $(addprefix -l, $(LIBS_PROTOC))
  # to:
  # HOST_LDLIBS_PROTOC += -L/usr/local/lib $(addprefix -l, $(LIBS_PROTOC))
  # Then use "make clean" and "make -j" to re-build grpc
  # Source: https://github.com/grpc/grpc/issues/9549
  cd ../../
  make -j
  sudo make install
  sudo ldconfig
else
  echo "grpc and protobuf have already been installed."
fi

#switch to deps directory
cd $NFA_DIR/deps

# install several packages that are required to build bess
sudo apt-get install libssl-dev libunwind8-dev liblzma-dev libgoogle-glog-dev libgflags-dev libgtest-dev libpcap-dev libgraph-easy-perl

# download and build bess
if [ ! -d "./bess" ]; then
  git clone -b $BESS_BRANCH https://github.com/duanjp8617/bess.git
  cd bess
  cp $NFA_DIR/besspatch/myflowgen.c ./core/modules/
  ./build.py
else
  echo "bess has already been built."
fi

# build google test, no need, bess installs it.
# build google flag, no need, bess installs it.
# build google log, no need, bess installs it.
