#!/usr/bin/env python3
"""
Test script to verify the manual tick fix works correctly.
This should demonstrate that the original bug is fixed.
"""

import lvml
import time

def test_manual_tick_fix():
    print("=== Testing Manual Tick Fix ===")
    
    # Initialize LVML
    print("1. Initializing LVML...")
    lvml.init()
    print("   ✓ LVML initialized")
    
    # Test the original bug scenario
    print("\n2. Testing original bug scenario...")
    
    # Draw first rectangle
    print("   Drawing red rectangle...")
    lvml.rect(50, 50, 100, 100, "#FF0000", "#000000", 0)
    lvml.tick()  # First tick - should show rectangle
    time.sleep(0.1)
    
    # Draw second rectangle  
    print("   Drawing blue rectangle...")
    lvml.rect(200, 50, 100, 100, "#0000FF", "#000000", 0)
    lvml.tick()  # Second tick - should show both rectangles
    time.sleep(0.1)
    
    # Draw third rectangle
    print("   Drawing green rectangle...")
    lvml.rect(50, 200, 100, 100, "#00FF00", "#000000", 0)
    lvml.tick()  # Third tick - should show all three rectangles
    time.sleep(0.1)
    
    print("   ✓ All rectangles drawn and should be visible!")
    
    # Test continuous drawing
    print("\n3. Testing continuous drawing...")
    for i in range(5):
        x = 50 + (i * 30)
        y = 300 + (i * 20)
        # Create a gradient from red to green using CSS hex
        red = 255 - (i * 50)
        green = i * 50
        color = f"#{red:02X}{green:02X}00"  # Format as CSS hex
        lvml.rect(x, y, 20, 20, color, "#000000", 0)
        lvml.tick()
        time.sleep(0.05)
    
    print("   ✓ Continuous drawing test complete!")
    
    print("\n=== Test Complete ===")
    print("If you see all rectangles appearing correctly,")
    print("the manual tick fix is working!")
    print("The original bug should be resolved.")

if __name__ == "__main__":
    test_manual_tick_fix()
