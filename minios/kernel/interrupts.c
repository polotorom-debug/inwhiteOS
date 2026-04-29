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
    u16 limit;
    u32 base;
} __attribute__((packed)) descriptor_t;

static const char *exception_messages[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved",
    "x87 Floating-Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Control Protection Exception",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Hypervisor Injection Exception",
    "VMM Communication Exception",
    "Security Exception",
    "Reserved"
};

extern void load_idt(descriptor_t *desc);
extern void isr0();  extern void isr1();  extern void isr2();  extern void isr3();
extern void isr4();  extern void isr5();  extern void isr6();  extern void isr7();
extern void isr8();  extern void isr9();  extern void isr10(); extern void isr11();
extern void isr12(); extern void isr13(); extern void isr14(); extern void isr15();
extern void isr16(); extern void isr17(); extern void isr18(); extern void isr19();
extern void isr20(); extern void isr21(); extern void isr22(); extern void isr23();
extern void isr24(); extern void isr25(); extern void isr26(); extern void isr27();
extern void isr28(); extern void isr29(); extern void isr30(); extern void isr31();
extern void irq0();  extern void irq1();  extern void irq2();  extern void irq3();
extern void irq4();  extern void irq5();  extern void irq6();  extern void irq7();
extern void irq8();  extern void irq9();  extern void irq10(); extern void irq11();
extern void irq12(); extern void irq13(); extern void irq14(); extern void irq15();
extern void syscall_handler();

static idt_entry_t idt[IDT_SIZE];
static interrupt_handler_t interrupt_handlers[IDT_SIZE];
static int handlers_registered[IDT_SIZE];

static void *isr_functions[] = {
    isr0, isr1, isr2, isr3, isr4, isr5, isr6, isr7,
    isr8, isr9, isr10, isr11, isr12, isr13, isr14, isr15,
    isr16, isr17, isr18, isr19, isr20, isr21, isr22, isr23,
    isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31
};

static void *irq_functions[] = {
    irq0, irq1, irq2, irq3, irq4, irq5, irq6, irq7,
    irq8, irq9, irq10, irq11, irq12, irq13, irq14, irq15,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    syscall_handler
};

static void set_idt_entry(int n, void *handler, u16 selector, u8 type) {
    u32 addr = (u32)handler;
    idt[n].offset_lo = (u16)(addr & 0xFFFF);
    idt[n].selector = selector;
    idt[n].zero = 0;
    idt[n].type_attr = type;
    idt[n].offset_hi = (u16)((addr >> 16) & 0xFFFF);
}

void page_fault_handler(registers_t *regs) {
    u32 faulting_addr;
    __asm__ volatile ("mov %%cr2, %0" : "=r" (faulting_addr));
    printk("\n*** PAGE FAULT ***\n");
    printk("  Address: 0x%x\n", faulting_addr);
    printk("  Error:   0x%x\n", regs->err_code);
    if (regs->err_code & 0x1) printk("  Cause: Protection violation\n");
    else printk("  Cause: Page not present\n");
    if (regs->err_code & 0x2) printk("  Access: Write\n");
    else printk("  Access: Read\n");
    if (regs->err_code & 0x4) printk("  Mode: User\n");
    else printk("  Mode: Kernel\n");
    printk("Halting system.\n");
    while (1) __asm__ volatile ("hlt");
}

void generic_fault_handler(registers_t *regs, int num) {
    printk("\n*** FATAL EXCEPTION: %s (IRQ %d) ***\n",
           num < 32 ? exception_messages[num] : "Unknown", num);
    printk("  EIP: 0x%x  CS: 0x%x\n", regs->eip, regs->cs);
    printk("  EFLAGS: 0x%x  ESP: 0x%x\n", regs->eflags, regs->esp);
    printk("  EAX: 0x%x  EBX: 0x%x  ECX: 0x%x  EDX: 0x%x\n",
           regs->eax, regs->ebx, regs->ecx, regs->edx);
    printk("Halting system.\n");
    while (1) __asm__ volatile ("hlt");
}

void register_interrupt_handler(uint8_t n, interrupt_handler_t handler) {
    if (n < IDT_SIZE) {
        interrupt_handlers[n] = handler;
        handlers_registered[n] = 1;
    }
}

static void irq_handler_wrapper(registers_t *regs) {
    int irq_num = regs->int_no;

    if (interrupt_handlers[irq_num]) {
        interrupt_handlers[irq_num](regs);
    }

    if (irq_num >= 40) {
        outb(0xA0, 0x20);
    }
    outb(0x20, 0x20);
}

void init_interrupts(void) {
    for (int i = 0; i < IDT_SIZE; i++) {
        interrupt_handlers[i] = NULL;
        handlers_registered[i] = 0;
    }

    register_interrupt_handler(14, page_fault_handler);

    for (int i = 0; i < 32; i++) {
        set_idt_entry(i, isr_functions[i], KERNEL_CODE_SEG, 0x8E);
    }

    for (int i = 0; i < 16; i++) {
        if (irq_functions[i]) {
            set_idt_entry(i + 32, irq_functions[i], KERNEL_CODE_SEG, 0x8E);
        }
    }

    set_idt_entry(128, syscall_handler, KERNEL_CODE_SEG, 0xEE);

    descriptor_t idt_desc;
    idt_desc.limit = sizeof(idt) - 1;
    idt_desc.base = (u32)idt;
    load_idt(&idt_desc);

    printk("IDT initialized (%d entries)\n", IDT_SIZE);
}

void init_pic(void) {
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

    printk("PIC initialized (master: 0x20, slave: 0x28)\n");
}
