#ifndef MINIOS_H
#define MINIOS_H

#include <stdint.h>

#define NULL ((void*)0)
#define TRUE 1
#define FALSE 0

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed char s8;
typedef signed short s16;
typedef signed int s32;

#define VIDEO_MEM 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

void printk(const char *fmt, ...);
void printf(const char *fmt, ...);

void outb(uint16_t port, uint8_t val);
uint8_t inb(uint16_t port);
void outw(uint16_t port, uint16_t val);
uint16_t inw(uint16_t port);

void enable_interrupts();
void disable_interrupts();
void halt();

void init_idt();
void init_pic();
void init_timer(uint32_t freq);

void init_kmalloc();
void *kmalloc(uint32_t size);
void kfree(void *ptr);

void init_fs();
int read_file(const char *filename, void *buf, uint32_t size);
int write_file(const char *filename, void *buf, uint32_t size);

void init_process();
void schedule();
void switch_to(u32 new_esp);

extern uint32_t jiffies;
extern uint32_t current_esp;
extern uint32_t next_process_esp;

typedef struct registers {
    uint32_t edi, esi, ebp, ebx, edx, ecx, eax;
    uint32_t eip, cs, eflags, esp, ss;
} registers_t;

typedef void (*interrupt_handler_t)(registers_t*);

void register_interrupt_handler(uint8_t n, interrupt_handler_t handler);

void keyboard_init();
void handle_keyboard(registers_t *regs);

void shell_init();
void shell_execute(const char *cmd);

#endif