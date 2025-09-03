# lvml - XML-Driven UI with MicroPython Scripting for ESP32-S3

A complete modular system that combines LVGL graphics, XML-based UI loading, WiFi connectivity, and MicroPython scripting for ESP32-S3 devices, specifically optimized for ESP32-S3-Box-3 with 16MB PSRAM support.

## Features

- ✅ **Modular Architecture** - Clean separation of concerns with pluggable modules
- ✅ **XML-Driven UI** - Load user interfaces from XML files via WiFi or local storage
- ✅ **MicroPython Scripting** - Execute Python scripts within UI elements
- ✅ **LVGL Graphics Library** - Full LVGL v9.3.0 integration with XML support
- ✅ **WiFi & HTTP Client** - Fetch UI definitions and data from remote servers
- ✅ **ESP32-S3-Box-3 Optimized** - Configured for 16MB Octal PSRAM
- ✅ **Display Buffer Management** - Automatic PSRAM allocation for 320x240 displays
- ✅ **Memory Diagnostics** - Comprehensive memory information and monitoring
- ✅ **Patch System** - Clean submodule management with automated patches
- ✅ **Frozen Modules** - Python files built into firmware
- ✅ **Custom Partition Tables** - Optimized flash layout for large applications

## Architecture

### Modular Design

The LVML system is built with a clean modular architecture:

- `lvml/core/` - Core LVML functionality and LVGL integration
- `lvml/xml/` - XML parsing and UI processing
- `lvml/network/` - WiFi and HTTP client functionality
- `lvml/micropython/` - MicroPython script execution engine
- `lvml/utils/` - Memory management and utility functions
- `lvml/driver/` - Hardware drivers (ESP32-S3-Box-3 LCD)

### Project Layout

- `lvml/`
  - `lvmlmodule.c`: Main MicroPython module interface
  - `lvml.h` & `lvml.c`: Core LVML system coordination
  - `micropython.cmake`: CMake configuration for the module
  - `micropython.mk`: Makefile configuration
- `patches/`
  - `mpconfigboard.h`: MicroPython board configuration
  - `sdkconfig.board`: ESP32-S3-Box-3 PSRAM configuration
  - `manifest.py`: Frozen module configuration
- `scripts/`
  - `apply_patches.sh`: Automated patch and configuration system
- `third-party/`
  - `micropython/`: MicroPython v1.24-release (submodule)
  - `lvgl/`: LVGL v9.3.0 (submodule) - includes XML support
- `py/`
  - `main.py`: Example main.py for ESP32 boot initialization
  - `user_main.py`: Example customer application
  - `user_code.py`: Alternative customer code format

## Setup

Clone this repository with submodules:

```bash
git clone --recurse-submodules git@github.com:liusida/lvml.git
cd lvml
```

Or if you already cloned without submodules:

```bash
git submodule update --init --recursive
```

## Build (ESP32-S3-Box-3 Optimized)

The build system automatically applies patches and configurations for ESP32-S3-Box-3 with 16MB PSRAM support.

### Automated Build System

```bash
# Build firmware (automatically applies patches and PSRAM config)
make build

# Clean all build artifacts
make clean-all

# Apply patches manually (if needed)
make apply-patches

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
export USER_C_MODULES=/Users/star/Projects/lvml/lvml/micropython.cmake
make BOARD=ESP32_GENERIC_S3 VARIANT=SPIRAM_OCT USER_C_MODULES=/Users/star/Projects/lvml/lvml/micropython.cmake all
```

### Unix port (sanity check)

```bash
make -C third-party/micropython/ports/unix \
  USER_C_MODULES=/Users/star/Projects/lvml/lvml/micropython.cmake \
  all
third-party/micropython/ports/unix/build-standard/micropython -c "import lvml; lvml.hello()"
```

## Usage

### Basic LVML Functions

```python
import lvml

# Check memory status
lvml.memory_info()

# Initialize LVML (allocates display buffers in PSRAM)
lvml.init()

# Check if LVML is initialized
lvml.is_initialized()

# Get LVML version
print("LVML Version:", lvml.get_version())

# Set background color
lvml.set_bg("red")  # or "blue", "green", "black", "white", or hex like "0xFF0000"
```

### Network and XML UI Loading

```python
import lvml

# Initialize LVML
lvml.init()

# Connect to WiFi
lvml.connect_wifi("MyWiFi", "password")

# Load UI from remote XML file
lvml.load_from_url("http://example.com/ui.xml")

# Or load UI from local XML string
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

# Check if all systems are ready
if lvml.is_ready():
    print("LVML is ready for operation!")
```

### Advanced Features

```python
import lvml

# Set display rotation (0=0°, 1=90°, 2=180°, 3=270°)
lvml.set_rotation(1)

# Call LVGL tick handler (for animations and updates)
lvml.tick()

# Clean up when done
lvml.deinit()
```

## XML UI Format

LVML supports XML-based UI definitions with embedded MicroPython scripting:

