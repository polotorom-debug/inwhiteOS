#include "minios.h"

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

static const char scancode_to_ascii[] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' '
};

static const char scancode_to_ascii_shifted[] = {
    0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0, ' '
};

static int shift_pressed = 0;
static char keyboard_buffer[256];
static int buffer_pos = 0;

extern void keyboard_read();

void handle_keyboard(registers_t *regs) {
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);
    
    if (scancode == 0x2A || scancode == 0x36) {
        shift_pressed = 1;
        return;
    }
    
    if (scancode == 0xAA || scancode == 0xB6) {
        shift_pressed = 0;
        return;
    }
    
    if (scancode & 0x80) {
        return;
    }
    
    if (scancode < sizeof(scancode_to_ascii)) {
        char c;
        if (shift_pressed) {
            c = scancode_to_ascii_shifted[scancode];
        } else {
            c = scancode_to_ascii[scancode];
        }
        
        if (c && buffer_pos < 255) {
            keyboard_buffer[buffer_pos++] = c;
        }
    }
}

void keyboard_init() {
    register_interrupt_handler(33, handle_keyboard);
    
    inb(KEYBOARD_DATA_PORT);
    inb(KEYBOARD_STATUS_PORT);
    
    printk("Keyboard initialized\n");
}

int keyboard_read_buffer(char *buf, int max_len) {
    int len = buffer_pos;
    if (len > max_len) len = max_len;
    
    for (int i = 0; i < len; i++) {
        buf[i] = keyboard_buffer[i];
    }
    
    buffer_pos = 0;
    
    return len;
}

int keyboard_available() {
    return buffer_pos > 0;
}