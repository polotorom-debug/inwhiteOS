#include "minios.h"

#define PIT_COMMAND 0x43
#define PIT_CHANNEL0 0x40
#define PIT_FREQ 1193180

uint32_t jiffies = 0;

void timer_callback(registers_t *regs) {
    jiffies++;
    
    if (jiffies % 100 == 0) {
        schedule();
    }
}

void init_timer(uint32_t freq) {
    uint32_t divisor = PIT_FREQ / freq;
    
    outb(PIT_COMMAND, 0x36);
    
    outb(PIT_CHANNEL0, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0, (uint8_t)((divisor >> 8) & 0xFF));
    
    register_interrupt_handler(32, timer_callback);
    
    printk("Timer initialized at %d Hz (divisor: %d)\n", freq, divisor);
}