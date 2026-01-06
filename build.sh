#!/bin/bash

SRC_FILE=_anmedit.cpp
CXX=clang
CXX_ARGS="-g -gcodeview -O0 -I ../include -I ../include/SDL2 -I ../src/th06 -I ../subprojects -I ../subprojects/imgui"
LINK_ARGS="-L ../lib -fuse-ld=lld -lSDL2 -lSDL2_image -lSDL2main -lSDL3 -Wl,/subsystem:windows"
OUT_FILE=AnmEdit6.exe

set -e

mkdir -p build
pushd build
set -x
$CXX $CXX_ARGS $LINK_ARGS -o $OUT_FILE ../src/$SRC_FILE
set +x
popd