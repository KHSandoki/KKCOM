@echo off
REM Build standalone executable with static linking

echo =====================================
echo Building Standalone KKCOM Executable
echo =====================================
echo.

REM Check if vcpkg exists
if not exist "vcpkg\vcpkg.exe" (
    echo vcpkg not found. Running setup...
    call setup_vcpkg.bat
    if %errorlevel% neq 0 (
        echo Failed to setup vcpkg.
        pause
        exit /b 1
    )
)

REM Check if static libraries are installed
echo Checking for static libraries...
if not exist "vcpkg\installed\x64-windows-static\lib\glfw3.lib" (
    echo Installing static GLFW3...
    cd vcpkg
    vcpkg install glfw3:x64-windows-static
    cd ..
)

if not exist "vcpkg\installed\x64-windows-static\include\nlohmann\json.hpp" (
    echo Installing static nlohmann-json...
    cd vcpkg
    vcpkg install nlohmann-json:x64-windows-static
    cd ..
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

REM Clean and create build directory
if exist build_standalone (
    echo Cleaning previous build...
    rmdir /s /q build_standalone
)
mkdir build_standalone
cd build_standalone

echo.
echo Setting up Visual Studio environment...

REM Try to find and setup Visual Studio environment
set "VS_FOUND="
set "CMAKE_FOUND="

REM Check for VS2022
if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\Tools\VsDevCmd.bat" (
    call "C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\Tools\VsDevCmd.bat" -arch=x64
    set "VS_FOUND=2022"
    goto :cmake_check
)
if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" (
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" -arch=x64
    set "VS_FOUND=2022"
    goto :cmake_check
)
if exist "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\Common7\Tools\VsDevCmd.bat" (
    call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\Common7\Tools\VsDevCmd.bat" -arch=x64
    set "VS_FOUND=2022"
    goto :cmake_check
)

REM Check for VS2019
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\Common7\Tools\VsDevCmd.bat" (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\Common7\Tools\VsDevCmd.bat" -arch=x64
    set "VS_FOUND=2019"
    goto :cmake_check
)
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat" (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat" -arch=x64
    set "VS_FOUND=2019"
    goto :cmake_check
)
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\Common7\Tools\VsDevCmd.bat" (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\Common7\Tools\VsDevCmd.bat" -arch=x64
    set "VS_FOUND=2019"
    goto :cmake_check
)
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\Common7\Tools\VsDevCmd.bat" (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\Common7\Tools\VsDevCmd.bat" -arch=x64
    set "VS_FOUND=2019"
    goto :cmake_check
)

echo ERROR: Visual Studio not found!
echo Please install Visual Studio 2019 or 2022 with C++ development tools.
pause
exit /b 1

:cmake_check
echo Visual Studio %VS_FOUND% environment loaded.

REM Check if cmake is available
cmake --version >nul 2>&1
if %errorlevel% equ 0 (
    set "CMAKE_FOUND=YES"
    echo CMake found.
) else (
    echo CMake not found in PATH. Checking common locations...
    if exist "C:\Program Files\CMake\bin\cmake.exe" (
        set "PATH=C:\Program Files\CMake\bin;%PATH%"
        set "CMAKE_FOUND=YES"
        echo CMake found in Program Files.
    ) else if exist "C:\Program Files (x86)\CMake\bin\cmake.exe" (
        set "PATH=C:\Program Files (x86)\CMake\bin;%PATH%"
        set "CMAKE_FOUND=YES"
        echo CMake found in Program Files (x86).
    ) else (
        echo ERROR: CMake not found!
        echo Please install CMake from https://cmake.org/download/
        echo Make sure CMake is added to your PATH.
        pause
        exit /b 1
    )
)

echo.
echo Configuring with static linking...
if "%VS_FOUND%"=="2022" (
    cmake .. -DCMAKE_TOOLCHAIN_FILE=..\vcpkg\scripts\buildsystems\vcpkg.cmake ^
             -DVCPKG_TARGET_TRIPLET=x64-windows-static ^
             -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded ^
             -G "Visual Studio 17 2022" -A x64
) else (
    cmake .. -DCMAKE_TOOLCHAIN_FILE=..\vcpkg\scripts\buildsystems\vcpkg.cmake ^
             -DVCPKG_TARGET_TRIPLET=x64-windows-static ^
             -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded ^
             -G "Visual Studio 16 2019" -A x64
)

if %errorlevel% neq 0 (
    echo CMake configuration failed!
    pause
    exit /b 1
)

echo.
echo Building Release version...
cmake --build . --config Release

if %errorlevel% equ 0 (
    echo.
    echo =====================================
    echo Standalone build successful!
    echo =====================================
    echo.
    echo Executable: Release\KKCOM_CPP.exe
    echo Size check:
    dir Release\KKCOM_CPP.exe
    echo.
    echo This executable should run without any DLL dependencies!
    echo.
    echo Testing executable...
    Release\KKCOM_CPP.exe --help 2>nul
    if %errorlevel% equ 0 (
        echo Executable runs successfully!
    ) else (
        echo Note: Executable may need to be run in GUI environment
    )
    pause
) else (
    echo.
    echo Build failed! Check the error messages above.
    pause
    exit /b 1
)

cd ..