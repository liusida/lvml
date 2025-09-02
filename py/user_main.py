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
    
    # Example: You could add LVGL initialization here
    # lvgl.init()
    # lvgl.create_ui()
    
    print("Customer application complete.")
