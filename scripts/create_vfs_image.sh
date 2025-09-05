#!/bin/bash
# need to `pip install littlefs-python`

# littlefs-python create --block-size 4096 --fs-size 0xCF0000 ./vfs ./build/vfs.img
littlefs-python create --block-size 4096 --fs-size 0x7D000 ./vfs ./build/vfs.img
