# main.py - Simple initialization and user code execution
# Upload this file to the root of your ESP32 filesystem
import lvml
import png_lvml
lvml.init()
lvml.show_image(png_lvml.PNG_DATA)
lvml.tick()
