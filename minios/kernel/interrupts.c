#include "minios.h"

#define IDT_SIZE 256

typedef struct {
    u16 offset_lo;
    u16 selector;
    u8 zero;
    u8 type_attr;
    u16 offset_hi;
} __attribute__((packed)) idt_entry_t;

typedef struct {
    u32 ptr[2];
    u16 limit;
    u32 base;
} __attribute__((packed)) idt_ptr_t;

typedef struct {
    u16 limit;
    u32 base;
} __attribute__((packed)) descriptor_t;

extern void load_idt(descriptor_t *desc);
extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();
extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();
extern void syscall_handler();

static idt_entry_t idt[IDT_SIZE];
static interrupt_handler_t handlers[IDT_SIZE];

void *isr_functions[] = {
    isr0, isr1, isr2, isr3, isr4, isr5, isr6, isr7,
    isr8, isr9, isr10, isr11, isr12, isr13, isr14, isr15,
    isr16, isr17, isr18, isr19, isr20, isr21, isr22, isr23,
    isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31
};

void *irq_functions[] = {
    irq0, irq1, irq2, irq3, irq4, irq5, irq6, irq7,
    irq8, irq9, irq10, irq11, irq12, irq13, irq14, irq15,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    syscall_handler
};

void set_idt_entry(int n, void *handler, u16 selector, u8 type) {
    u32 addr = (u32)handler;
    idt[n].offset_lo = (u16)(addr & 0xFFFF);
    idt[n].selector = selector;
    idt[n].zero = 0;
    idt[n].type_attr = type;
    idt[n].offset_hi = (u16)((addr >> 16) & 0xFFFF);
}

void init_idt() {
    for (int i = 0; i < IDT_SIZE; i++) {
        handlers[i] = 0;
    }
    
    descriptor_t idt_desc;
    idt_desc.limit = sizeof(idt) - 1;
    idt_desc.base = (u32)idt;
    
    for (int i = 0; i < 32; i++) {
        set_idt_entry(i, isr_functions[i], 0x08, 0x8E);
    }
    
    for (int i = 0; i < 16; i++) {
        if (irq_functions[i]) {
            set_idt_entry(i + 32, irq_functions[i], 0x08, 0x8E);
        }
    }
    
    set_idt_entry(128, syscall_handler, 0x08, 0x8E);
    
    load_idt(&idt_desc);
    
    printk("IDT initialized with %d entries\n", IDT_SIZE);
}

void init_pic() {
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    
    outb(0x21, 0x00);
    outb(0xA1, 0x00);
    
    outb(0x21, 0xFC);
    outb(0xA1, 0xFF);
    
    printk("PIC initialized\n");
}

void register_interrupt_handler(uint8_t n, interrupt_handler_t handler) {
    if (n < IDT_SIZE) {
        handlers[n] = handler;
    }
}

void generic_interrupt_handler(registers_t *regs) {
    if (handlers[regs->eip & 0xFF]) {
        handlers[regs->eip & 0xFF](regs);
    }
    
    if (regs->eip >= 32 && regs->eip < 48) {
        int irq = regs->eip - 32;
        if (irq >= 8) {
            outb(0xA0, 0x20);
        }
        outb(0x20, 0x20);
    }
}