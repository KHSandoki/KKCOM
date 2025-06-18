# KKCOM - Serial Communication Tool

<div align="center">
  <img src="icon_design.svg" width="128" height="128" alt="KKCOM Logo">
  
  [![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)](https://github.com/yourusername/KKCOM/releases)
  [![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
  [![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey.svg)](README.md)
</div>

KKCOM is a powerful and user-friendly serial communication application available in both Python and C++ editions. It provides a terminal-like interface for COM port communication with advanced features for command management, data logging, and real-time monitoring.

## 🚀 Features

### Core Functionality
- **Multi-platform serial communication** (Windows, Linux, macOS)
- **Real-time data reception and display** with scrollable terminal interface
- **Configurable baud rates** (300-921600)
- **Custom command management** with 300+ programmable buttons across 3 EXT tabs
- **Toggle send functionality** for alternating command transmission
- **Data filtering** for selective message display
- **Comprehensive logging** with timestamp support

### Advanced Features
- **Resizable UI panels** for optimal workflow
- **Auto-filename generation** based on current timestamp
- **Configuration persistence** (save/load settings)
- **Double-click editing** for command buttons
- **Professional logging** with RX/TX indicators
- **24px Consolas font** for optimal readability
- **Modern tabbed interface** with integrated toggle controls

## 📋 System Requirements

### Python Edition
- Python 3.7+
- tkinter (usually bundled)
- customtkinter
- pyserial

### C++ Edition
- Windows: Visual Studio 2019+ or MinGW
- Linux: GCC 7+ with C++17 support
- macOS: Xcode 10+
- Dependencies: ImGui, GLFW, OpenGL, nlohmann/json, vcpkg (recommended)

## 🛠️ Installation & Usage

### Quick Start (Python Edition)
```bash
# Clone the repository
git clone https://github.com/yourusername/KKCOM.git
cd KKCOM

# Install dependencies
pip install customtkinter pyserial

# Run the application
python main.py
```

### Building C++ Edition
```bash
# Windows with vcpkg
cd KKCOM_CPP
.\setup_vcpkg.bat
.\build_vcpkg.bat

# Linux/macOS
cd KKCOM_CPP
chmod +x build.sh
./build.sh
```

## 📖 User Guide

### Basic Operation
1. **Connect**: Select COM port and baud rate, click Connect
2. **Send Commands**: Type in input field or use custom EXT buttons
3. **Monitor Data**: View real-time reception in main terminal area
4. **Log Sessions**: Enable logging via Log menu for session recording

### Advanced Features
- **Custom Commands**: Double-click EXT buttons to edit names and commands
- **Toggle Send**: Use Toggle tab for alternating command transmission
- **Data Filtering**: Apply filters to display only relevant messages
- **Panel Resize**: Drag splitters to adjust panel heights
- **Auto Logging**: Enable timestamp-based automatic log file naming

### EXT Command Tabs
- **EXT 1-3**: 100 programmable buttons each (300 total)
- **Double-click editing**: Modify button names and commands
- **Persistent storage**: Commands saved automatically
- **Toggle Tab**: Integrated alternating transmission controls

## 🏗️ Project Structure

```
KKCOM/
├── main.py                 # Python edition main application
├── KKCOM_CPP/             # C++ edition source code
│   ├── include/           # Header files
│   ├── src/              # Source files
│   ├── CMakeLists.txt    # Build configuration
│   └── build_vcpkg.bat   # Windows build script
├── icons/                 # Application icons (SVG format)
├── README.md             # This file
├── CLAUDE.md             # Development guidance
└── .gitignore           # Git ignore rules
```

## 🎨 Application Icons

The project includes professionally designed icons in multiple formats:
- **Detailed version** (256x256) for desktop shortcuts
- **Simple version** (64x64) for small UI elements  
- **Modern flat design** with animations for contemporary interfaces

See `README_ICONS.md` for conversion instructions and usage guidelines.

## 🔧 Configuration

### Application Settings
- **COM Port**: Automatically detected available ports
- **Baud Rate**: Standard rates from 300 to 921600
- **Font**: 24px Consolas for optimal readability
- **UI Layout**: Resizable panels with splitter controls

### Logging Configuration
- **File Path**: Custom or auto-generated filenames
- **Timestamp Format**: `YYYY-MM-DD HH:MM:SS.mmm`
- **Data Indicators**: `[RX]` for received, `[TX]` for transmitted
- **Auto-naming**: `com_log_YYYYMMDD_HHMMSS.txt` format

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## 📝 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🏷️ Version History

### v1.0.0-beta.1 (Current)
- Initial release with core functionality
- Python and C++ editions
- Modern UI with resizable panels
- Comprehensive logging system
- 300+ programmable command buttons
- Professional icon set

## 📞 Support

For support, feature requests, or bug reports:
- Open an issue on [GitHub Issues](https://github.com/yourusername/KKCOM/issues)
- Check existing documentation in `CLAUDE.md`

## 🙏 Acknowledgments

- **ImGui** - Immediate mode GUI library for C++ edition
- **CustomTkinter** - Modern tkinter widgets for Python edition
- **vcpkg** - C++ package management
- **Community contributors** - Testing and feedback

---

<div align="center">
  <strong>KKCOM Development Team</strong><br>
  <em>Making serial communication simple and powerful</em>
</div>