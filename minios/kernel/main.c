#include "minios.h"

void main();

void _start() {
    __asm__ volatile (
        "mov $0x10, %ax\n\t"
        "mov %ax, %ds\n\t"
        "mov %ax, %es\n\t"
        "mov %ax, %fs\n\t"
        "mov %ax, %gs\n\t"
        "mov %ax, %ss\n\t"
    );
    
    __asm__ volatile (
        "jmp %0"
        :
        : "r" (main)
    );
}

void clear_screen();

void shell_mainloop();

void main() {
    clear_screen();
    
    printk("MiniOS v0.01 - Starting...\n");
    printk("=======================\n\n");
    
    __asm__ volatile ("mov $0x10, %ax; mov %ax, %ds; mov %ax, %es");
    printk("GDT: OK\n");
    
    init_idt();
    printk("IDT: OK\n");
    
    init_pic();
    printk("PIC: OK\n");
    
    init_timer(100);
    printk("Timer: OK\n");
    
    init_kmalloc();
    printk("Memory: OK\n");
    
    keyboard_init();
    printk("Keyboard: OK\n");
    
    init_fs();
    printk("FS: OK\n");
    
    init_process();
    printk("Process: OK\n");
    
    enable_interrupts();
    
    printk("\n=======================\n");
    printk(" MiniOS Ready!\n");
    printk("=======================\n\n");
    
    clear_screen();
    printk("\n");
    printk("  ___  ___  ___  ___  ___  ___\n");
    printk(" | _ \\/ __|/ _ \\| __||_ _|| _ \\\n");
    printk(" |  _/\\__ \\ (_) | _|  | | |   /\n");
    printk(" |_|  |___/\\___/|_|  |___||_|_\\\n");
    printk("\n");
    printk("MiniOS v0.01 - Type 'help' for commands\n");
    printk("\n");
    
    shell_mainloop();
    
    while (1) {
        __asm__ volatile ("hlt");
    }
}