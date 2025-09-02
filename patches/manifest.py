# Manifest file for lvml project
# This includes Python files as frozen modules in the firmware

include("$(MPY_DIR)/extmod/uasyncio/manifest.py")
include("$(MPY_DIR)/extmod/webrepl/manifest.py")
include("$(MPY_DIR)/extmod/urequests/manifest.py")
include("$(MPY_DIR)/extmod/umqtt/manifest.py")

# Include our custom Python modules
freeze("$(PROJECT_ROOT)/py", "main.py")
freeze("$(PROJECT_ROOT)/py", "user_code.py") 
freeze("$(PROJECT_ROOT)/py", "user_main.py")
