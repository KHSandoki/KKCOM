@echo off
REM One-click install and run script for KKCOM C++

title KKCOM C++ - One-Click Setup

echo ==========================================
echo    KKCOM C++ Edition - Windows Setup
echo ==========================================
echo.

REM Check environment first
echo Step 1: Testing environment...
call test_environment.bat >nul 2>nul

REM Check if Visual Studio compiler is available
where cl >nul 2>nul
if %errorlevel% neq 0 (
    echo.
    echo ERROR: Visual Studio compiler not found!
    echo.
    echo Please install Visual Studio Community and run this from:
    echo "Developer Command Prompt for VS"
    echo.
    echo Get Visual Studio: https://visualstudio.microsoft.com/vs/community/
    pause
    exit /b 1
)

echo ✓ Environment check passed
echo.

REM Choose build method
echo Step 2: Choose build method
echo.
echo [1] Quick Test (Console version, no dependencies)
echo [2] Full GUI Version (requires internet for dependencies)
echo [3] Exit
echo.
set /p choice="Enter your choice (1-3): "

if "%choice%"=="1" goto quick_test
if "%choice%"=="2" goto full_gui
if "%choice%"=="3" goto exit
echo Invalid choice, defaulting to Quick Test...

:quick_test
echo.
echo Building Quick Test version...
call compile_manual.bat

if exist "manual_build\KKCOM_Serial_Test.exe" (
    echo.
    echo Success! Running the test...
    echo.
    manual_build\KKCOM_Serial_Test.exe
) else (
    echo Build failed. Please check the errors above.
    pause
)
goto exit

:full_gui
echo.
echo Building Full GUI version...
echo This will download dependencies from the internet.
echo.
set /p confirm="Continue? (y/n): "
if /i not "%confirm%"=="y" goto exit

echo.
echo Setting up dependencies...
call setup_vcpkg.bat

if %errorlevel% neq 0 (
    echo Dependency setup failed.
    pause
    goto exit
)

echo.
echo Building GUI application...
call build_vcpkg.bat

if exist "build_vcpkg\Release\KKCOM_CPP.exe" (
    echo.
    echo Success! Starting KKCOM GUI...
    echo.
    build_vcpkg\Release\KKCOM_CPP.exe
) else (
    echo GUI build failed. Try the Quick Test version instead.
    pause
)
goto exit

:exit
echo.
echo Thank you for using KKCOM C++!
pause