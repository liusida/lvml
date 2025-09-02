# lvml user C module for MicroPython

Minimal MicroPython user C module named `lvml` exposing a single function `lvml.hello()` implemented in C.

## Layout

- `micropython.cmake`: Top-level entry to include module(s) when building via CMake.
- `lvml/`
  - `lvmlmodule.c`: Module implementation.
  - `micropython.cmake`: CMake glue for the module.
  - `micropython.mk`: Makefile glue for Make-based ports.

## Build (ESP32-S3 using your local MicroPython tree)

Assuming:
- MicroPython repo at `/Users/star/Projects/github/micropython`
- This repo at `/Users/star/Projects/github/custom_micropy_binding`

### ESP-IDF (CMake) ports

Example for ESP32-S3 (adjust board as needed):

```bash
cd /Users/star/Projects/github/micropython/ports/esp32
# If needed: source the ESP-IDF env first
# . ./idf.sh

idf.py -DUSER_C_MODULES=/Users/star/Projects/github/custom_micropy_binding/micropython.cmake -DMICROPY_BOARD=ESP32S3_BOX3 build
```

If your board name differs, list available boards under `ports/esp32/boards` and pick the closest (e.g. `ESP32S3_BOX`).

### Unix port (sanity check)

```bash
make -C /Users/star/Projects/github/micropython/ports/unix \
  USER_C_MODULES=/Users/star/Projects/github/custom_micropy_binding/micropython.cmake \
  all
/Users/star/Projects/github/micropython/ports/unix/build-standard/micropython -c "import lvml; lvml.hello()"
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
