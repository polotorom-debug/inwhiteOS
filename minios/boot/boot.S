.code16

.set BOOTSEG, 0x07C0
.set KERNEL_LOAD_SEG, 0x1000
.set KERNEL_LOAD_OFFSET, 0x0000
.set SECTORS_TO_LOAD, 32
.set ROOT_DIR_SECTOR, 19
.set FAT_SECTOR, 1

.global _start
.text

_start:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x0400
    
    mov si, boot_message
    call puts
    
load_root_dir:
    mov ax, WORD [boot_drive]
    mov cx, 1
    mov dx, ROOT_DIR_SECTOR
    mov bx, buffer
    call readsector
    jc load_error
    
    mov si, buffer
    mov di, kern_filename
    mov cx, 11
    rep cmpsb
    je found_kernel
    
    add bx, 32
    cmp WORD [buffer + 0x200 + 11], 0
    jne load_root_dir
    
    jmp kernel_not_found

found_kernel:
    mov ax, [buffer + 0x1A]
    mov [fat_start], ax
    
load_fat:
    mov ax, WORD [boot_drive]
    mov cx, 1
    mov dx, FAT_SECTOR
    mov bx, fat_buffer
    call readsector
    
    mov ax, [fat_start]
    mov bx, 2
    mul bx
    mov si, ax
    add si, fat_buffer
    mov ax, [si]
    mov [next_cluster], ax
    
load_kernel_sectors:
    mov ax, [fat_start]
    add ax, 31
    mov dx, ax
    mov cx, 1
    mov ax, WORD [boot_drive]
    mov bx, KERNEL_LOAD_OFFSET
    push es
    mov es, KERNEL_LOAD_SEG
    xor bx, bx
    call readsector
    pop es
    jc load_error
    
    add bx, 512
    cmp bx, 0
    je next_seg
    
    jmp load_kernel
    
next_seg:
    mov ax, es
    add ax, 0x1000
    mov es, ax
    xor bx, bx
    
load_kernel:
    mov ax, [next_cluster]
    cmp ax, 0xFFF
    je jump_to_kernel
    
    sub ax, 2
    add ax, 33
    mov dx, ax
    mov cx, 1
    mov ax, WORD [boot_drive]
    call readsector
    jc load_error
    
    add bx, 512
    cmp bx, 0x200
    je seg_full
    
    jmp load_kernel
    
seg_full:
    mov ax, es
    add ax, 0x1000
    mov es, ax
    xor bx, bx
    jmp load_kernel
    
jump_to_kernel:
    cli
    mov ax, 0x2400
    int 0x11
    
    mov ax, 0x0003
    int 0x10
    
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    mov ss, ax
    mov sp, 0x7C00
    
    push word KERNEL_LOAD_SEG
    push word 0x0000
    retf
    
load_error:
    mov si, error_msg
    call puts
    jmp halt

kernel_not_found:
    mov si, not_found_msg
    call puts
    
halt:
    jmp halt

puts:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    int 0x10
    jmp puts
.done:
    ret

readsector:
    pusha
    push es
    
    mov di, 5
.retry:
    mov ah, 0x02
    mov al, 1
    mov ch, 0
    mov cl, dl
    mov dh, 0
    mov dl, al
    int 0x13
    
    jnc .done
    xor ah, ah
    int 0x13
    dec di
    jnz .retry
    
    pop es
    stc
    jmp .exit
    
.done:
    pop es
    clc
.exit:
    popa
    ret

boot_message:
    .asciz "MiniOS Boot Loader...\r\n"

error_msg:
    .asciz "\r\nError loading kernel!\r\n"

not_found_msg:
    .asciz "Kernel not found!\r\n"

kern_filename:
    .ascii "KERNEL    BIN"

boot_drive:
    .word 0

fat_start:
    .word 0

next_cluster:
    .word 0

buffer:
    .skip 512

fat_buffer:
    .skip 1024

.fill 510 - (. - _start), 0
.word 0xAA55