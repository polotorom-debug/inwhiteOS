.PHONY: all clean run

AS = nasm
CC = gcc
LD = ld

ASFLAGS = -f elf32
CFLAGS = -m32 -fno-pie -fno-stack-protector -nostdinc -nostdlib -fno-builtin -Wall -Werror
LDFLAGS = -m elf_i386 -T linker.ld --oformat elf32-i386

KERNEL_SRC = kernel/main.c kernel/printk.c kernel/interrupts.c kernel/timer.c kernel/process.c kernel/syscall.c
KERNEL_OBJ = $(KERNEL_SRC:.c=.o)

BOOT_SRC = boot/boot.S
BOOT_OBJ = boot/boot.o

DRIVER_SRC = drivers/keyboard.c
DRIVER_OBJ = $(DRIVER_SRC:.c=.o)

MM_SRC = mm/kmalloc.c
MM_OBJ = $(MM_SRC:.c=.o)

FS_SRC = fs/minix_fs.c
FS_OBJ = $(FS_SRC:.c=.o)

SHELL_SRC = shell/shell.c
SHELL_OBJ = $(SHELL_SRC:.c=.o)

ISR_OBJ = kernel/isr.o

ASM_HEADERS = -I include

all: boot/boot.bin kernel/kernel.bin floppy.img

boot/boot.bin: boot/boot.S
	$(AS) $(ASFLAGS) -o $@ $<

kernel/kernel.bin: $(KERNEL_OBJ) $(DRIVER_OBJ) $(MM_OBJ) $(FS_OBJ) $(SHELL_OBJ) $(ISR_OBJ) linker.ld
	$(LD) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $(ASM_HEADERS) -c -o $@ $<

%.o: %.S
	$(AS) $(ASFLAGS) -o $@ $<

floppy.img: boot/boot.bin kernel/kernel.bin
	dd if=/dev/zero of=$@ bs=512 count=2880 2>/dev/null
	dd if=boot/boot.bin of=$@ conv=notrunc 2>/dev/null
	$(AS) -f bin -o temp_flppy.img boot/boot.img.S 2>/dev/null || true
	./copy_image.sh 2>/dev/null || python3 -c "
import sys
with open('floppy.img', 'r+b') as f:
    with open('boot/boot.bin', 'rb') as b:
        boot = b.read()
        f.seek(0)
        f.write(boot)
        f.seek(512)
    with open('kernel/kernel.bin', 'rb') as k:
        kernel = k.read()
        f.seek(512*33)
        f.write(kernel)
"

clean:
	rm -f boot/*.o kernel/*.o drivers/*.o mm/*.o fs/*.o shell/*.o
	rm -f boot/boot.bin kernel/kernel.bin floppy.img
	rm -f temp_flppy.img 2>/dev/null

run: floppy.img
	qemu-system-i386 -fda floppy.img -m 16 -no-reboot

debug: floppy.img
	qemu-system-i386 -fda floppy.img -m 16 -s -S &

bochs: floppy.img
	bochs -f bochsrc.txt