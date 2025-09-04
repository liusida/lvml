#!/usr/bin/env python3
"""
Test script to verify manual tick system works correctly.
This demonstrates that drawing works with manual tick() calls.
"""

import lvml
import time

def test_manual_tick():
    print("=== Testing Manual Tick System ===")
    
    # Initialize LVML
    print("1. Initializing LVML...")
    lvml.init()
    print("   ✓ LVML initialized")
    
    # Run debug function to show system status
    print("\n2. Running debug() to show system status...")
    lvml.debug()
    
    # Test drawing with manual tick calls
    print("\n3. Testing drawing with manual tick() calls...")
    
    # Draw first rectangle
    print("   Drawing red rectangle...")
    lvml.rect(50, 50, 100, 100, "#FF0000", "#000000", 0)
    lvml.tick()  # Process timers
    time.sleep(0.1)  # Wait a bit
    
    # Draw second rectangle
    print("   Drawing blue rectangle...")
    lvml.rect(200, 50, 100, 100, "#0000FF", "#000000", 0)
    lvml.tick()  # Process timers
    time.sleep(0.1)  # Wait a bit
    
    # Draw third rectangle
    print("   Drawing green rectangle...")
    lvml.rect(50, 200, 100, 100, "#00FF00", "#000000", 0)
    lvml.tick()  # Process timers
    time.sleep(0.1)  # Wait a bit
    
    print("   ✓ All rectangles drawn - they should appear after tick() calls!")
    
    # Run debug with display test
    print("\n4. Running debug(True) to test display...")
    lvml.debug(True)
    
    print("\n=== Test Complete ===")
    print("If you see colored rectangles appearing after tick() calls,")
    print("the manual tick system is working correctly!")
    print("Remember to call lvml.tick() periodically in your main loop.")

if __name__ == "__main__":
    test_manual_tick()
