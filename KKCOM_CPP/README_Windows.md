# KKCOM C++ Edition - Windows Setup Guide

This is a Windows-specific guide for building and running the KKCOM C++ serial communication tool.

## Prerequisites

### Required
- Windows 7 or later
- Visual Studio 2019 or 2022 (Community Edition is free)
  - Make sure to install "Desktop development with C++" workload

### Optional (for full GUI version)
- Git (for downloading dependencies)
- CMake (for advanced building)

## Quick Start (Console Version)

The easiest way to test the serial communication:

1. **Open Developer Command Prompt**
   - Press `Win + R`, type `cmd`
   - Or search for "Developer Command Prompt for VS" in Start menu

2. **Navigate to project folder**
   ```cmd
   cd path\to\KKCOM_CPP
   ```

3. **Compile and run**
   ```cmd
   compile_manual.bat
   manual_build\KKCOM_Serial_Test.exe
   ```

This will create a console application that tests serial port communication.

## Building Options

### Option 1: Simple Test Version (Recommended)
```cmd
compile_manual.bat
```
- No external dependencies required
- Console-based serial port testing
- Quick to compile and test

### Option 2: Using CMake (Basic)
```cmd
build_test.bat
```
- Builds console test version with CMake
- Requires CMake to be installed

### Option 3: Full GUI Version with vcpkg (Advanced)
```cmd
setup_vcpkg.bat     REM Download and setup dependencies
build_vcpkg.bat     REM Build full GUI version
```
- Downloads Dear ImGui, GLFW, and nlohmann/json automatically
- Creates full GUI application
- Takes longer to setup but provides complete functionality

### Option 4: Manual GUI Setup (Advanced)
1. Download dependencies manually:
   ```cmd
   mkdir libs
   cd libs
   git clone https://github.com/ocornut/imgui.git
   cd ..
   ```

2. Build with CMake:
   ```cmd
   build.bat
   ```

## Usage

### Console Test Version
- Lists available COM ports
- Attempts to connect to first available port
- Sends test AT commands
- Displays received data

### GUI Version (if built)
- Full graphical interface matching original Python version
- 300 programmable command buttons
- Real-time data display with filtering
- Configuration save/load functionality

## Troubleshooting

### "Visual Studio compiler not found"
- Install Visual Studio Community with C++ development tools
- Use "Developer Command Prompt for VS" instead of regular command prompt

### "Cannot find header files"
- Make sure you're in the correct project directory
- Check that all source files are present in src/ and include/ folders

### Serial port access issues
- Make sure no other application is using the COM port
- Run as Administrator if needed
- Check that the device is properly connected

### Building GUI version fails
- Ensure all dependencies are downloaded to libs/ folder
- Try the console version first to verify basic functionality
- Check that CMake and Git are properly installed

## Project Structure

```
KKCOM_CPP/
├── src/                    # Source files
│   ├── main.cpp           # Main GUI application
│   ├── test_serial.cpp    # Console test version
│   ├── SerialApp.cpp      # GUI application logic
│   ├── SerialManager.cpp  # Serial communication
│   └── ConfigManager.cpp  # Configuration management
├── include/               # Header files
├── libs/                  # External libraries (auto-downloaded)
├── compile_manual.bat     # Quick compilation script
├── build_test.bat         # CMake test build
├── setup_vcpkg.bat        # Dependency setup
└── build_vcpkg.bat        # Full GUI build
```

## Performance

The C++ version provides significant performance improvements over the original Python version:
- Faster startup time
- Lower memory usage
- More responsive UI
- Better handling of high-speed serial data