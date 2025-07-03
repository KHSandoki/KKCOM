@echo off
REM Simple build script using existing vcpkg setup

echo ========================================
echo Simple KKCOM Static Build
echo ========================================
echo.

REM Setup Visual Studio 2019 BuildTools environment (detected from previous output)
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
if exist build_simple (
    echo Cleaning previous build...
    rmdir /s /q build_simple
)
mkdir build_simple
cd build_simple

echo.
echo Configuring with CMake...
cmake .. -DCMAKE_TOOLCHAIN_FILE=..\vcpkg\scripts\buildsystems\vcpkg.cmake ^
         -DVCPKG_TARGET_TRIPLET=x64-windows-static ^
         -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded ^
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
echo Building Release version...
cmake --build . --config Release --verbose

if %errorlevel% equ 0 (
    echo.
    echo ========================================
    echo Build Successful!
    echo ========================================
    echo.
    echo Executable: Release\KKCOM_CPP.exe
    
    echo.
    echo Checking dependencies...
    dumpbin /dependents Release\KKCOM_CPP.exe 2>nul | findstr /i "glfw" && (
        echo WARNING: Still has GLFW dependency!
    ) || (
        echo SUCCESS: No GLFW dependency found!
    )
    
    echo.
    echo File size:
    dir Release\KKCOM_CPP.exe
    
) else (
    echo.
    echo Build failed! Check error messages above.
    pause
    exit /b 1
)

cd ..
pause