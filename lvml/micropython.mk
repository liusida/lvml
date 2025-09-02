LVML_MOD_DIR := $(USERMOD_DIR)

# Add all C files to SRC_USERMOD.
SRC_USERMOD += $(LVML_MOD_DIR)/lvmlmodule.c

# We can add our module folder to include paths if needed
CFLAGS_USERMOD += -I$(LVML_MOD_DIR)
