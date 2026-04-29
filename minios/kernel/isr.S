.extern idt
.extern handlers
.equ KERNEL_DS, 0x10
.equ KERNEL_CS, 0x08

.macro isr num
.global isr\num
isr\num:
    push \num
    jmp isr_common_stub
.endm

.macro irq num
.global irq\num
irq\num:
    push \num
    jmp irq_common_stub
.endm

.global load_idt
load_idt:
    lidt [esp+2]
    ret

.global isr_common_stub
isr_common_stub:
    pushad
    
    mov ax, KERNEL_DS
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    push esp
    call generic_interrupt_handler
    add esp, 4
    
    popad
    iret

.global irq_common_stub
irq_common_stub:
    pushad
    
    mov ax, KERNEL_DS
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    push esp
    call generic_interrupt_handler
    add esp, 4
    
    popad
    iret

.global syscall_handler
syscall_handler:
    pushad
    
    mov ax, KERNEL_DS
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    push esp
    call handle_syscall
    add esp, 4
    
    popad
    iret

isr 0
isr 1
isr 2
isr 3
isr 4
isr 5
isr 6
isr 7
isr 8
isr 9
isr 10
isr 11
isr 12
isr 13
isr 14
isr 15
isr 16
isr 17
isr 18
isr 19
isr 20
isr 21
isr 22
isr 23
isr 24
isr 25
isr 26
isr 27
isr 28
isr 29
isr 30
isr 31

irq 0
irq 1
irq 2
irq 3
irq 4
irq 5
irq 6
irq 7
irq 8
irq 9
irq 10
irq 11
irq 12
irq 13
irq 14
irq 15

.global jiffies
jiffies:
    .long 0

.global current_esp
current_esp:
    .long 0

.global next_process_esp
next_process_esp:
    .long 0