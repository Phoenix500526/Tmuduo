#!/bin/sh

set -x

SOURCE_DIR=`pwd`
BUILD_DIR=${BUILD_DIR:-./build}
BUILD_TYPE=${BUILD_TYPE:-debug}

mkdir -p $BUILD_DIR \
  && cd $BUILD_DIR \
  && conan install .. \
            --build missing     \
            -s compiler=clang   \
            -s compiler.version=3.8 \
            -s compiler.libcxx=libstdc++11  \
  && cmake .. \
           -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
           -DCMAKE_CXX_COMPILER=clang++ \
           -DCMAKE_C_COMPILER=clang \
  && make $*

# Use the following command to run all the unit tests
# at the dir $BUILD_DIR/$BUILD_TYPE :
# CTEST_OUTPUT_ON_FAILURE=TRUE make test

# cd $SOURCE_DIR && doxygen

