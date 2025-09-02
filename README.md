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

### ESP-IDF (CMake) ports

Example for ESP32-S3 (adjust board as needed):

```bash
cd third-party/micropython/ports/esp32
# If needed: source the ESP-IDF env first
# . ./idf.sh

idf.py -DUSER_C_MODULES=/Users/star/Projects/lvml/micropython.cmake -DMICROPY_BOARD=ESP32S3_BOX3 build
```

If your board name differs, list available boards under `ports/esp32/boards` and pick the closest (e.g. `ESP32S3_BOX`).

### Unix port (sanity check)

```bash
make -C third-party/micropython/ports/unix \
  USER_C_MODULES=/Users/star/Projects/lvml/micropython.cmake \
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
