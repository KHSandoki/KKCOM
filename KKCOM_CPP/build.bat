@echo off
REM Build script for KKCOM C++ Edition on Windows

echo Building KKCOM C++ Edition for Windows...

REM Check if Visual Studio is available
where cl >nul 2>nul
if %errorlevel% neq 0 (
    echo Visual Studio compiler not found. Please run this from a Visual Studio Developer Command Prompt.
    echo Or install Visual Studio with C++ development tools.
    pause
    exit /b 1
)

REM Create build directory
if not exist build mkdir build
cd build

REM Check if Dear ImGui exists
if not exist "..\libs\imgui" (
    echo Dear ImGui not found. Please download it to libs\imgui\
    echo You can get it from: https://github.com/ocornut/imgui
    echo.
    echo Quick setup:
    echo mkdir ..\libs
    echo cd ..\libs
    echo git clone https://github.com/ocornut/imgui.git
    echo cd ..
    echo.
    pause
    exit /b 1
)

REM Run CMake for Visual Studio
cmake .. -G "Visual Studio 16 2019" -A x64 -DCMAKE_BUILD_TYPE=Release

REM If VS2019 not found, try VS2022
if %errorlevel% neq 0 (
    echo Trying Visual Studio 2022...
    cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release
)

REM If still fails, try default generator
if %errorlevel% neq 0 (
    echo Trying default generator...
    cmake .. -DCMAKE_BUILD_TYPE=Release
)

if %errorlevel% neq 0 (
    echo CMake configuration failed!
    pause
    exit /b 1
)

REM Build the project
cmake --build . --config Release

if %errorlevel% equ 0 (
    echo Build successful!
    echo Run with: Release\KKCOM_CPP.exe
    pause
) else (
    echo Build failed!
    pause
    exit /b 1
)

cd ..