import vfs
from esp32 import Partition
import os

def ensure_filesystem():
    """Ensure filesystem is mounted, initialize if needed"""
    try:
        # Quick test - if this works, filesystem is ready
        os.mkdir("/tmp")
        return True
    except:
        pass
    
    print("Setting up filesystem...")
    
    # Find vfs partition
    partitions = Partition.find(Partition.TYPE_DATA)
    vfs_partition = None
    for p in partitions:
        if p.info()[4] == "vfs":
            vfs_partition = p
            break
    
    if not vfs_partition:
        print("No vfs partition found")
        return False
    
    try:
        # Try to mount existing filesystem first
        fs = vfs.VfsFat(vfs_partition)
        vfs.mount(fs, "/")
        print("Filesystem mounted")
        return True
    except:
        # Format and mount
        try:
            vfs.VfsFat.mkfs(vfs_partition)
            fs = vfs.VfsFat(vfs_partition)
            vfs.mount(fs, "/")
            print("Filesystem formatted and mounted")
            return True
        except Exception as e:
            print("Filesystem setup failed:", e)
            return False

# Initialize filesystem at startup
ensure_filesystem()
