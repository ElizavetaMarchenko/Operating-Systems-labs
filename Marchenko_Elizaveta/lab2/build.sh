#!/bin/bash
set -e

BUILD_DIR="build"

echo "Cleaning..."
rm -rf $BUILD_DIR
mkdir -p $BUILD_DIR
cd $BUILD_DIR

echo "Generating Makefile using CMake..."
cmake ..

echo "Compiling..."
make

echo "Compiling completed successfully!"
