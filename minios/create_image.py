#!/usr/bin/env python3
import sys

with open('floppy.img', 'r+b') as f:
    with open('boot/boot.bin', 'rb') as b:
        boot = b.read()
        f.seek(0)
        f.write(boot)
    
    with open('kernel/kernel.bin', 'rb') as k:
        kernel = k.read()
        f.seek(512 * 33)
        f.write(kernel)

print("Floppy image created successfully!")
print(f"Boot sector: 512 bytes")
print(f"Kernel: {len(kernel)} bytes")