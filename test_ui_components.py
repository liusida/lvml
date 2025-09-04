#!/usr/bin/env python3
"""
Test script for LVML UI components
Demonstrates buttons, text areas, and rectangles
"""

import lvml
import time

# Initialize LVML
lvml.init()

# Set background
lvml.set_bg("lightblue")

# Create some rectangles
lvml.rect(10, 10, 100, 50, "red", "black", 2)
lvml.rect(120, 10, 100, 50, "green", "black", 2)
lvml.rect(230, 10, 80, 50, "blue", "black", 2)

# Create buttons
lvml.button(10, 70, 100, 40, "Click Me!", "darkgreen", "white")
lvml.button(120, 70, 100, 40, "Submit", "darkblue", "white")
lvml.button(230, 70, 80, 40, "Cancel", "darkred", "white")

# Create text areas
lvml.textarea(10, 120, 200, 60, "Enter your name...", "white", "black")
lvml.textarea(10, 190, 200, 60, "Enter your message...", "white", "black")

# Create a larger text area
lvml.textarea(220, 120, 90, 130, "Notes...", "lightgray", "black")

# Manual tick loop
print("UI components created! Press Ctrl+C to exit.")
print("You should see rectangles, buttons, and text areas on the display.")
print("Call lvml.tick() periodically to update the display.")

try:
    while True:
        lvml.tick()
        time.sleep(0.01)  # 100ms tick rate
except KeyboardInterrupt:
    print("\nExiting...")
    lvml.deinit()
