@echo off
REM Debug build script using existing vcpkg setup

echo ========================================
echo KKCOM Debug Build with Console
echo ========================================
echo.

REM Setup Visual Studio 2019 BuildTools environment
echo Setting up Visual Studio 2019 BuildTools environment...
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\Common7\Tools\VsDevCmd.bat" -arch=x64

if %errorlevel% neq 0 (
    echo Failed to setup Visual Studio environment!
    pause
    exit /b 1
)

REM Check if static libraries are available
if not exist "vcpkg\installed\x64-windows-static\lib\glfw3.lib" (
    echo Error: Static GLFW3 library not found!
    echo Please run setup first with: build_standalone.bat
    pause
    exit /b 1
)

echo Static libraries found.

REM Clean and recreate build directory
if exist build_debug (
    echo Cleaning previous build...
    rmdir /s /q build_debug
)
mkdir build_debug
cd build_debug

echo.
echo Configuring with CMake (Debug Mode)...
cmake .. -DCMAKE_TOOLCHAIN_FILE=..\vcpkg\scripts\buildsystems\vcpkg.cmake ^
         -DVCPKG_TARGET_TRIPLET=x64-windows-static ^
         -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded ^
         -DENABLE_DEBUG_MODE=ON ^
         -G "Visual Studio 16 2019" -A x64

if %errorlevel% neq 0 (
    echo CMake configuration failed!
    echo.
    echo Troubleshooting steps:
    echo 1. Make sure CMake is installed and in PATH
    echo 2. Check Visual Studio installation
    echo 3. Verify vcpkg setup
    pause
    exit /b 1
)

echo.
echo Building Debug version with console...
cmake --build . --config Debug --verbose

if %errorlevel% equ 0 (
    echo.
    echo ========================================
    echo Debug Build Successful!
    echo ========================================
    echo.
    echo Executable: Debug\KKCOM_CPP.exe
    echo This version will show a console window for debugging.
    
    echo.
    echo File size:
    dir Debug\KKCOM_CPP.exe
    
) else (
    echo.
    echo Build failed! Check error messages above.
    pause
    exit /b 1
)

cd ..
pause