#!/usr/bin/env python3
"""
Test script to verify color parsing works correctly for all formats.
"""

import lvml
import time

def test_color_parsing():
    print("=== Testing Color Parsing ===")
    
    # Initialize LVML
    lvml.init()
    print("✓ LVML initialized")
    
    # Test different color formats
    print("\n1. Testing CSS hex colors:")
    lvml.rect(10, 10, 50, 50, "#FF0000", "#000000", 0)    # Red
    lvml.tick()
    time.sleep(0.5)
    
    print("\n2. Testing C-style hex integers:")
    lvml.rect(70, 10, 50, 50, 0xFF0000, 0x000000, 0)     # Red (should work)
    lvml.tick()
    time.sleep(0.5)
    
    print("\n3. Testing decimal integers (should fail):")
    lvml.rect(130, 10, 50, 50, 16711680, 0, 0)           # 0xFF0000 as decimal
    lvml.tick()
    time.sleep(0.5)
    
    print("\n4. Testing named colors:")
    lvml.rect(10, 70, 50, 50, "red", "black", 0)         # Red
    lvml.tick()
    time.sleep(0.5)
    
    print("\n5. Testing plain hex strings:")
    lvml.rect(70, 70, 50, 50, "FF0000", "000000", 0)     # Red
    lvml.tick()
    time.sleep(0.5)
    
    print("\n=== Color Parsing Test Complete ===")
    print("If you see 5 red rectangles, all formats work correctly!")
    print("The 0xFF0000 format should work because the C code treats integers as hex values.")

if __name__ == "__main__":
    test_color_parsing()
