# KKCOM C++ Edition

A modern C++ implementation of KKCOM serial communication tool with ImGui interface.

## Features

- **Dual Mode Build System**
  - Debug mode: Console window with detailed logging
  - Release mode: Clean GUI without console
- **Embedded Resources**
  - Application icon and splash screen embedded in executable
  - Single .exe file deployment
- **Serial Communication**
  - Multiple COM port support
  - Configurable baud rates
  - Real-time data reception
- **Custom Commands**
  - EXT command tabs
  - Save/load command configurations
  - Toggle send functionality

## Building

### Debug Mode (with console)
```bash
build_debug.bat
```
Executable: `build_debug/Debug/KKCOM_CPP.exe`

### Release Mode (GUI only)
```bash
build_release.bat
```
Executable: `build_release/Release/KKCOM_CPP.exe`

## Requirements

- Visual Studio 2019 BuildTools or later
- CMake 3.16+
- vcpkg (included)

## Dependencies

All dependencies are managed through vcpkg:
- GLFW3 (static)
- nlohmann/json (static)
- Dear ImGui (bundled)
- stb_image (bundled)

## Project Structure

```
KKCOM_CPP/
├── build_debug.bat     # Debug build script
├── build_release.bat   # Release build script
├── CMakeLists.txt      # Main CMake configuration
├── KKCOM.rc           # Windows resource file
├── include/           # Header files
├── src/              # Source files
├── libs/             # Third-party libraries
├── vcpkg/            # Package manager
└── assets/           # Resource files (if needed)
```

## Configuration

The application supports both debug and release configurations:

- **Debug Mode**: `DEBUG_MODE=1` - Shows console window with detailed logging
- **Release Mode**: `DEBUG_MODE=0` - Clean GUI application without console

## License

See the main KKCOM project for license information.