#!/bin/bash

if [ -d build/ ]; then
  cd build/
  make install
else
  ./build.sh
fi
