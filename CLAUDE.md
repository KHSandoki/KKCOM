# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

KKCOM is a Python-based serial communication GUI application built with tkinter and customtkinter. The application provides a terminal-like interface for serial port communication with features for sending commands, filtering data, and managing custom command sets.

## Running the Application

```bash
python main.py
```

The application requires Python with the following dependencies:
- tkinter (usually bundled with Python)
- customtkinter
- pyserial

## Architecture

### Core Components

- **SerialApp Class**: The main application class that handles the entire GUI and serial communication logic
- **Serial Communication**: Uses pyserial for COM port communication with configurable baudrate
- **Threading**: Implements separate threads for data reception and periodic command sending
- **Data Persistence**: Saves/loads custom commands to/from `data.json`

### Key Features

1. **Serial Communication**
   - COM port selection with auto-refresh
   - Configurable baudrate (300-921600)
   - Real-time data reception and display
   - Data filtering capabilities

2. **Command Management**
   - EXT tabs with customizable command buttons (100 slots in EXT 1)
   - Editable button labels and commands
   - Save/load functionality for command sets
   - Toggle send feature for alternating commands

3. **UI Layout**
   - Main text area for received data display
   - Bottom input section for manual commands
   - Tabbed interface for different command sets
   - Scrollable frames for large command lists

### File Structure

- `main.py`: Single-file application containing all functionality
- `data.json`: Generated file storing custom command configurations (gitignored)

### Threading Architecture

- **Main Thread**: GUI operations and user interactions
- **Receive Thread**: Continuous serial data reception (`receive_data()`)
- **Send Every Thread**: Periodic command sending (`send_every()`)
- **Toggle Send Thread**: Alternating command transmission (`toggle_send()`)

### Data Flow

1. User configures COM port and baudrate
2. Connection established via `connect_port()`
3. Receive thread starts monitoring incoming data
4. Data filtered and displayed in main text area
5. Commands sent via entry field or custom buttons
6. Custom commands saved to JSON for persistence

---

## C++ Edition (KKCOM_CPP)

The C++ rewrite lives in `KKCOM_CPP/` and uses ImGui + GLFW + OpenGL.

### Build (Release)

Prerequisites: Visual Studio 2019, CMake, vcpkg (already set up in `KKCOM_CPP/vcpkg/`)

If `build_release/` already exists (incremental build):
```bash
cd KKCOM_CPP/build_release
cmake --build . --config Release
# Output: KKCOM_CPP/build_release/Release/KKCOM_CPP.exe
```

If a clean build is needed, delete `build_release/` first then run `build_release.bat` (requires interactive terminal with UAC).

### Versioning

Version is defined in `KKCOM_CPP/include/version.h`:
```cpp
#define KKCOM_VERSION_BUILD 6
#define KKCOM_VERSION_STRING "1.0.0"
#define KKCOM_VERSION_STRING_FULL "1.0.0-beta.6"
```

Update these values before building a new release. The window title reads `KKCOM_VERSION_STRING_FULL` at runtime.

### Release to GitHub

1. Update version in `KKCOM_CPP/include/version.h`
2. Commit: `git add KKCOM_CPP/include/version.h && git commit -m "Bump version to vX.X.X-betaX"`
3. Tag: `git tag vX.X.X-betaX && git push origin master && git push origin vX.X.X-betaX`
4. Rebuild: `cd KKCOM_CPP/build_release && cmake --build . --config Release`
5. Create release (requires `gh` CLI, installed at `C:\Program Files\GitHub CLI\gh.exe`):
```bash
"/c/Program Files/GitHub CLI/gh.exe" release create vX.X.X-betaX \
  "KKCOM_CPP/build_release/Release/KKCOM_CPP.exe#KKCOM_CPP.exe" \
  --title "KKCOM C++ Edition vX.X.X-betaX" \
  --notes "Release notes here"
```