# Root Make glue for MicroPython USER_C_MODULES
# This file is included by ports that use Make (e.g., unix)
# It delegates to the lvml module's micropython.mk

include $(USERMOD_DIR)/lvml/micropython.mk
