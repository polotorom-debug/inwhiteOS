#include "minios.h"

#define PIT_COMMAND 0x43
#define PIT_CHANNEL0 0x40
#define PIT_FREQ 1193180

uint32_t jiffies = 0;
static int timer_handler_registered = 0;

static void timer_callback(registers_t *regs) {
    (void)regs;
    jiffies++;

    if (jiffies % 100 == 0) {
        schedule();
    }
}

void init_timer(uint32_t freq) {
    if (freq == 0) freq = 100;

    u32 divisor = PIT_FREQ / freq;

    outb(PIT_COMMAND, 0x36);
    u8 l = (u8)(divisor & 0xFF);
    u8 h = (u8)((divisor >> 8) & 0xFF);
    outb(PIT_CHANNEL0, l);
    outb(PIT_CHANNEL0, h);

    register_interrupt_handler(32, timer_callback);
    timer_handler_registered = 1;

    printk("Timer initialized at %d Hz (divisor: %d)\n", freq, divisor);
}

u32 get_uptime_seconds(void) {
    return jiffies / 100;
}

u32 get_jiffies(void) {
    return jiffies;
}

void sleep_ticks(u32 ticks) {
    u32 end = jiffies + ticks;
    while (jiffies < end) {
        __asm__ volatile ("hlt");
    }
}

void sleep_ms(u32 ms) {
    u32 ticks = (ms * 100) / 1000;
    if (ticks == 0) ticks = 1;
    sleep_ticks(ticks);
}
