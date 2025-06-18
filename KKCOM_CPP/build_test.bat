@echo off
REM Build script for KKCOM Serial Test (no GUI) on Windows

echo Building KKCOM Serial Test for Windows...

REM Check if Visual Studio is available
where cl >nul 2>nul
if %errorlevel% neq 0 (
    echo Visual Studio compiler not found. Please run this from a Visual Studio Developer Command Prompt.
    echo Or install Visual Studio with C++ development tools.
    pause
    exit /b 1
)

REM Create build directory
if not exist build_test mkdir build_test
cd build_test

REM Run CMake with test configuration
cmake .. -f ..\CMakeLists_test.txt -G "Visual Studio 16 2019" -A x64 -DCMAKE_BUILD_TYPE=Release

REM If VS2019 not found, try VS2022
if %errorlevel% neq 0 (
    echo Trying Visual Studio 2022...
    cmake .. -f ..\CMakeLists_test.txt -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release
)

REM If still fails, try default generator
if %errorlevel% neq 0 (
    echo Trying default generator...
    cmake .. -f ..\CMakeLists_test.txt -DCMAKE_BUILD_TYPE=Release
)

if %errorlevel% neq 0 (
    echo CMake configuration failed!
    pause
    exit /b 1
)

REM Build the project
cmake --build . --config Release

if %errorlevel% equ 0 (
    echo Test build successful!
    echo Run with: Release\KKCOM_Serial_Test.exe
    echo.
    echo This test will:
    echo 1. List available serial ports
    echo 2. Try to connect to the first available port
    echo 3. Send test AT commands
    echo 4. Display any received data
    pause
) else (
    echo Test build failed!
    pause
    exit /b 1
)

cd ..