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

#define KERNEL_SPACE_START 0xC0000000
#define PAGE_SIZE 4096
#define PAGE_DIR_ENTRIES 1024
#define PAGE_TABLE_ENTRIES 1024

#define GDT_ENTRIES 6
#define KERNEL_CODE_SEG 0x08
#define KERNEL_DATA_SEG 0x10
#define USER_CODE_SEG 0x18
#define USER_DATA_SEG 0x20
#define TSS_SEG 0x28

#define MAX_SYSCALLS 16

void printk(const char *fmt, ...);
void printf(const char *fmt, ...);
void putchar(char c);
void clear_screen(void);
void set_cursor(int x, int y);
void update_cursor(void);
void scroll(void);
void kprint(const char *s);

void outb(uint16_t port, uint8_t val);
uint8_t inb(uint16_t port);
void outw(uint16_t port, uint16_t val);
uint16_t inw(uint16_t port);

void enable_interrupts(void);
void disable_interrupts(void);
void halt(void);

void init_gdt(void);
void init_idt(void);
void init_pic(void);
void init_timer(uint32_t freq);
void init_paging(void);
void enable_paging(void);

void init_kmalloc(void);
void *kmalloc(uint32_t size);
void *kmalloc_aligned(uint32_t size, uint32_t align);
void *kzalloc(uint32_t size);
void kfree(void *ptr);
void *krealloc(void *ptr, uint32_t new_size);
void kmem_stats(uint32_t *total, uint32_t *used, uint32_t *free);

void init_fs(void);
int read_file(const char *filename, void *buf, uint32_t size);
int write_file(const char *filename, void *buf, uint32_t size);
int delete_file(const char *filename);
int create_file(const char *filename);
void list_files(void);

void init_process(void);
int create_process(const char *name, void (*func)(void), int priority);
void schedule(void);
void switch_context(u32 *old_esp, u32 *new_esp);
int get_current_pid(void);
const char *get_current_process_name(void);
int kill_process(int pid);

void init_user_mode(void);
void switch_to_user_mode(u32 entry, u32 stack);

void keyboard_init(void);
void handle_keyboard(registers_t *regs);
int keyboard_available(void);
int keyboard_read_char(void);
int keyboard_read_buffer(char *buf, int max_len);
void keyboard_clear_buffer(void);
void keyboard_set_callback(void (*cb)(char));

void shell_init(void);
void shell_execute(const char *cmd);
void shell_mainloop(void);

void list_drivers(void);
int register_driver(const char *name, int (*init)(void), int (*read)(void *, int), int (*write)(const void *, int), int (*ioctl)(int, void *));

int handle_syscall(registers_t *regs);

int strlen_simple(const char *s);

u32 get_uptime_seconds(void);
u32 get_jiffies(void);
void sleep_ticks(u32 ticks);
void sleep_ms(u32 ms);

page_t *get_page(u32 address, int make, page_directory_t *dir);
void alloc_page(u32 address, page_directory_t *dir);
void free_page(u32 address, page_directory_t *dir);
page_directory_t *clone_page_directory(page_directory_t *src);

void init_interrupts(void);
void init_pic(void);

void set_kernel_stack(u32 stack);
void scheduler_toggle(int enable);

extern uint32_t jiffies;
extern uint32_t current_esp;
extern uint32_t next_process_esp;

typedef struct {
    uint32_t edi, esi, ebp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, esp, ss;
} registers_t;

typedef void (*interrupt_handler_t)(registers_t*);

void register_interrupt_handler(uint8_t n, interrupt_handler_t handler);

typedef struct process {
    int pid;
    char name[32];
    u32 esp;
    u32 ebp;
    u32 eip;
    u32 eflags;
    int state;
    int priority;
    int ticks;
    u32 kernel_stack[1024];
    u32 *page_dir;
} process_t;

#define PROC_FREE 0
#define PROC_RUNNING 1
#define PROC_READY 2
#define PROC_SLEEPING 3
#define PROC_ZOMBIE 4

#define PAGE_PRESENT 0x01
#define PAGE_WRITE 0x02
#define PAGE_USER 0x04

typedef struct page_directory {
    u32 *tables[PAGE_DIR_ENTRIES];
    u32 tablesPhysical[PAGE_DIR_ENTRIES];
    u32 physicalAddr;
} page_directory_t;

typedef struct page {
    u32 present:1;
    u32 rw:1;
    u32 user:1;
    u32 accessed:1;
    u32 dirty:1;
    u32 unused:7;
    u32 frame:20;
} __attribute__((packed)) page_t;

#endif
