#!/usr/bin/env sh
set -evx
env | sort

mkdir external || true
wget -qO- https://sourceforge.net/projects/boost/files/boost/1.60.0/boost_1_60_0.tar.gz/download | tar xvz -C external/
export BOOST_ROOT=external/boost_1_60_0/

mkdir build || true
mkdir build/$BUILD_TYPE || true
cd build/$BUILD_TYPE
cmake -DCMAKE_CXX_FLAGS=$CXX_FLAGS -DCMAKE_BUILD_TYPE=$BUILD_TYPE ../../
make
make test
