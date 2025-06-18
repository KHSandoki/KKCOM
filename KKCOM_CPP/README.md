# KKCOM C++ Edition

A cross-platform serial communication tool built with C++ and Dear ImGui, rewritten from the original Python version.

## Features

- **Serial Communication**: Connect to COM ports with configurable baud rates
- **Real-time Data Display**: View incoming serial data with filtering capabilities
- **Custom Commands**: 300 programmable command buttons across 3 tabs
- **Timed Sending**: Send commands at regular intervals
- **Toggle Sending**: Alternate between two commands with individual timings
- **Configuration Management**: Save/load settings to JSON files

## Dependencies

- C++17 compatible compiler
- CMake 3.16+
- OpenGL
- GLFW3
- nlohmann/json
- Dear ImGui (included as submodule)

## Building

### Linux/macOS

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get install libglfw3-dev libgl1-mesa-dev nlohmann-json3-dev

# Build
mkdir build
cd build
cmake ..
make
```

### Windows

```bash
# Using vcpkg
vcpkg install glfw3 nlohmann-json

# Build with Visual Studio
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]/scripts/buildsystems/vcpkg.cmake
cmake --build .
```

## Setup

1. Clone the repository:
```bash
git clone <repository-url>
cd KKCOM_CPP
```

2. Initialize ImGui submodule:
```bash
git submodule update --init --recursive
```

3. Build the project (see Building section above)

## Usage

1. Run the executable:
```bash
./KKCOM_CPP  # Linux/macOS
KKCOM_CPP.exe  # Windows
```

2. Select COM port and baud rate
3. Click "Connect" to establish connection
4. Use the input field to send commands manually
5. Configure custom commands in the EXT tabs
6. Save configuration using the "Save Config" button

## Configuration

Settings are automatically saved to `config.json` including:
- Custom command configurations
- Last used COM port and baud rate
- Filter settings
- Toggle command settings

## Original Python Version

This C++ version maintains all functionality from the original Python tkinter application while providing:
- Better performance
- Cross-platform compatibility
- Modern UI with Dear ImGui
- Efficient threading model