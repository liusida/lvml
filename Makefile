# Makefile for lvml MicroPython firmware
# Fast build system using make only (no CMake overhead)

# Project configuration
PROJECT_ROOT := $(shell pwd)
MICROPYTHON_DIR := $(PROJECT_ROOT)/third-party/micropython
BUILD_DIR := $(PROJECT_ROOT)/build
BOARD ?= ESP32_GENERIC_S3
VARIANT ?= SPIRAM_OCT
PORT ?= /dev/ttyUSB0

# Colors for output
RED := \033[0;31m
GREEN := \033[0;32m
YELLOW := \033[1;33m
BLUE := \033[0;34m
NC := \033[0m

# Logging functions
define log_info
	@echo -e "$(BLUE)[INFO]$(NC) $(1)"
endef

define log_success
	@echo -e "$(GREEN)[SUCCESS]$(NC) $(1)"
endef

define log_error
	@echo -e "$(RED)[ERROR]$(NC) $(1)"
endef

.PHONY: help build flash clean monitor erase check-deps

# Default target
help:
	@echo "Available targets:"
	@echo "  build      - Build firmware for specified board"
	@echo "  flash      - Flash firmware to ESP32 device"
	@echo "  erase      - Erase ESP32 flash memory"
	@echo "  monitor    - Monitor serial output"
	@echo "  clean      - Clean build directories"
	@echo "  check-deps - Check dependencies"
	@echo ""
	@echo "Variables:"
	@echo "  BOARD      - Target board (default: ESP32_GENERIC_S3)"
	@echo "  VARIANT    - Board variant (default: SPIRAM_OCT)"
	@echo "  PORT       - Serial port (default: /dev/ttyUSB0)"
	@echo ""
	@echo "Examples:"
	@echo "  make build"
	@echo "  make BOARD=ESP32_GENERIC_S3 VARIANT=SPIRAM_OCT build"
	@echo "  make BOARD=ESP32S3_BOX flash"
	@echo "  make PORT=/dev/ttyACM0 monitor"

# Check dependencies
check-deps:
	$(call log_info,Checking dependencies...)
	@if [ ! -d "$(MICROPYTHON_DIR)" ]; then \
		$(call log_error,MicroPython submodule not found. Run: git submodule update --init --recursive); \
		exit 1; \
	fi
	@if [ ! -d "$(MICROPYTHON_DIR)/ports/esp32" ]; then \
		$(call log_error,ESP32 port not found in MicroPython); \
		exit 1; \
	fi
	@if [ -z "$$IDF_PATH" ]; then \
		$(call log_error,ESP-IDF not found. Please source ESP-IDF environment first:); \
		$(call log_error,  source $$IDF_PATH/export.sh); \
		exit 1; \
	fi
	$(call log_success,All dependencies found)

# Build mpy-cross (required for some ports)
build-mpy-cross:
	$(call log_info,Building mpy-cross...)
	@cd $(MICROPYTHON_DIR) && make -C mpy-cross
	$(call log_success,mpy-cross built successfully)

# Build firmware using make (faster than CMake)
build: check-deps build-mpy-cross
	$(call log_info,Building firmware for board: $(BOARD) variant: $(VARIANT))
	@mkdir -p $(BUILD_DIR)
	@cd $(MICROPYTHON_DIR)/ports/esp32 && \
		export USER_C_MODULES=$(PROJECT_ROOT)/lvml && \
		make BOARD=$(BOARD) VARIANT=$(VARIANT) USER_C_MODULES=$(PROJECT_ROOT)/lvml all
	@cp $(MICROPYTHON_DIR)/ports/esp32/build-$(BOARD)-$(VARIANT)/firmware.bin $(BUILD_DIR)/lvml-$(BOARD)-$(VARIANT).bin 2>/dev/null || \
		cp $(MICROPYTHON_DIR)/ports/esp32/build-$(BOARD)-$(VARIANT)/micropython.bin $(BUILD_DIR)/lvml-$(BOARD)-$(VARIANT).bin
	$(call log_success,Firmware built: $(BUILD_DIR)/lvml-$(BOARD)-$(VARIANT).bin)

# Flash firmware using esptool (faster than idf.py)
flash: check-deps
	$(call log_info,Flashing firmware to ESP32 on port $(PORT)...)
	@cd $(MICROPYTHON_DIR)/ports/esp32 && \
		export USER_C_MODULES=$(PROJECT_ROOT)/lvml && \
		make BOARD=$(BOARD) VARIANT=$(VARIANT) USER_C_MODULES=$(PROJECT_ROOT)/lvml deploy PORT=$(PORT)
	$(call log_success,Firmware flashed successfully)

# Erase flash using esptool
erase:
	$(call log_info,Erasing ESP32 flash on port $(PORT)...)
	@cd $(MICROPYTHON_DIR)/ports/esp32 && \
		esptool.py --port $(PORT) erase_flash
	$(call log_success,Flash erased successfully)

# Monitor serial output
monitor:
	$(call log_info,Starting serial monitor on port $(PORT)...)
	@cd $(MICROPYTHON_DIR)/ports/esp32 && \
		picocom $(PORT) -b 115200

# Clean build directories
clean:
	$(call log_info,Cleaning build directories...)
	@cd $(MICROPYTHON_DIR)/ports/esp32 && make clean
	@rm -rf $(BUILD_DIR)
	$(call log_success,Build cleaned)

# Full clean (including mpy-cross)
fullclean: clean
	$(call log_info,Full cleaning...)
	@cd $(MICROPYTHON_DIR) && make -C mpy-cross clean
	$(call log_success,Full clean completed)

# Quick build and flash
deploy: build flash
	$(call log_success,Deploy completed!)

# Build and monitor
build-monitor: build flash monitor

# Show build info
info:
	@echo "Project root: $(PROJECT_ROOT)"
	@echo "MicroPython dir: $(MICROPYTHON_DIR)"
	@echo "Build dir: $(BUILD_DIR)"
	@echo "Target board: $(BOARD)"
	@echo "Board variant: $(VARIANT)"
	@echo "Serial port: $(PORT)"
	@echo "ESP-IDF path: $$IDF_PATH"
