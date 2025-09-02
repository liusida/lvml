# main.py - Simple initialization and user code execution
# Upload this file to the root of your ESP32 filesystem
import lvml
import gc
import sys

def initialize_system():
    """Run initialization before user code"""
    print("=== Initializing System ===")
    
    # Initialize LVGL
    if not lvml.is_initialized():
        lvml.init()
        lvml.set_dark_bg()  # Set dark background
    else:
        print("LVGL already initialized")
    
    # Test the module
    lvml.hello()
    
    # Add any other initialization:
    # - Set up hardware pins
    # - Configure display
    # - Initialize sensors
    # - Set up network
    # - etc.
    
    # Clean up memory
    gc.collect()
    print(f"Free memory: {gc.mem_free()} bytes")
    print("=== System Ready ===")

def run_user_code():
    """Execute customer's Python code"""
    try:
        # Try to run user_main.py first
        import user_main
        print("Running user_main.py...")
        user_main.main()
        
    except ImportError:
        try:
            # If no user_main.py, try user_code.py
            print("Running user_code.py...")
            with open('user_code.py', 'r') as f:
                exec(f.read())
                
        except OSError:
            # No user code found - just print message
            print("No user code found. System ready for manual operation.")
            print("Upload user_main.py or user_code.py to run your code.")
            
    except Exception as e:
        print(f"Error in user code: {e}")
        sys.print_exception(e)

# Main execution
print("ESP32 Starting...")
initialize_system()
run_user_code()
print("Boot sequence complete.")
