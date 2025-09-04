# Python Files for ESP32

This directory contains example Python files for your ESP32 device.

## Files

- **`main.py`** - Main application entry point (upload to ESP32 root)
- **`user_main.py`** - Example customer application (upload to ESP32 root)
- **`user_code.py`** - Alternative customer code format (upload to ESP32 root)

## Usage

### 1. Upload main.py to ESP32
```bash
# Using ampy
ampy --port /dev/ttyUSB0 put main.py

# Using rshell
rshell -p /dev/ttyUSB0
> cp main.py /pyboard/

# Using Thonny IDE
# Just save main.py to the device root
```

### 2. Upload customer code (choose one)
```bash
# Option A: user_main.py (recommended)
ampy --port /dev/ttyUSB0 put user_main.py

# Option B: user_code.py (direct execution)
ampy --port /dev/ttyUSB0 put user_code.py
```

## Boot Sequence

1. ESP32 boots
2. `main.py` runs automatically
3. System initialization happens
4. Customer code executes
5. Ready for operation

## File Locations on ESP32

```
ESP32 Filesystem:
/
├── main.py          ← Upload this
├── user_main.py     ← Upload this (Option A)
└── user_code.py     ← Upload this (Option B)
```

## Customization

Edit the files to add your specific initialization code and customer application logic.
