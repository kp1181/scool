#!/bin/bash

mkdir -p doc
naturaldocs -p nd/ -o html doc/

if [ -n "$1" ]; then
  echo "removing docs..."
  rm -rf doc/*
  rm -rf "nd/Working Data"
fi
