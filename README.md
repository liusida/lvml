# LVML - XML-Driven UI with MicroPython Scripting for ESP32-S3

A complete modular system that combines LVGL graphics, XML-based UI loading, WiFi connectivity, and MicroPython scripting for ESP32-S3 devices, specifically optimized for ESP32-S3-Box-3 with 16MB PSRAM support.

## Current Development Focus

Based on your current thinking, the main development priorities are:

1. **Initialize BLE keyboard** - Enable Bluetooth Low Energy keyboard functionality
2. **WiFi setting UI** - Create a user interface in `./boot/main.py` to configure WiFi (choose SSID and enter password)
3. **Access URL for XML** - Fetch XML UI definitions from remote servers
4. **Render XML** - Parse and display XML-based user interfaces

The long-term vision is to have server-side XML (mixed with MicroPython) implement the business logic of applications, creating a flexible, remotely-updatable UI system.

## Features

### ✅ Completed (v1.0.0)
- **Modular Architecture** - Clean separation of concerns with pluggable modules
- **LVGL Graphics Library** - Full LVGL v9.3.0 integration with ESP32-S3-Box-3 LCD driver
- **ESP32-S3-Box-3 Optimized** - Configured for 16MB Octal PSRAM with custom partition table
- **Display Buffer Management** - Automatic PSRAM allocation for 320x240 displays
- **Memory Diagnostics** - Comprehensive memory information and monitoring
- **Patch System** - Clean submodule management with automated patches
- **Frozen Modules** - Python files built into firmware
- **Basic UI Elements** - Rectangles, buttons, text areas with color support
- **MicroPython Integration** - Full MicroPython module interface

### 🚧 In Development
- **BLE Keyboard** - Bluetooth Low Energy keyboard functionality
- **WiFi Configuration UI** - Interactive WiFi setup interface
- **XML Parser** - Complete XML parsing and UI rendering
- **Network Manager** - WiFi and HTTP client functionality
- **MicroPython Script Execution** - Runtime script execution from XML

### 📋 Planned Features
- **Complete XML UI Support** - Full LVGL widget support via XML
- **Advanced MicroPython Scripting** - Event handling and object binding
- **Image Loading** - Support for images from URLs and local storage
- **Animation Support** - Transitions and animations
- **Chart Components** - Data visualization widgets

## Architecture

### Modular Design

The LVML system is built with a clean modular architecture:

- `lvml/core/` - Core LVML functionality and LVGL integration
- `lvml/xml/` - XML parsing and UI processing (in development)
- `lvml/network/` - WiFi and HTTP client functionality (in development)
- `lvml/micropython/` - MicroPython script execution engine (in development)
- `lvml/utils/` - Memory management and utility functions
- `lvml/driver/` - Hardware drivers (ESP32-S3-Box-3 LCD)

### Project Layout

```
lvml/
├── lvmlmodule.c          # Main MicroPython module interface
├── lvml.h & lvml.c       # Core LVML system coordination
├── core/                 # Core LVML functionality
│   ├── lvml_core.h/c     # Core system and display management
│   └── lvml_ui.h/c       # UI element creation and management
├── driver/               # Hardware drivers
│   └── esp32_s3_box3_lcd.h/c  # ESP32-S3-Box-3 LCD driver
├── xml/                  # XML parsing (in development)
│   ├── xml_parser.h/c    # XML parsing and processing
│   └── xml_ui.h/c        # XML to LVGL object conversion
├── network/              # Network functionality (in development)
│   ├── network_manager.h/c  # WiFi and HTTP client
│   └── http_client.h/c   # HTTP request handling
├── micropython/          # MicroPython integration (in development)
│   └── mp_executor.h/c   # Script execution engine
└── utils/                # Utility functions
    └── memory_manager.h/c  # Memory management
```

## Setup

### Prerequisites

- ESP-IDF v5.0+ (for ESP32-S3 support)
- Python 3.8+
- Git with submodule support

### Installation

Clone this repository with submodules:

```bash
git clone --recurse-submodules https://github.com/yourusername/lvml.git
cd lvml
```

Or if you already cloned without submodules:

```bash
git submodule update --init --recursive
```

### ESP-IDF Setup

Source the ESP-IDF environment:

```bash
# Adjust path to your ESP-IDF installation
source ~/esp-idf/export.sh
```

## Build (ESP32-S3-Box-3 Optimized)

The build system automatically applies patches and configurations for ESP32-S3-Box-3 with 16MB PSRAM support.

### Quick Build

```bash
# Build firmware (automatically applies patches and PSRAM config)
make build

# Clean all build artifacts
make clean-all

# Show available targets
make help
```

### Build Process

The build system automatically:
1. **Applies LVGL patches** (format string fixes)
2. **Configures PSRAM** (Octal mode for ESP32-S3-Box-3)
3. **Sets up partition table** (16MB flash with large app partition)
4. **Includes frozen modules** (Python files built into firmware)
5. **Builds MicroPython** with LVGL integration

### Manual Build (if needed)

```bash
cd third-party/micropython/ports/esp32
export USER_C_MODULES=/path/to/lvml/lvml/micropython.cmake
make BOARD=ESP32_GENERIC_S3 VARIANT=SPIRAM_OCT USER_C_MODULES=/path/to/lvml/lvml/micropython.cmake all
```

## Usage

### Basic LVML Functions

