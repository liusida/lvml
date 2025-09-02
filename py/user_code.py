# user_code.py - Alternative customer code (direct execution)
# Upload this file to the root of your ESP32 filesystem

print("Customer code starting...")

# Import and use the lvml module
import lvml
lvml.hello()

# Customer's code here
print("Customer code running...")

# LVGL is already initialized by main.py with dark background
# You can now create UI elements, labels, buttons, etc.
# Example:
# import lvgl
# label = lvgl.label_create(lvgl.screen_active())
# lvgl.label_set_text(label, "Hello from customer code!")

print("Customer code complete.")
