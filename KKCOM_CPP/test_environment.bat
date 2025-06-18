@echo off
REM Test compilation environment for KKCOM C++

echo Testing Windows compilation environment...
echo.

REM Test Visual Studio compiler
echo [1/4] Checking Visual Studio compiler...
where cl >nul 2>nul
if %errorlevel% equ 0 (
    echo ✓ Visual Studio compiler found
    cl 2>&1 | findstr "Microsoft" >nul
    if %errorlevel% equ 0 (
        echo ✓ Compiler version check passed
    )
) else (
    echo ✗ Visual Studio compiler NOT found
    echo   Please run from Developer Command Prompt for VS
    goto :end_test
)

REM Test source files
echo [2/4] Checking source files...
if exist "src\SerialManager.cpp" (
    echo ✓ SerialManager.cpp found
) else (
    echo ✗ SerialManager.cpp missing
    goto :end_test
)

if exist "src\test_serial.cpp" (
    echo ✓ test_serial.cpp found
) else (
    echo ✗ test_serial.cpp missing
    goto :end_test
)

if exist "include\SerialManager.h" (
    echo ✓ SerialManager.h found
) else (
    echo ✗ SerialManager.h missing
    goto :end_test
)

REM Test Git (optional)
echo [3/4] Checking Git (optional)...
where git >nul 2>nul
if %errorlevel% equ 0 (
    echo ✓ Git found (can download dependencies)
) else (
    echo ⚠ Git not found (manual download needed for GUI version)
)

REM Test CMake (optional)
echo [4/4] Checking CMake (optional)...
where cmake >nul 2>nul
if %errorlevel% equ 0 (
    echo ✓ CMake found (can use build scripts)
    cmake --version | findstr "cmake"
) else (
    echo ⚠ CMake not found (manual compilation only)
)

echo.
echo Environment test completed!
echo.
echo Recommended next steps:
echo 1. For quick test: compile_manual.bat
echo 2. For GUI version: setup_vcpkg.bat then build_vcpkg.bat
echo.

:end_test
pause