```python
import lvml

# Initialize LVML (allocates display buffers in PSRAM)
lvml.init()

# Check if LVML is initialized
lvml.is_initialized()

# Get LVML version
print("LVML Version:", lvml.get_version())

# Set background color
lvml.set_bg("red")  # or "blue", "green", "black", "white", or hex like "0xFF0000"

# Create UI elements
lvml.rect(10, 10, 100, 50, "#FF0000", "#000000", 2)  # Red rectangle with black border
lvml.button(50, 100, 120, 40, "Click Me", "#0066CC", "#FFFFFF")  # Blue button
lvml.textarea(10, 150, 200, 80, "Enter text here...", "#FFFFFF", "#000000")  # Text area

# Process LVGL tick (call periodically for animations)
lvml.tick()

# Set display rotation (0=0°, 1=90°, 2=180°, 3=270°)
lvml.set_rotation(1)

# Debug system
lvml.debug(True)  # Shows memory info and test display
```

### Network and XML UI Loading (In Development)

```python
import lvml

# Initialize LVML
lvml.init()

# Connect to WiFi (planned)
lvml.connect_wifi("MyWiFi", "password")

# Load UI from remote XML file (planned)
lvml.load_from_url("http://example.com/ui.xml")

# Or load UI from local XML string (planned)
xml_data = """
<ui>
  <button text="Hello World" x="100" y="100" width="120" height="40">
    <script event="click">
      print("Button clicked!")
    </script>
  </button>
</ui>
"""
lvml.load_from_xml(xml_data)

# Check if all systems are ready (planned)
if lvml.is_ready():
    print("LVML is ready for operation!")
```

### Color Format Support

LVML supports multiple color formats:

```python
# CSS-style hex colors (recommended)
lvml.rect(10, 10, 50, 50, "#FF0000", "#000000", 0)

# Named colors
lvml.rect(10, 70, 50, 50, "red", "black", 0)

# C-style hex colors
lvml.rect(10, 130, 50, 50, 0xFF0000, 0x000000, 0)

# Plain hex strings
lvml.rect(10, 190, 50, 50, "FF0000", "000000", 0)
```

## Hardware Requirements

- **ESP32-S3-Box-3** (recommended) or compatible ESP32-S3 board
- **16MB PSRAM** (Octal mode)
- **16MB Flash** (minimum)
- **320x240 Display** (for LVGL graphics)

## ESP32 Boot Initialization

### Frozen Modules (Built into Firmware)

Python files in the `boot/` directory are automatically built into the firmware as frozen modules:

- `boot/main.py` - Boot initialization and WiFi setup UI (planned)

### Current Boot Process

```python
# boot/main.py - Current simple initialization
import lvml
lvml.init()
```

### Planned WiFi Setup UI

The planned WiFi setup UI in `boot/main.py` will provide:

1. **SSID Selection** - Scan and display available WiFi networks
2. **Password Input** - Secure password entry interface
3. **Connection Status** - Real-time connection feedback
4. **Configuration Storage** - Save WiFi credentials for future use

## Development Status

### Current Implementation

The project currently provides a solid foundation with:

- **Working LVGL Integration** - Full display support with PSRAM optimization
- **Basic UI Elements** - Rectangles, buttons, text areas
- **Memory Management** - Comprehensive PSRAM and internal RAM handling
- **Build System** - Automated patching and configuration
- **MicroPython Module** - Complete Python interface

### In Development

- **BLE Keyboard** - Bluetooth Low Energy keyboard functionality
- **WiFi Configuration** - Interactive setup interface
- **XML Parser** - Complete XML parsing and UI rendering
- **Network Manager** - WiFi and HTTP client functionality

### Architecture Benefits

1. **Modular Design** - Easy to extend and maintain
2. **PSRAM Optimization** - Efficient use of 16MB external RAM
3. **Remote Updates** - XML-based UI allows server-side updates
4. **MicroPython Integration** - Python scripting for business logic
5. **Hardware Abstraction** - Clean separation from hardware specifics

## Troubleshooting

### PSRAM Issues

If PSRAM is not detected:
1. Ensure you're using ESP32-S3-Box-3 or compatible board
2. Check that `CONFIG_SPIRAM_MODE_OCT=y` is set
3. Verify `CONFIG_SPIRAM_IGNORE_NOTFOUND=` (disabled)

### Memory Issues

Use `lvml.debug()` to diagnose memory problems:
- PSRAM should show 16,777,216 bytes (16MB)
- Internal RAM should show ~350KB
- Display buffers require ~230KB total

### Build Issues

```bash
# Clean everything and rebuild
make clean-all
make build
```

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test with ESP32-S3-Box-3
5. Submit a pull request

### Development Areas

- **BLE Keyboard**: Implement Bluetooth Low Energy keyboard functionality
- **WiFi UI**: Create interactive WiFi configuration interface
- **XML Parser**: Complete XML parsing and LVGL object creation
- **Network Module**: Add real WiFi and HTTP client functionality
- **MicroPython Integration**: Complete script execution and LVGL object binding
- **UI Components**: Add support for more LVGL widgets

## License

This project is open source. See individual component licenses:
- MicroPython: MIT License
- LVGL: MIT License

## Roadmap

### Phase 1: Core Foundation ✅
- [x] LVGL integration with ESP32-S3-Box-3
- [x] Basic UI elements (rectangles, buttons, text areas)
- [x] Memory management and PSRAM optimization
- [x] Build system and patching

### Phase 2: Network & Configuration 🚧
- [ ] BLE keyboard functionality
- [ ] WiFi configuration UI
- [ ] Network manager implementation
- [ ] HTTP client for XML fetching

### Phase 3: XML & Scripting 📋
- [ ] Complete XML parser
- [ ] MicroPython script execution
- [ ] Event handling system
- [ ] Advanced UI components

### Phase 4: Advanced Features 📋
- [ ] Image loading and display
- [ ] Animation and transitions
- [ ] Chart and graph components
- [ ] Remote update system

---

**Note**: This project is actively under development. The current implementation provides a solid foundation for XML-driven UIs with MicroPython scripting on ESP32-S3 devices.