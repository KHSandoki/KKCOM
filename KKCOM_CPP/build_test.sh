#!/bin/bash

# Build script for KKCOM Serial Test (no GUI)

echo "Building KKCOM Serial Test..."

# Create build directory
mkdir -p build_test
cd build_test

# Run CMake with test configuration
cmake .. -f ../CMakeLists_test.txt -DCMAKE_BUILD_TYPE=Debug

# Build
make -j$(nproc)

if [ $? -eq 0 ]; then
    echo "Test build successful!"
    echo "Run with: ./KKCOM_Serial_Test"
    echo ""
    echo "This test will:"
    echo "1. List available serial ports"
    echo "2. Try to connect to the first available port"
    echo "3. Send test AT commands"
    echo "4. Display any received data"
else
    echo "Test build failed!"
    exit 1
fi