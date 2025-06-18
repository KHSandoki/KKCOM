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