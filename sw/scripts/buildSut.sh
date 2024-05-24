#!/bin/bash

set -e

SCRIPT=$(realpath "$0")
SCRIPT_PATH=$(dirname "$SCRIPT")
ROOT="${SCRIPT_PATH}/.."
APPDIR=${ROOT}/applications/SUT
BUILD_DIR=${ROOT}/applications/SUT/CI_Build
source $ROOT/scripts/setenv.sh

clean()
{
    rm -rf $BUILD_DIR/*
}

build()
{
    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=toolchain.cmake -S . -B $BUILD_DIR 
    cd $BUILD_DIR
    make -j$(nproc)
}

install()
{
    make install
}

cd $ROOT/applications/SUT
clean
build
install