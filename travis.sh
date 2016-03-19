#!/usr/bin/env sh
set -evx
env | sort

cd $TRAVIS_BUILD_DIR
mkdir external || true
wget -qO- https://sourceforge.net/projects/boost/files/boost/1.60.0/boost_1_60_0.tar.gz/download | tar xz -C external/
export BOOST_ROOT=$TRAVIS_BUILD_DIR/external/boost_1_60_0/

cd $TRAVIS_BUILD_DIR
wget -qO googletest.zip https://github.com/google/googletest/archive/master.zip
unzip -qd external googletest.zip
mkdir -p build/external/googletest
cd build/external/googletest
cmake -DCMAKE_BUILD_TYPE=RELEASE $TRAVIS_BUILD_DIR/external/googletest-master/googletest/
make
export GTEST_ROOT=$TRAVIS_BUILD_DIR/external/googletest-master/googletest/
mkdir -p $GTEST_ROOT/lib
cp lib*.a $GTEST_ROOT/lib

cd $TRAVIS_BUILD_DIR
mkdir build || true
mkdir build/$BUILD_TYPE || true
cd build/$BUILD_TYPE
cmake -DCMAKE_CXX_FLAGS=$CXX_FLAGS -DCMAKE_BUILD_TYPE=$BUILD_TYPE ../../
make
make tests
./tests/tests