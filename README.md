# lvml - MicroPython + LVGL for ESP32-S3

A complete MicroPython user C module that integrates LVGL graphics library for ESP32-S3 devices, specifically optimized for ESP32-S3-Box-3 with 16MB PSRAM support.

## Features

- ✅ **LVGL Graphics Library** - Full LVGL v9.3.0 integration with XML support
- ✅ **ESP32-S3-Box-3 Optimized** - Configured for 16MB Octal PSRAM
- ✅ **Display Buffer Management** - Automatic PSRAM allocation for 320x240 displays
- ✅ **Memory Diagnostics** - Comprehensive memory information and monitoring
- ✅ **Patch System** - Clean submodule management with automated patches
- ✅ **Frozen Modules** - Python files built into firmware
- ✅ **Custom Partition Tables** - Optimized flash layout for large applications

## Layout

- `lvml/`
  - `lvmlmodule.c`: LVGL integration module with memory management
  - `micropython.cmake`: CMake configuration for the module
- `patches/`
  - `lvgl_format_fix.patch`: Fixes LVGL compilation issues
  - `partitions-16MiB-large-app.csv`: Custom partition table for large apps
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

### Basic LVGL Functions

```python
import lvml

# Check memory status
lvml.memory_info()

# Initialize LVGL (allocates display buffers in PSRAM)
lvml.init()

# Check if LVGL is initialized
lvml.is_initialized()

# Set dark background
lvml.set_dark_bg()
```

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
Initializing LVGL...
Memory: PSRAM=16777216 bytes, Internal=353919 bytes
Display buffers allocated in PSRAM (115200 bytes each)
LVGL initialization complete
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

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test with ESP32-S3-Box-3
5. Submit a pull request

## License

This project is open source. See individual component licenses:
- MicroPython: MIT License
- LVGL: MIT License
