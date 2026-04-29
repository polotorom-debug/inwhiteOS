#include "minios.h"

void main();

void _start(void) {
    __asm__ volatile (
        "mov $0x10, %%ax\n\t"
        "mov %%ax, %%ds\n\t"
        "mov %%ax, %%es\n\t"
        "mov %%ax, %%fs\n\t"
        "mov %%ax, %%gs\n\t"
        "mov %%ax, %%ss\n\t"
        "mov $0x7C00, %%esp\n\t"
        :
        :
        : "ax"
    );

    main();
}

void shell_mainloop(void);

void main(void) {
    clear_screen();

    printk("MiniOS v0.02 - Starting...\n");
    printk("==========================\n\n");

    init_gdt();
    printk("[OK] GDT\n");

    init_interrupts();
    printk("[OK] IDT\n");

    init_pic();
    printk("[OK] PIC\n");

    init_kmalloc();
    printk("[OK] Memory Manager\n");

    init_paging();
    printk("[OK] Paging\n");

    init_timer(100);
    printk("[OK] Timer (100Hz)\n");

    keyboard_init();
    printk("[OK] Keyboard\n");

    init_fs();
    printk("[OK] File System\n");

    init_process();
    printk("[OK] Process Manager\n");

    init_user_mode();
    printk("[OK] User Mode\n");

    enable_interrupts();

    printk("\n==========================\n");
    printk(" MiniOS v0.02 Ready!\n");
    printk("==========================\n\n");

    clear_screen();
    printk("\n");
    printk("  ___  ___  ___  ___  ___  ___\n");
    printk(" | _ \\/ __|/ _ \\| __||_ _|| _ \\\n");
    printk(" |  _/\\__ \\ (_) | _|  | | |   /\n");
    printk(" |_|  |___/\\___/|_|  |___||_|_\\\n");
    printk("\n");
    printk("MiniOS v0.02 - Type 'help' for commands\n");
    printk("\n");

    shell_mainloop();

    while (1) {
        __asm__ volatile ("hlt");
    }
}