```xml
<ui title="My Application">
  <button id="btn1" text="Click Me" x="50" y="50" width="100" height="40">
    <script event="click">
      print("Button clicked!")
      # Access LVGL objects by ID
      btn1.set_text("Clicked!")
    </script>
  </button>
  
  <label id="lbl1" text="Status: Ready" x="50" y="100" width="200" height="30"/>
  
  <slider id="slider1" x="50" y="150" width="200" height="20" min="0" max="100" value="50">
    <script event="value_changed">
      value = slider1.get_value()
      lbl1.set_text(f"Value: {value}")
    </script>
  </slider>
</ui>
```

### Supported UI Elements

- **Buttons** - Clickable buttons with text
- **Labels** - Text display elements
- **Sliders** - Value input controls
- **Images** - Display images from URLs or local storage
- **Containers** - Group and layout elements
- **Charts** - Data visualization components

### MicroPython Scripting

Scripts can be embedded in any UI element and executed on events:

- **click** - Button click events
- **value_changed** - Slider/input value changes
- **long_press** - Long press gestures
- **focus** - Element focus events

### Expected Output

```python
>>> import lvml
>>> lvml.memory_info()
=== Memory Information ===
PSRAM:     Total=16777216 bytes, Free=16777216 bytes, Largest=16777216 bytes
Internal:  Total=  353919 bytes, Free=  274299 bytes, Largest=  180224 bytes
PSRAM usage: 0% used
Internal RAM usage: 20% used
Total system memory: 17131135 bytes, 0% used
========================

>>> lvml.init()
[LVML] Initializing core system v1.0.0
[LVML] Custom delay callback set up
[LVML] Core system initialized successfully
[XML] XML parser initialized successfully
[NETWORK] Network manager initialized successfully
[MP] MicroPython executor initialized successfully
Memory: PSRAM=16777216 bytes, Internal=353919 bytes
Display buffers allocated in PSRAM (115200 bytes each)
ESP32-S3-Box-3 LCD display created successfully!
LVML initialization complete

>>> lvml.connect_wifi("MyWiFi", "password")
[NETWORK] Connecting to WiFi: MyWiFi
[NETWORK] WiFi connected successfully

>>> lvml.load_from_url("http://example.com/ui.xml")
[NETWORK] Fetching URL: http://example.com/ui.xml
[XML] Parsing XML data (1024 bytes)
[MP] Executing 2 MicroPython scripts
LVML UI loaded successfully from URL
```

## Hardware Requirements

- **ESP32-S3-Box-3** (recommended) or compatible ESP32-S3 board
- **16MB PSRAM** (Octal mode)
- **16MB Flash** (minimum)
- **320x240 Display** (for LVGL graphics)

## ESP32 Boot Initialization

### Frozen Modules (Built into Firmware)

Python files in the `py/` directory are automatically built into the firmware as frozen modules:

- `main.py` - Boot initialization
- `user_main.py` - Customer application
- `user_code.py` - Alternative customer code

### Manual Upload (Alternative)

If you prefer to upload files manually:

```bash
# Upload main.py (required)
ampy --port /dev/ttyUSB0 put py/main.py

# Upload customer code
ampy --port /dev/ttyUSB0 put py/user_main.py
```

## Troubleshooting

### PSRAM Issues

If PSRAM is not detected:
1. Ensure you're using ESP32-S3-Box-3 or compatible board
2. Check that `CONFIG_SPIRAM_MODE_OCT=y` is set
3. Verify `CONFIG_SPIRAM_IGNORE_NOTFOUND=` (disabled)

### Memory Issues

Use `lvml.memory_info()` to diagnose memory problems:
- PSRAM should show 16,777,216 bytes (16MB)
- Internal RAM should show ~350KB
- Display buffers require ~230KB total

### Build Issues

```bash
# Clean everything and rebuild
make clean-all
make build
```

## Development Status

### ✅ Completed (v1.0.0)
- Modular architecture with clean interfaces
- Core LVML system with LVGL integration
- ESP32-S3-Box-3 LCD driver
- Memory management utilities
- Build system with automatic file discovery
- MicroPython module interface

### 🚧 In Development
- XML parser implementation
- WiFi and HTTP client functionality
- MicroPython script execution engine
- LVGL object binding system

### 📋 Planned Features
- Complete XML UI element support
- Advanced MicroPython scripting capabilities
- Network data fetching and display
- Image loading from URLs
- Chart and graph components
- Animation and transition support

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test with ESP32-S3-Box-3
5. Submit a pull request

### Development Areas

- **XML Parser**: Implement actual XML parsing and LVGL object creation
- **Network Module**: Add real WiFi and HTTP client functionality
- **MicroPython Integration**: Complete script execution and LVGL object binding
- **UI Components**: Add support for more LVGL widgets
- **Examples**: Create example XML files and applications

## License

This project is open source. See individual component licenses:
- MicroPython: MIT License
- LVGL: MIT License
