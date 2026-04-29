.global _start
.text

_start:
    jmp _start
    .fill 510 - (. - _start), 0
    .word 0xAA55

.global boot_sects
boot_sects:
    .incbin "boot/boot.bin"
    .fill (18 - 1) * 512 - (.), 0

.global kimage
kimage:
    .incbin "kernel/kernel.bin"