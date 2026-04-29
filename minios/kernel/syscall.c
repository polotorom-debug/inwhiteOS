#include "minios.h"

extern void *video_mem;

int handle_syscall(registers_t *regs) {
    int sysno = regs->eax;
    int arg1 = regs->ebx;
    int arg2 = regs->ecx;
    int arg3 = regs->edx;
    
    switch (sysno) {
        case 1: {
            const char *s = (const char*)arg1;
            while (*s) putchar(*s++);
            break;
        }
        case 2: {
            return get_current_pid();
        }
        case 3: {
            extern int keyboard_available();
            return keyboard_available();
        }
        case 4: {
            extern int keyboard_read_buffer(char*, int);
            return keyboard_read_buffer((char*)arg1, arg2);
        }
        case 5: {
            clear_screen();
            break;
        }
        case 6: {
            extern void list_files();
            list_files(); // arg1 es la dirección de la cadena
            printk("Syscall test: argument = %d\n", arg1);
            break;
        }
        default:
            return -1;
    }
    
    return 0;
}

int syscall(int num, int arg1, int arg2, int arg3) {
    int ret;
    __asm__ volatile (
        "mov %1, %%eax\n\t"
        "mov %2, %%ebx\n\t"
        "mov %3, %%ecx\n\t"
        "mov %4, %%edx\n\t"
        "int $0x80\n\t"
        "mov %%eax, %0"
        : "=r" (ret)
        : "r" (num), "r" (arg1), "r" (arg2), "r" (arg3)
        : "eax", "ebx", "ecx", "edx", "memory"
    );
    return ret;
}

void sys_print(const char *s) {
    syscall(1, (int)s, 0, 0);
}

int sys_getpid() {
    return syscall(2, 0, 0, 0);
}

int sys_readkey() {
    return syscall(3, 0, 0, 0);
}