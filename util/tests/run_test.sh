#!/usr/bin/env bash
mkdir -p dist
g++ $1 -o dist/$1.bin -I../.. -std=c++2a ../alloc.cc ../string.cc ../task.cc ../util.cc && ./dist/$1.bin

