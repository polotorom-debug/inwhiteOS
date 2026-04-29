#include "minios.h"

u16 *video_mem = (u16*)VIDEO_MEM;
int cursor_x = 0;
int cursor_y = 0;

void set_cursor(int x, int y) {
    cursor_x = x;
    cursor_y = y;
}

void update_cursor() {
    u16 pos = cursor_y * VGA_WIDTH + cursor_x;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (u8)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (u8)((pos >> 8) & 0xFF));
}

void scroll() {
    if (cursor_y >= VGA_HEIGHT) {
        for (int i = 0; i < (VGA_HEIGHT - 1) * VGA_WIDTH; i++) {
            video_mem[i] = video_mem[i + VGA_WIDTH];
        }
        for (int i = (VGA_HEIGHT - 1) * VGA_WIDTH; i < VGA_HEIGHT * VGA_WIDTH; i++) {
            video_mem[i] = 0x0F00;
        }
        cursor_y = VGA_HEIGHT - 1;
    }
}

void putchar(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\r') {
        cursor_x = 0;
    } else if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
            video_mem[cursor_y * VGA_WIDTH + cursor_x] = 0x0F00;
        }
    } else {
        video_mem[cursor_y * VGA_WIDTH + cursor_x] = (0x0F << 8) | c;
        cursor_x++;
    }
    
    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
    }
    
    scroll();
    update_cursor();
}

void printk(const char *fmt, ...) {
    char buffer[32];
    int i = 0;
    
    const char *p = fmt;
    __builtin_va_list args;
    __builtin_va_start(args, fmt);
    
    while (*p) {
        if (*p == '%') {
            p++;
            switch (*p) {
                case 'd': {
                    int val = __builtin_va_arg(args, int);
                    int j = 0;
                    if (val < 0) {
                        putchar('-');
                        val = -val;
                    }
                    if (val == 0) {
                        putchar('0');
                    } else {
                        while (val > 0) {
                            buffer[j++] = '0' + (val % 10);
                            val /= 10;
                        }
                        while (j > 0) putchar(buffer[--j]);
                    }
                    break;
                }
                case 'x': {
                    unsigned int val = __builtin_va_arg(args, unsigned int);
                    const char *hex = "0123456789abcdef";
                    char hexbuf[16];
                    int j = 0;
                    if (val == 0) {
                        putchar('0');
                    } else {
                        while (val > 0) {
                            hexbuf[j++] = hex[val & 0xF];
                            val >>= 4;
                        }
                        while (j > 0) putchar(hexbuf[--j]);
                    }
                    break;
                }
                case 's': {
                    char *s = __builtin_va_arg(args, char*);
                    while (*s) putchar(*s++);
                    break;
                }
                case 'c': {
                    char c = (char)__builtin_va_arg(args, int);
                    putchar(c);
                    break;
                }
                case '%':
                    putchar('%');
                    break;
                default:
                    break;
            }
        } else {
            putchar(*p);
        }
        p++;
    }
    
    __builtin_va_end(args);
}

void printf(const char *fmt, ...) {
    printk(fmt);
}

void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

uint8_t inb(uint16_t port) {
    uint8_t val;
    __asm__ volatile ("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

void outw(uint16_t port, uint16_t val) {
    __asm__ volatile ("outw %0, %1" : : "a"(val), "Nd"(port));
}

uint16_t inw(uint16_t port) {
    uint16_t val;
    __asm__ volatile ("inw %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

void enable_interrupts() {
    __asm__ volatile ("sti");
}

void disable_interrupts() {
    __asm__ volatile ("cli");
}

void halt() {
    while (1) {
        __asm__ volatile ("hlt");
    }
}

void kprint(const char *s) {
    while (*s) {
        putchar(*s++);
    }
}

void clear_screen() {
    for (int i = 0; i < VGA_HEIGHT * VGA_WIDTH; i++) {
        video_mem[i] = 0x0F00;
    }
    cursor_x = 0;
    cursor_y = 0;
    update_cursor();
}