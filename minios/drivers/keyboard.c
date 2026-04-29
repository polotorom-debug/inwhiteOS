#include "minios.h"

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define KBD_BUFFER_SIZE 256

static const char scancode_to_ascii[] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' '
};

static const char scancode_shifted[] = {
    0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0, ' '
};

typedef struct {
    char buffer[KBD_BUFFER_SIZE];
    int head;
    int tail;
    int count;
} kbd_circular_buffer_t;

static kbd_circular_buffer_t kbd_buf;
static int shift_pressed = 0;
static int caps_lock = 0;
static void (*key_callback)(char) = NULL;

static void kbd_buf_init(void) {
    kbd_buf.head = 0;
    kbd_buf.tail = 0;
    kbd_buf.count = 0;
    for (int i = 0; i < KBD_BUFFER_SIZE; i++) kbd_buf.buffer[i] = 0;
}

static void kbd_buf_push(char c) {
    if (kbd_buf.count >= KBD_BUFFER_SIZE) return;
    kbd_buf.buffer[kbd_buf.head] = c;
    kbd_buf.head = (kbd_buf.head + 1) % KBD_BUFFER_SIZE;
    kbd_buf.count++;
}

static char kbd_buf_pop(void) {
    if (kbd_buf.count == 0) return 0;
    char c = kbd_buf.buffer[kbd_buf.tail];
    kbd_buf.tail = (kbd_buf.tail + 1) % KBD_BUFFER_SIZE;
    kbd_buf.count--;
    return c;
}

void handle_keyboard(registers_t *regs) {
    (void)regs;
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);

    if (scancode == 0x2A || scancode == 0x36) {
        shift_pressed = 1;
        return;
    }
    if (scancode == 0xAA || scancode == 0xB6) {
        shift_pressed = 0;
        return;
    }
    if (scancode == 0x1D) {
        return;
    }
    if (scancode == 0x9D) {
        return;
    }

    if (scancode == 0x3A) {
        caps_lock = !caps_lock;
        return;
    }

    if (scancode & 0x80) return;

    if (scancode >= sizeof(scancode_to_ascii)) return;

    char c;
    int shifted = shift_pressed || (caps_lock && scancode_to_ascii[scancode] >= 'a' && scancode_to_ascii[scancode] <= 'z');

    if (shifted) {
        c = scancode_shifted[scancode];
    } else {
        c = scancode_to_ascii[scancode];
    }

    if (c && key_callback) {
        key_callback(c);
    }

    if (c) {
        kbd_buf_push(c);
    }
}

void keyboard_init(void) {
    kbd_buf_init();
    register_interrupt_handler(33, handle_keyboard);

    inb(KEYBOARD_DATA_PORT);
    inb(KEYBOARD_STATUS_PORT);

    printk("Keyboard initialized (circular buffer)\n");
}

int keyboard_available(void) {
    return kbd_buf.count > 0;
}

int keyboard_read_char(void) {
    if (kbd_buf.count == 0) return -1;
    return (int)kbd_buf_pop();
}

int keyboard_read_buffer(char *buf, int max_len) {
    int len = 0;
    while (len < max_len && kbd_buf.count > 0) {
        buf[len++] = kbd_buf_pop();
    }
    return len;
}

void keyboard_clear_buffer(void) {
    kbd_buf.head = 0;
    kbd_buf.tail = 0;
    kbd_buf.count = 0;
}

void keyboard_set_callback(void (*cb)(char)) {
    key_callback = cb;
}
