.PHONY: all clean run debug bochs

AS = nasm
CC = gcc
LD = ld

ASFLAGS = -f elf32
CFLAGS = -m32 -fno-pie -fno-stack-protector -nostdinc -nostdlib -fno-builtin -Wall -Wextra -O2
LDFLAGS = -m elf_i386 -T linker.ld --oformat elf32-i386

INCLUDES = -I include

KERNEL_SRCS = kernel/main.c kernel/printk.c kernel/interrupts.c kernel/timer.c \
              kernel/process.c kernel/syscall.c kernel/gdt.c
KERNEL_OBJS = $(KERNEL_SRCS:.c=.o)

BOOT_SRCS = boot/boot.S
BOOT_OBJS = $(BOOT_SRCS:.S=.o)

DRIVER_SRCS = drivers/keyboard.c drivers/driver_framework.c
DRIVER_OBJS = $(DRIVER_SRCS:.c=.o)

MM_SRCS = mm/kmalloc.c mm/paging.c
MM_OBJS = $(MM_SRCS:.c=.o)

FS_SRCS = fs/minix_fs.c
FS_OBJS = $(FS_SRCS:.c=.o)

SHELL_SRCS = shell/shell.c
SHELL_OBJS = $(SHELL_SRCS:.c=.o)

ISR_OBJS = kernel/isr.o mm/paging.o kernel/gdt.o

ALL_OBJS = $(KERNEL_OBJS) $(DRIVER_OBJS) $(MM_OBJS) $(FS_OBJS) $(SHELL_OBJS) $(ISR_OBJS)

QEMU = qemu-system-i386
QEMU_FLAGS = -fda floppy.img -m 32 -no-reboot -serial stdio

all: boot/boot.bin kernel/kernel.bin floppy.img
	@echo "Build complete!"
	@echo "  Boot sector: boot/boot.bin"
	@echo "  Kernel:      kernel/kernel.bin"
	@echo "  Image:       floppy.img"

boot/boot.bin: boot/boot.S
	$(AS) $(ASFLAGS) -o $@ $<

kernel/kernel.bin: $(ALL_OBJS) linker.ld
	$(LD) $(LDFLAGS) -o $@ $(ALL_OBJS)
	@echo "Kernel linked at 0x1000"

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

%.o: %.S
	$(AS) $(ASFLAGS) -o $@ $<

floppy.img: boot/boot.bin kernel/kernel.bin
	@echo "Creating floppy image..."
	dd if=/dev/zero of=floppy.img bs=512 count=2880 2>/dev/null
	dd if=boot/boot.bin of=floppy.img conv=notrunc 2>/dev/null
	python3 create_image.py
	@echo "Floppy image created successfully!"

run: floppy.img
	$(QEMU) $(QEMU_FLAGS)

debug: floppy.img
	$(QEMU) $(QEMU_FLAGS) -s -S &
	@echo "QEMU started in debug mode. Connect with: gdb -ex 'target remote :1234'"

bochs: floppy.img
	bochs -f bochsrc.txt -q

clean:
	rm -f $(ALL_OBJS)
	rm -f boot/boot.bin kernel/kernel.bin floppy.img
	rm -f bochs.log temp_flppy.img
	@echo "Cleaned."
