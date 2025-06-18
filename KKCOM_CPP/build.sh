#!/bin/bash

# Build script for KKCOM C++ Edition

echo "Building KKCOM C++ Edition..."

# Create build directory
mkdir -p build
cd build

# Check if Dear ImGui exists
if [ ! -d "../libs/imgui" ]; then
    echo "Dear ImGui not found. Please download it to libs/imgui/"
    echo "You can get it from: https://github.com/ocornut/imgui"
    echo ""
    echo "Quick setup:"
    echo "mkdir -p ../libs"
    echo "cd ../libs"
    echo "git clone https://github.com/ocornut/imgui.git"
    echo "cd .."
    echo ""
    exit 1
fi

# Run CMake
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
make -j$(nproc)

if [ $? -eq 0 ]; then
    echo "Build successful!"
    echo "Run with: ./KKCOM_CPP"
else
    echo "Build failed!"
    exit 1
fi