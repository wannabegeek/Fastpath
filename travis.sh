#!/usr/bin/env sh
set -evx
env | sort

mkdir build || true
mkdir build/$BUILD_TYPE || true
cd build/$BUILD_TYPE
cmake -DCMAKE_CXX_FLAGS=$CXX_FLAGS -DCMAKE_BUILD_TYPE=$BUILD_TYPE ../../
make
make test
