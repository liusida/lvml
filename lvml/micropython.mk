LVML_MOD_DIR := $(USERMOD_DIR)

# Add main source files
SRC_USERMOD += $(LVML_MOD_DIR)/lvmlmodule.c
SRC_USERMOD += $(LVML_MOD_DIR)/lvml.c

# Add all C files from subdirectories using glob patterns
SRC_USERMOD += $(wildcard $(LVML_MOD_DIR)/core/*.c)
SRC_USERMOD += $(wildcard $(LVML_MOD_DIR)/xml/*.c)
SRC_USERMOD += $(wildcard $(LVML_MOD_DIR)/network/*.c)
SRC_USERMOD += $(wildcard $(LVML_MOD_DIR)/micropython/*.c)
SRC_USERMOD += $(wildcard $(LVML_MOD_DIR)/utils/*.c)
SRC_USERMOD += $(wildcard $(LVML_MOD_DIR)/driver/*.c)

# Add include directories
CFLAGS_USERMOD += -I$(LVML_MOD_DIR)
CFLAGS_USERMOD += -I$(LVML_MOD_DIR)/core
CFLAGS_USERMOD += -I$(LVML_MOD_DIR)/xml
CFLAGS_USERMOD += -I$(LVML_MOD_DIR)/network
CFLAGS_USERMOD += -I$(LVML_MOD_DIR)/micropython
CFLAGS_USERMOD += -I$(LVML_MOD_DIR)/utils
CFLAGS_USERMOD += -I$(LVML_MOD_DIR)/driver
