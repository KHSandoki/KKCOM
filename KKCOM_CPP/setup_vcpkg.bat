@echo off
REM Setup vcpkg and dependencies for KKCOM C++ Edition

echo Setting up vcpkg and dependencies...

REM Check if vcpkg is already installed
if exist "vcpkg\vcpkg.exe" (
    echo vcpkg already exists, updating...
    cd vcpkg
    git pull
    cd ..
) else (
    echo Cloning vcpkg...
    git clone https://github.com/Microsoft/vcpkg.git
    if %errorlevel% neq 0 (
        echo Failed to clone vcpkg. Make sure git is installed.
        pause
        exit /b 1
    )
)

REM Bootstrap vcpkg
cd vcpkg
if not exist "vcpkg.exe" (
    echo Bootstrapping vcpkg...
    call bootstrap-vcpkg.bat
    if %errorlevel% neq 0 (
        echo Failed to bootstrap vcpkg.
        pause
        exit /b 1
    )
)

REM Install dependencies (static versions)
echo Installing dependencies (static linking)...
vcpkg install glfw3:x64-windows-static
vcpkg install nlohmann-json:x64-windows-static

if %errorlevel% equ 0 (
    echo Dependencies installed successfully!
    echo.
    echo Now you can build with:
    echo build_vcpkg.bat
) else (
    echo Failed to install some dependencies.
    pause
    exit /b 1
)

cd ..
pause