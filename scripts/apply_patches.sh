#!/bin/bash

# Script to apply patches to submodules before building
# Usage: ./scripts/apply_patches.sh [cleanup]
#   - Without arguments: applies patches and configs
#   - With 'cleanup': restores original configurations

set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PATCHES_DIR="$PROJECT_ROOT/patches"

# Check if cleanup was requested
if [ "$1" = "cleanup" ]; then
    echo "Cleaning up patches and configurations..."
    
    # Restore original board configuration
    BOARD_CONFIG="$PROJECT_ROOT/third-party/micropython/ports/esp32/boards/ESP32_GENERIC_S3/sdkconfig.board"
    if [ -f "$BOARD_CONFIG.backup" ]; then
        mv "$BOARD_CONFIG.backup" "$BOARD_CONFIG"
        echo "Board configuration restored"
    fi
    
    # Restore original mpconfigboard.h
    BOARD_HEADER="$PROJECT_ROOT/third-party/micropython/ports/esp32/boards/ESP32_GENERIC_S3/mpconfigboard.h"
    if [ -f "$BOARD_HEADER.backup" ]; then
        mv "$BOARD_HEADER.backup" "$BOARD_HEADER"
        echo "Board header file restored"
    fi
    
    # Remove copied partition table
    if [ -f "$PROJECT_ROOT/third-party/micropython/ports/esp32/partitions-16MiB-large-app.csv" ]; then
        rm "$PROJECT_ROOT/third-party/micropython/ports/esp32/partitions-16MiB-large-app.csv"
        echo "Custom partition table removed"
    fi
    
    # Remove copied manifest file
    if [ -f "$PROJECT_ROOT/third-party/micropython/ports/esp32/boards/ESP32_GENERIC_S3/manifest.py" ]; then
        rm "$PROJECT_ROOT/third-party/micropython/ports/esp32/boards/ESP32_GENERIC_S3/manifest.py"
        echo "Custom manifest file removed"
    fi
    
    # Restore original CMake file
    CMAKE_FILE="$PROJECT_ROOT/third-party/micropython/ports/esp32/boards/ESP32_GENERIC_S3/mpconfigboard.cmake"
    if [ -f "$CMAKE_FILE.backup" ]; then
        mv "$CMAKE_FILE.backup" "$CMAKE_FILE"
        echo "CMake file restored"
    fi
    

    
    echo "Cleanup completed"
    exit 0
fi

echo "Applying patches to submodules..."

# Apply LVGL format fix patch
if [ -f "$PATCHES_DIR/lvgl_format_fix.patch" ]; then
    echo "Applying LVGL format fix patch..."
    cd "$PROJECT_ROOT/third-party/lvgl"
    if git apply --check "$PATCHES_DIR/lvgl_format_fix.patch" 2>/dev/null; then
        git apply "$PATCHES_DIR/lvgl_format_fix.patch"
        echo "LVGL format fix patch applied successfully"
    else
        echo "LVGL format fix patch already applied or incompatible"
    fi
    cd "$PROJECT_ROOT"
fi

# Copy custom mpconfigboard.h for JTAG configuration
if [ -f "$PATCHES_DIR/mpconfigboard.h" ]; then
    echo "Copying custom mpconfigboard.h for JTAG configuration..."
    BOARD_HEADER="$PROJECT_ROOT/third-party/micropython/ports/esp32/boards/ESP32_GENERIC_S3/mpconfigboard.h"
    if [ -f "$BOARD_HEADER" ]; then
        # Create backup
        cp "$BOARD_HEADER" "$BOARD_HEADER.backup"
        # Copy custom header
        cp "$PATCHES_DIR/mpconfigboard.h" "$BOARD_HEADER"
        echo "Custom mpconfigboard.h copied successfully"
    else
        echo "Warning: Board header file not found: $BOARD_HEADER"
    fi
fi

# Copy custom partition table to MicroPython directory
if [ -f "$PATCHES_DIR/partitions-16MiB-large-app.csv" ]; then
    echo "Copying custom partition table..."
    cp "$PATCHES_DIR/partitions-16MiB-large-app.csv" "$PROJECT_ROOT/third-party/micropython/ports/esp32/"
    echo "Custom partition table copied successfully"
fi

# Copy custom board configuration
if [ -f "$PATCHES_DIR/sdkconfig.board" ]; then
    echo "Copying custom board configuration..."
    BOARD_CONFIG="$PROJECT_ROOT/third-party/micropython/ports/esp32/boards/ESP32_GENERIC_S3/sdkconfig.board"
    if [ -f "$BOARD_CONFIG" ]; then
        # Create backup
        cp "$BOARD_CONFIG" "$BOARD_CONFIG.backup"
        # Copy custom config
        cp "$PATCHES_DIR/sdkconfig.board" "$BOARD_CONFIG"
        echo "Board configuration copied successfully"
    else
        echo "Warning: Board configuration file not found: $BOARD_CONFIG"
    fi
fi

# Copy custom manifest file
if [ -f "$PATCHES_DIR/manifest.py" ]; then
    echo "Copying custom manifest file..."
    MANIFEST_FILE="$PROJECT_ROOT/third-party/micropython/ports/esp32/boards/ESP32_GENERIC_S3/manifest.py"
    cp "$PATCHES_DIR/manifest.py" "$MANIFEST_FILE"
    echo "Manifest file copied successfully"
fi

# Copy custom CMake configuration for frozen manifest
if [ -f "$PATCHES_DIR/mpconfigboard.cmake" ]; then
    echo "Copying custom mpconfigboard.cmake for frozen manifest..."
    CMAKE_FILE="$PROJECT_ROOT/third-party/micropython/ports/esp32/boards/ESP32_GENERIC_S3/mpconfigboard.cmake"
    if [ -f "$CMAKE_FILE" ]; then
        # Create backup
        cp "$CMAKE_FILE" "$CMAKE_FILE.backup"
        # Copy custom CMake file
        cp "$PATCHES_DIR/mpconfigboard.cmake" "$CMAKE_FILE"
        echo "Custom mpconfigboard.cmake copied successfully"
    else
        echo "Warning: CMake file not found: $CMAKE_FILE"
    fi
fi



echo "All patches and configurations applied successfully"
