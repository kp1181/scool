#!/bin/bash

DIR="."

usage() {
  echo "usage: $0 [-hvdtD] [-p DIR]"
  echo "options:"
  echo "  -h        display this help"
  echo "  -v        enable verbose mode"
  echo "  -d        build debug version"
  echo "  -t        build profiling version"
  echo "  -p [DIR]  install in DIR (default: DIR="$DIR"/bin)"
}

CMAKE_CALL="../"

while getopts ":hvp:dtBR" arg; do
  case $arg in
    h)
      usage
      exit -1
      ;;
    v)
      VERBOSE="VERBOSE=1"
      ;;
    p)
      DIR=$OPTARG
      ;;
    d)
      CMAKE_CALL="$CMAKE_CALL -DCMAKE_BUILD_TYPE=Debug"
      ;;
    t)
      CMAKE_CALL="$CMAKE_CALL -DCMAKE_BUILD_TYPE=Profile"
      ;;
    ?)
      usage
      exit -1
      ;;
  esac
done

if [ -d build ]; then
  rm -rf build
fi

mkdir -p build/

if [ ! -d build ]; then
  echo "error: unable to create build folder"
  exit -1
fi

echo "CMake call: $CMAKE_CALL"

cd build/
cmake $CMAKE_CALL -DCMAKE_INSTALL_PREFIX="$DIR"
make -j 8 install $VERBOSE
