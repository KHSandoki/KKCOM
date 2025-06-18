@echo off
REM Manual compilation script for Windows (no CMake required)

echo Manual compilation for KKCOM Serial Test...

REM Check if Visual Studio is available
where cl >nul 2>nul
if %errorlevel% neq 0 (
    echo Visual Studio compiler not found.
    echo Please run this from a Visual Studio Developer Command Prompt.
    echo.
    echo To get Visual Studio Developer Command Prompt:
    echo 1. Install Visual Studio Community (free)
    echo 2. Install "Desktop development with C++" workload
    echo 3. Open "Developer Command Prompt for VS" from Start menu
    pause
    exit /b 1
)

REM Create output directory
if not exist manual_build mkdir manual_build

echo Compiling KKCOM Serial Test (console version)...

REM Compile the serial test version
cl /EHsc /std:c++17 /I include ^
   src\test_serial.cpp src\SerialManager.cpp ^
   /Fe:manual_build\KKCOM_Serial_Test.exe ^
   /link winmm.lib ws2_32.lib setupapi.lib

if %errorlevel% equ 0 (
    echo.
    echo Compilation successful!
    echo Executable: manual_build\KKCOM_Serial_Test.exe
    echo.
    echo This test program will:
    echo - List available COM ports
    echo - Try to connect to the first available port
    echo - Send test commands
    echo - Display received data
    echo.
    echo Run it with: manual_build\KKCOM_Serial_Test.exe
) else (
    echo.
    echo Compilation failed!
    echo Please check the error messages above.
)

pause