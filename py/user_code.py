# user_code.py - Alternative customer code (direct execution)
# Upload this file to the root of your ESP32 filesystem

print("Customer code starting...")

# Import and use the lvml module
import lvml
lvml.hello()

# Customer's code here
print("Customer code running...")

# Example: You could add LVGL initialization here
# lvgl.init()
# lvgl.create_ui()

print("Customer code complete.")
