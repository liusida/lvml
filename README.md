# lvml user C module for MicroPython

Minimal MicroPython user C module named `lvml` exposing a single function `lvml.hello()` implemented in C.

## Layout

- `micropython.cmake`: Top-level entry to include module(s) when building via CMake.
- `lvml/`
  - `lvmlmodule.c`: Module implementation.
  - `micropython.cmake`: CMake glue for the module.
  - `micropython.mk`: Makefile glue for Make-based ports.
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

## Build (ESP32-S3 using the included MicroPython)

The MicroPython and LVGL dependencies are included as submodules in the `third-party/` directory.

### Fast Build System (Makefile only)

We use a Makefile-only build system for faster builds (no CMake overhead):

```bash
# Build firmware (default: ESP32_GENERIC_S3 with SPIRAM_OCT)
make build

# Build for specific board and variant
make BOARD=ESP32_GENERIC_S3 VARIANT=SPIRAM_OCT build

# Build for different board
make BOARD=ESP32S3_BOX build

# Flash to device
make flash

# Build and flash in one command
make deploy

# Monitor serial output
make monitor

# Clean build
make clean

# Show help
make help
```

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

### Test in MicroPython REPL

```python
import lvml
lvml.hello()
```

Expected output:

```
hello from lvml
```

## ESP32 Boot Initialization

The `py/` directory contains example Python files for ESP32 boot initialization:

### Upload Files to ESP32

1. **Upload main.py** (required):
```bash
ampy --port /dev/ttyUSB0 put py/main.py
```

2. **Upload customer code** (choose one):
```bash
# Option A: user_main.py (recommended)
ampy --port /dev/ttyUSB0 put py/user_main.py

# Option B: user_code.py (direct execution)
ampy --port /dev/ttyUSB0 put py/user_code.py
```

### Boot Sequence

1. ESP32 boots → `main.py` runs automatically
2. System initialization happens first
3. Customer code executes after initialization
4. Ready for operation

See `py/README.md` for detailed usage instructions.
