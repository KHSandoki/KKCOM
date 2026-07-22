# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

KKCOM is a C++ serial communication GUI application built with Dear ImGui + GLFW + OpenGL.
It provides a terminal-like interface for serial port communication: sending commands,
filtering and coloring received data, HEX I/O, and managing custom command sets.

(A legacy Python/tkinter edition previously lived here; it has been removed and the C++
edition now lives at the repository root.)

## Project Structure

```
CMakeLists.txt        # CMake build configuration
KKCOM.rc              # Windows resource file (icon + version info)
build_debug.bat       # Debug build script
build_release.bat     # Release build script
include/              # Headers (SerialApp.h, SerialManager.h, ConfigManager.h, version.h, ...)
src/                  # Sources (main.cpp, SerialApp.cpp, SerialManager.cpp, ConfigManager.cpp, ...)
third_party/textselect/  # Vendored ImGuiTextSelect (locally patched)
image/                # Icon / splash assets
libs/                 # Dear ImGui (gitignored, fetched at build time)
vcpkg/                # vcpkg (gitignored)
```

### Core Components

- **SerialApp** (`src/SerialApp.cpp`): main application class — GUI, received-data view
  (ASCII/HEX, timestamps, TX/RX, rule-based + ANSI syntax coloring), command sending.
- **SerialManager** (`src/SerialManager.cpp`): Win32/POSIX serial port I/O, the receive
  thread, disconnect detection.
- **ConfigManager** (`src/ConfigManager.cpp`): loads/saves `config.json` (EXT groups,
  pinned commands, coloring rules, line ending, etc.).

### Dependencies

- Dear ImGui + GLFW + OpenGL (UI), nlohmann/json (config), ImGuiTextSelect (text selection).
- glfw3 and nlohmann-json come from vcpkg (`x64-windows-static`). Dear ImGui is fetched into
  `libs/imgui` at build time. ImGuiTextSelect is vendored under `third_party/`.

## Serial port hand-off (automation)

On Windows a COM port is exclusive: while KKCOM is connected no other program can open
it. To let an external tool (a script, or an automation agent like Claude Code) use the
port, KKCOM watches for a trigger file and releases the port when it appears:

- **Trigger file:** `kkcom_release.request` in KKCOM's working directory (same folder as
  `config.json`). The exact path is shown under **File → Trigger file...** (with a Copy
  path button).
- Create that file (any/empty contents) → KKCOM disconnects, deletes the file to
  acknowledge, and writes `kkcom_release.status` (JSON: `released`, `port`, `ts`). Poll
  for the request file to disappear (or read the status file) to confirm before opening
  the port from your tool.
- **Reconnect is manual** — click **Connect** in KKCOM when you're done. By design KKCOM
  never auto-reconnects, so it won't grab the port back while your tool is still using it.
- The watcher can be toggled under **File → Release COM on trigger file** (persisted as
  `comReleaseWatch` in `config.json`). Implemented in `SerialApp::comReleaseWatchLoop`
  (detects the file on a background thread) and `SerialApp::handleComReleaseRequest`
  (does the disconnect on the UI thread).

## Build (Release)

Prerequisites: Visual Studio 2022 BuildTools, CMake, vcpkg (set up in `vcpkg/`).

If `build_release/` already exists (incremental build):
```bash
cd build_release
cmake --build . --config Release
# Output: build_release/Release/KKCOM_CPP.exe
```

For a clean build, delete `build_release/` then run `build_release.bat`.

## CI

`.github/workflows/build-cpp.yml` builds on a `windows-2022` runner: it fetches Dear ImGui
into `libs/imgui`, installs glfw3 + nlohmann-json via vcpkg, configures with the
`Visual Studio 17 2022` generator, builds Release, and uploads `KKCOM_CPP.exe` as an artifact.
Pushing a `v*` tag also attaches the exe to the corresponding GitHub Release.

## Versioning

Version is defined in `include/version.h`:
```cpp
#define KKCOM_VERSION_BUILD 9
#define KKCOM_VERSION_STRING "1.0.5"
#define KKCOM_VERSION_STRING_FULL "1.0.5"
```

Update these (and the version fields in `KKCOM.rc`) before a new release. The window title
reads `KKCOM_VERSION_STRING_FULL` at runtime.

## Release to GitHub

1. Update `include/version.h` (and `KKCOM.rc`).
2. Commit the version bump.
3. Tag and push: `git tag vX.X.X && git push origin <branch> && git push origin vX.X.X`.
4. The CI workflow builds the tagged commit and attaches `KKCOM_CPP.exe` to the GitHub Release.
