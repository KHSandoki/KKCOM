@echo off
REM Build script using vcpkg dependencies

echo Building KKCOM C++ Edition with vcpkg...

REM Check if vcpkg exists
if not exist "vcpkg\vcpkg.exe" (
    echo vcpkg not found. Please run setup_vcpkg.bat first.
    pause
    exit /b 1
)

REM Check if Dear ImGui exists
if not exist "libs\imgui" (
    echo Dear ImGui not found. Downloading...
    if not exist libs mkdir libs
    cd libs
    git clone https://github.com/ocornut/imgui.git
    if %errorlevel% neq 0 (
        echo Failed to download Dear ImGui.
        pause
        exit /b 1
    )
    cd ..
)

REM Create build directory
if not exist build_vcpkg mkdir build_vcpkg
cd build_vcpkg

REM Run CMake with vcpkg toolchain
cmake .. -DCMAKE_TOOLCHAIN_FILE=..\vcpkg\scripts\buildsystems\vcpkg.cmake -G "Visual Studio 16 2019" -A x64

REM If VS2019 not found, try VS2022
if %errorlevel% neq 0 (
    echo Trying Visual Studio 2022...
    cmake .. -DCMAKE_TOOLCHAIN_FILE=..\vcpkg\scripts\buildsystems\vcpkg.cmake -G "Visual Studio 17 2022" -A x64
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