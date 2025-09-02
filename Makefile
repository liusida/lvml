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

# Simple logging (no complex functions)

.PHONY: help build clean check-deps init-submodules build-mpy-cross

# Default target
help:
	@echo "Available targets:"
	@echo "  build         - Build firmware for specified board"
	@echo "  clean         - Clean build directories"
	@echo "  check-deps    - Check dependencies"
	@echo "  init-submodules - Initialize MicroPython submodules"
	@echo ""
	@echo "Other targets (commented out for now):"
	@echo "  # flash, erase, monitor, deploy, build-monitor, info"
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
	@printf "$(BLUE)[INFO]$(NC) Checking dependencies...\n"
	@if [ ! -d "$(MICROPYTHON_DIR)" ]; then \
		printf "$(RED)[ERROR]$(NC) MicroPython submodule not found. Run: git submodule update --init --recursive\n"; \
		exit 1; \
	fi
	@if [ ! -d "$(MICROPYTHON_DIR)/ports/esp32" ]; then \
		printf "$(RED)[ERROR]$(NC) ESP32 port not found in MicroPython\n"; \
		exit 1; \
	fi
	@if [ -z "$$IDF_PATH" ]; then \
		printf "$(RED)[ERROR]$(NC) ESP-IDF not found. Please source ESP-IDF environment first:\n"; \
		printf "$(RED)[ERROR]$(NC)   source $$IDF_PATH/export.sh\n"; \
		exit 1; \
	fi
	@printf "$(GREEN)[SUCCESS]$(NC) All dependencies found\n"

# Initialize submodules (required for some features)
init-submodules:
	@printf "$(BLUE)[INFO]$(NC) Initializing MicroPython submodules...\n"
	@cd $(MICROPYTHON_DIR)/ports/esp32 && make BOARD=$(BOARD) submodules
	@printf "$(GREEN)[SUCCESS]$(NC) Submodules initialized\n"

# Build mpy-cross (required for some ports)
build-mpy-cross:
	@printf "$(BLUE)[INFO]$(NC) Building mpy-cross...\n"
	@cd $(MICROPYTHON_DIR) && make -C mpy-cross
	@printf "$(GREEN)[SUCCESS]$(NC) mpy-cross built successfully\n"

# Build firmware using make (faster than CMake)
build: check-deps init-submodules build-mpy-cross
	@printf "$(BLUE)[INFO]$(NC) Building firmware for board: $(BOARD) variant: $(VARIANT)\n"
	@mkdir -p $(BUILD_DIR)
	@cd $(MICROPYTHON_DIR)/ports/esp32 && \
		export USER_C_MODULES=$(PROJECT_ROOT)/lvml/micropython.cmake && \
		make BOARD=$(BOARD) VARIANT=$(VARIANT) USER_C_MODULES=$(PROJECT_ROOT)/lvml/micropython.cmake all
	@cp $(MICROPYTHON_DIR)/ports/esp32/build-$(BOARD)-$(VARIANT)/firmware.bin $(BUILD_DIR)/lvml-$(BOARD)-$(VARIANT).bin 2>/dev/null || \
		cp $(MICROPYTHON_DIR)/ports/esp32/build-$(BOARD)-$(VARIANT)/micropython.bin $(BUILD_DIR)/lvml-$(BOARD)-$(VARIANT).bin
	@printf "$(GREEN)[SUCCESS]$(NC) Firmware built: $(BUILD_DIR)/lvml-$(BOARD)-$(VARIANT).bin\n"

# Flash firmware using esptool (faster than idf.py)
# flash: check-deps
# 	@printf "$(BLUE)[INFO]$(NC) Flashing firmware to ESP32 on port $(PORT)...\n"
# 	@cd $(MICROPYTHON_DIR)/ports/esp32 && \
# 		export USER_C_MODULES=$(PROJECT_ROOT)/lvml/micropython.cmake && \
# 		make BOARD=$(BOARD) VARIANT=$(VARIANT) USER_C_MODULES=$(PROJECT_ROOT)/lvml/micropython.cmake deploy PORT=$(PORT)
# 	@printf "$(GREEN)[SUCCESS]$(NC) Firmware flashed successfully\n"

# Erase flash using esptool
# erase:
# 	@printf "$(BLUE)[INFO]$(NC) Erasing ESP32 flash on port $(PORT)...\n"
# 	@cd $(MICROPYTHON_DIR)/ports/esp32 && \
# 		esptool.py --port $(PORT) erase_flash
# 	@printf "$(GREEN)[SUCCESS]$(NC) Flash erased successfully\n"

# Monitor serial output
# monitor:
# 	@printf "$(BLUE)[INFO]$(NC) Starting serial monitor on port $(PORT)...\n"
# 	@cd $(MICROPYTHON_DIR)/ports/esp32 && \
# 		picocom $(PORT) -b 115200

# Clean build directories
clean:
	@printf "$(BLUE)[INFO]$(NC) Cleaning build directories...\n"
	@cd $(MICROPYTHON_DIR)/ports/esp32 && make clean
	@rm -rf $(BUILD_DIR)
	@printf "$(GREEN)[SUCCESS]$(NC) Build cleaned\n"

# Full clean (including mpy-cross)
# fullclean: clean
# 	@printf "$(BLUE)[INFO]$(NC) Full cleaning...\n"
# 	@cd $(MICROPYTHON_DIR) && make -C mpy-cross clean
# 	@printf "$(GREEN)[SUCCESS]$(NC) Full clean completed\n"

# Quick build and flash
# deploy: build flash
# 	@printf "$(GREEN)[SUCCESS]$(NC) Deploy completed!\n"

# Build and monitor
# build-monitor: build flash monitor
# Show build info
# info:
# 	@echo "Project root: $(PROJECT_ROOT)"
# 	@echo "MicroPython dir: $(MICROPYTHON_DIR)"
# 	@echo "Build dir: $(BUILD_DIR)"
# 	@echo "Target board: $(BOARD)"
# 	@echo "Board variant: $(VARIANT)"
# 	@echo "Serial port: $(PORT)"
# 	@echo "ESP-IDF path: $$IDF_PATH"
