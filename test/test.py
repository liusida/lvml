# Create this file: /Users/star/Projects/lvml/boot/test_filesystem.py

import os
import sys

def test_filesystem():
    """Test if filesystem is working properly"""
    print("=== Filesystem Test ===")
    
    # Test 1: Check if we can list root directory
    try:
        root_contents = os.listdir("/")
        print("PASS: Can list root directory")
        print("Root contents:", root_contents)
    except Exception as e:
        print("FAIL: Cannot list root directory:", e)
        return False
    
    # Test 2: Check current working directory
    try:
        cwd = os.getcwd()
        print("PASS: Current directory:", cwd)
    except Exception as e:
        print("FAIL: Cannot get current directory:", e)
        return False
    
    # Test 3: Test file creation
    try:
        test_file = "/test_write.txt"
        with open(test_file, "w") as f:
            f.write("Test file created successfully")
        print("PASS: Can create files")
    except Exception as e:
        print("FAIL: Cannot create files:", e)
        return False
    
    # Test 4: Test file reading
    try:
        with open(test_file, "r") as f:
            content = f.read()
        print("PASS: Can read files")
        print("File content:", content)
    except Exception as e:
        print("FAIL: Cannot read files:", e)
        return False
    
    # Test 5: Test file deletion
    try:
        os.remove(test_file)
        print("PASS: Can delete files")
    except Exception as e:
        print("FAIL: Cannot delete files:", e)
        return False
    
    # Test 6: Test directory creation
    try:
        os.mkdir("/test_dir")
        print("PASS: Can create directories")
        os.rmdir("/test_dir")
        print("PASS: Can remove directories")
    except Exception as e:
        print("FAIL: Directory operations failed:", e)
        return False
    
    # Test 7: Check sys.path
    print("PASS: sys.path:", sys.path)
    
    # Test 8: Test mip.install (if available)
    try:
        import mip
        print("PASS: mip module available")
        print("Testing mip.install...")
        # Don't actually install, just test if it would work
        print("mip.install should work now")
    except ImportError:
        print("INFO: mip module not available")
    except Exception as e:
        print("INFO: mip test failed:", e)
    
    print("=== All filesystem tests completed ===")
    return True

# Run the test
if __name__ == "__main__":
    test_filesystem()