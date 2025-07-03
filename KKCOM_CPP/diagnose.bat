@echo off
echo ========================================
echo KKCOM Build Environment Diagnosis
echo ========================================
echo.

echo [1] Checking Visual Studio...
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\Common7\Tools\VsDevCmd.bat" (
    echo ✓ VS2019 BuildTools found
) else (
    echo ✗ VS2019 BuildTools not found
)

echo.
echo [2] Checking CMake...
where cmake >nul 2>&1
if %errorlevel% equ 0 (
    echo ✓ CMake found in PATH
    cmake --version
) else (
    echo ✗ CMake not found in PATH
    if exist "C:\Program Files\CMake\bin\cmake.exe" (
        echo ✓ CMake found in Program Files
    ) else (
        echo ✗ CMake not installed
    )
)

echo.
echo [3] Checking vcpkg setup...
if exist "vcpkg\vcpkg.exe" (
    echo ✓ vcpkg found
) else (
    echo ✗ vcpkg not found
)

echo.
echo [4] Checking static libraries...
if exist "vcpkg\installed\x64-windows-static\lib\glfw3.lib" (
    echo ✓ Static GLFW3 library found
    dir "vcpkg\installed\x64-windows-static\lib\glfw3.lib"
) else (
    echo ✗ Static GLFW3 library not found
)

if exist "vcpkg\installed\x64-windows-static\include\nlohmann\json.hpp" (
    echo ✓ Static nlohmann-json found
) else (
    echo ✗ Static nlohmann-json not found
)

echo.
echo [5] Checking current build...
if exist "build_vcpkg\Release\KKCOM_CPP.exe" (
    echo ✓ Current executable found
    echo File size:
    dir "build_vcpkg\Release\KKCOM_CPP.exe"
    echo.
    echo Dependencies check:
    dumpbin /dependents "build_vcpkg\Release\KKCOM_CPP.exe" 2>nul | findstr /i "\.dll" || echo No DLL dependencies found
) else (
    echo ✗ No current executable found
)

echo.
echo [6] Recommendations...
if not exist "vcpkg\installed\x64-windows-static\lib\glfw3.lib" (
    echo - Run: build_standalone.bat to install static libraries
)

where cmake >nul 2>&1
if %errorlevel% neq 0 (
    echo - Install CMake from https://cmake.org/download/
    echo - Make sure to check "Add CMake to system PATH" during installation
)

echo.
echo [7] Quick fix commands...
echo To install CMake: winget install Kitware.CMake
echo To rebuild with static: build_simple.bat
echo.
pause