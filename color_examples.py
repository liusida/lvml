#!/usr/bin/env python3
"""
Example script showing all supported color formats in LVML.
"""

import lvml
import time

def color_examples():
    print("=== LVML Color Format Examples ===")
    
    # Initialize LVML
    lvml.init()
    print("✓ LVML initialized")
    
    # CSS-style hex colors (recommended)
    print("\n1. CSS-style hex colors (#RRGGBB):")
    lvml.rect(10, 10, 50, 50, "#FF0000", "#000000", 0)    # Red
    lvml.rect(70, 10, 50, 50, "#00FF00", "#000000", 0)    # Green
    lvml.rect(130, 10, 50, 50, "#0000FF", "#000000", 0)   # Blue
    lvml.tick()
    time.sleep(0.5)
    
    # Named colors
    print("\n2. Named colors:")
    lvml.rect(10, 70, 50, 50, "red", "black", 0)          # Red
    lvml.rect(70, 70, 50, 50, "green", "white", 2)        # Green with white border
    lvml.rect(130, 70, 50, 50, "blue", "black", 0)        # Blue
    lvml.tick()
    time.sleep(0.5)
    
    # C-style hex colors (0xRRGGBB)
    print("\n3. C-style hex colors (0xRRGGBB):")
    lvml.rect(10, 130, 50, 50, 0xFF0000, 0x000000, 0)     # Red
    lvml.rect(70, 130, 50, 50, 0x00FF00, 0x000000, 0)     # Green
    lvml.rect(130, 130, 50, 50, 0x0000FF, 0x000000, 0)    # Blue
    lvml.tick()
    time.sleep(0.5)
    
    # Plain hex strings (RRGGBB)
    print("\n4. Plain hex strings (RRGGBB):")
    lvml.rect(10, 190, 50, 50, "FF0000", "000000", 0)     # Red
    lvml.rect(70, 190, 50, 50, "00FF00", "000000", 0)     # Green
    lvml.rect(130, 190, 50, 50, "0000FF", "000000", 0)   # Blue
    lvml.tick()
    time.sleep(0.5)
    
    # Mixed formats
    print("\n5. Mixed color formats:")
    lvml.rect(10, 250, 50, 50, "#FF0000", "black", 2)     # CSS hex + named
    lvml.rect(70, 250, 50, 50, "green", 0x000000, 0)      # Named + C hex
    lvml.rect(130, 250, 50, 50, 0x0000FF, "#FFFFFF", 1)   # C hex + CSS hex
    lvml.tick()
    
    print("\n=== All color formats work! ===")
    print("✓ CSS hex: #FF0000")
    print("✓ Named: red, green, blue, white, black")
    print("✓ C hex: 0xFF0000")
    print("✓ Plain hex: FF0000")

if __name__ == "__main__":
    color_examples()
