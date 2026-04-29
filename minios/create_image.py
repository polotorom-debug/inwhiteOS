#!/usr/bin/env python3
"""Create a bootable floppy image for MiniOS."""
import sys
import os

def create_image():
    boot_file = 'boot/boot.bin'
    kernel_file = 'kernel/kernel.bin'
    image_file = 'floppy.img'

    if not os.path.exists(boot_file):
        print("ERROR: boot/boot.bin not found!")
        sys.exit(1)

    if not os.path.exists(kernel_file):
        print("ERROR: kernel/kernel.bin not found!")
        sys.exit(1)

    with open(boot_file, 'rb') as f:
        boot = f.read()

    with open(kernel_file, 'rb') as f:
        kernel = f.read()

    image_size = 1440 * 1024
    boot_size = 512
    kernel_start = 512 * 33

    if len(boot) > boot_size:
        print(f"WARNING: Boot sector is {len(boot)} bytes, truncating to {boot_size}")
        boot = boot[:boot_size]

    if len(kernel) > image_size - kernel_start:
        print(f"ERROR: Kernel too large ({len(kernel)} bytes, max {image_size - kernel_start})")
        sys.exit(1)

    with open(image_file, 'wb') as f:
        f.write(b'\0' * image_size)
        f.seek(0)
        f.write(boot)
        f.seek(kernel_start)
        f.write(kernel)
        f.write(b'\0' * (image_size - kernel_start - len(kernel)))

    print("Floppy image created successfully!")
    print(f"  Boot sector: {len(boot)} bytes")
    print(f"  Kernel:      {len(kernel)} bytes")
    print(f"  Image size:  {image_size} bytes (1.44 MB)")

if __name__ == '__main__':
    create_image()
