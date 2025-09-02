# user_main.py - Example customer application
# Upload this file to the root of your ESP32 filesystem

def main():
    """Customer's main application function"""
    print("Customer application starting...")
    
    # Import and use the lvml module
    import lvml
    lvml.hello()
    
    # Customer's code here
    print("Customer code running...")
    
    # LVGL is already initialized by main.py
    # You can now create UI elements, labels, buttons, etc.
    # Example:
    # import lvgl
    # label = lvgl.label_create(lvgl.screen_active())
    # lvgl.label_set_text(label, "Hello from customer code!")
    
    print("Customer application complete.")
