# KKCOM Program Icons

This folder contains several icon designs for the KKCOM serial communication program.

## Icon Files

### 1. `icon_design.svg` - Detailed Version (256x256)
- **Use case**: Main application icon, desktop shortcuts, high-resolution displays
- **Features**: 
  - Terminal screen with green text representing COM data
  - COM port connector with pins
  - Signal waves indicating communication
  - Program name at bottom
  - Green gradient background representing connectivity

### 2. `icon_simple.svg` - Simplified Version (64x64)
- **Use case**: Small icons, taskbar, system tray
- **Features**:
  - Simplified terminal and COM connector
  - Optimized for small sizes
  - Clear visibility at low resolutions

### 3. `icon_modern.svg` - Modern Flat Design (256x256)
- **Use case**: Modern UI applications, web interfaces
- **Features**:
  - Flat design with rounded corners
  - Terminal window with syntax highlighting colors
  - Animated signal indicators
  - Contemporary color scheme

## Design Elements

### Color Scheme
- **Primary Green**: `#2E7D32` (Dark Green) - Represents connectivity and success
- **Accent Green**: `#00E676` (Bright Green) - Terminal text and signals
- **Dark Gray**: `#1A1A1A` to `#424242` - Terminal/hardware elements
- **Gold**: `#FFD54F` - COM port pins (representing electrical connections)

### Symbolism
- **Terminal Screen**: Represents the main GUI interface
- **Green Text**: Classic terminal/console appearance
- **COM Connector**: Physical serial port representation
- **Signal Waves**: Data transmission and communication
- **Pins**: Electrical connections and hardware interface

## Converting to ICO Format

To create Windows ICO files from these SVG designs:

### Method 1: Using Online Converter
1. Visit https://convertio.co/svg-ico/ or similar service
2. Upload the SVG file
3. Download the ICO file

### Method 2: Using ImageMagick (Command Line)
```bash
# Install ImageMagick first
# Convert SVG to multiple ICO sizes
magick icon_design.svg -define icon:auto-resize=256,128,64,48,32,16 kkcom.ico
```

### Method 3: Using GIMP
1. Open SVG in GIMP
2. Scale to desired size (256x256, 128x128, 64x64, etc.)
3. Export as ICO file with multiple sizes

## Integration with C++ Application

### For Windows Applications:
```cpp
// In your resource file (.rc)
IDI_ICON1 ICON "kkcom.ico"

// In your code
HICON hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
```

### For Qt Applications:
```cpp
QApplication app(argc, argv);
app.setWindowIcon(QIcon(":/icons/kkcom.ico"));
```

### For GTK Applications:
```cpp
gtk_window_set_icon_from_file(GTK_WINDOW(window), "kkcom.png", NULL);
```

## Recommended Sizes

- **16x16**: System tray, small buttons
- **32x32**: Toolbar icons, list views
- **48x48**: Desktop icons (Windows)
- **64x64**: Large toolbar icons
- **128x128**: Application launchers
- **256x256**: High-DPI displays, macOS

## License

These icons are designed specifically for the KKCOM project and are part of the application's visual identity.