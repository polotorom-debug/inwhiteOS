#include "minios.h"

typedef struct {
    u16 limit_lo;
    u16 base_lo;
    u8 base_mid;
    u8 access;
    u8 granularity;
    u8 base_hi;
} __attribute__((packed)) gdt_entry_t;

typedef struct {
    u16 limit;
    u32 base;
} __attribute__((packed)) gdt_ptr_t;

typedef struct {
    u32 prev_tss;
    u32 esp0;
    u32 ss0;
    u32 esp1;
    u32 ss1;
    u32 esp2;
    u32 ss2;
    u32 cr3;
    u32 eip;
    u32 eflags;
    u32 eax;
    u32 ecx;
    u32 edx;
    u32 ebx;
    u32 esp;
    u32 ebp;
    u32 esi;
    u32 edi;
    u32 es;
    u32 cs;
    u32 ss;
    u32 ds;
    u32 fs;
    u32 gs;
    u32 ldt;
    u16 trap;
    u16 iomap_base;
} __attribute__((packed)) tss_entry_t;

static gdt_entry_t gdt[GDT_ENTRIES];
static gdt_ptr_t gdtp;
static tss_entry_t tss;

extern void gdt_flush(u32);
extern void tss_flush(void);

void gdt_set_entry(int num, u32 base, u32 limit, u8 access, u8 gran) {
    gdt[num].base_lo = (u16)(base & 0xFFFF);
    gdt[num].base_mid = (u8)((base >> 16) & 0xFF);
    gdt[num].base_hi = (u8)((base >> 24) & 0xFF);
    gdt[num].limit_lo = (u16)(limit & 0xFFFF);
    gdt[num].granularity = (u8)((limit >> 16) & 0x0F);
    gdt[num].granularity |= gran & 0xF0;
    gdt[num].access = access;
}

void init_gdt(void) {
    gdtp.limit = sizeof(gdt) - 1;
    gdtp.base = (u32)&gdt;

    gdt_set_entry(0, 0, 0, 0, 0);

    gdt_set_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    gdt_set_entry(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    gdt_set_entry(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);

    gdt_set_entry(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);

    tss.prev_tss = 0;
    tss.ss0 = KERNEL_DATA_SEG;
    tss.ss1 = 0;
    tss.ss2 = 0;
    tss.cr3 = 0;
    tss.eip = 0;
    tss.eflags = 0;
    tss.eax = 0;
    tss.ecx = 0;
    tss.edx = 0;
    tss.ebx = 0;
    tss.esp = 0;
    tss.ebp = 0;
    tss.esi = 0;
    tss.edi = 0;
    tss.es = 0;
    tss.cs = 0;
    tss.ss = 0;
    tss.ds = 0;
    tss.fs = 0;
    tss.gs = 0;
    tss.ldt = 0;
    tss.trap = 0;
    tss.iomap_base = sizeof(tss_entry_t);

    gdt_set_entry(5, (u32)&tss, sizeof(tss_entry_t) - 1, 0x89, 0x00);

    gdt_flush((u32)&gdtp);
    tss_flush();

    printk("GDT initialized (%d entries)\n", GDT_ENTRIES);
}

void set_kernel_stack(u32 stack) {
    tss.esp0 = stack;
}

void init_user_mode(void) {
    printk("User mode initialized (ring 3 ready)\n");
}

void switch_to_user_mode(u32 entry, u32 stack) {
    disable_interrupts();

    u32 user_cs = USER_CODE_SEG | 3;
    u32 user_ss = USER_DATA_SEG | 3;
    u32 user_ds = USER_DATA_SEG | 3;
    u32 user_es = USER_DATA_SEG | 3;
    u32 user_fs = USER_DATA_SEG | 3;
    u32 user_gs = USER_DATA_SEG | 3;

    tss.esp0 = stack;

    __asm__ volatile (
        "mov %0, %%eax\n"
        "push %%eax\n"
        "push %1\n"
        "pushf\n"
        "push %%eax\n"
        "push %%eax\n"
        "push %%eax\n"
        "mov %2, %%eax\n"
        "mov %%eax, %%ds\n"
        "mov %%eax, %%es\n"
        "mov %%eax, %%fs\n"
        "mov %%eax, %%gs\n"
        "iret\n"
        :
        : "r" (user_cs), "r" (entry), "r" (user_ds)
        : "eax", "memory"
    );
}
