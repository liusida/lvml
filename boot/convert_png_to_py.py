# convert_images.py - Run this on your computer
import os
import base64

def convert_png_to_py(png_path, py_path):
    """Convert PNG file to Python data file"""
    
    # Read PNG file
    with open(png_path, 'rb') as f:
        png_data = f.read()
    
    # Create Python file content
    py_content = f'''# {os.path.basename(py_path)} - {os.path.basename(png_path)} data
# Auto-generated from {os.path.basename(png_path)}

# PNG file data
PNG_DATA = {repr(png_data)}

# Image dimensions (you may need to adjust these)
WIDTH = 320
HEIGHT = 240

# File info
SIZE = {len(png_data)}
FORMAT = "PNG"
'''
    
    # Write Python file
    with open(py_path, 'w') as f:
        f.write(py_content)
    
    print(f"Converted {png_path} -> {py_path}")
    print(f"Data size: {len(png_data)} bytes")

# Convert your images
convert_png_to_py("images/lvml.png", "png_lvml.py")
convert_png_to_py("images/win98.png", "png_win98.py")
convert_png_to_py("images/dict.png", "png_dict.py")
convert_png_to_py("images/colorful_circle.png", "png_colorful_circle.py")
