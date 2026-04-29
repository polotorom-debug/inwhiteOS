#include "minios.h"

#define SYS_PRINT 1
#define SYS_GETPID 2
#define SYS_READKEY 3
#define SYS_KBD_AVAIL 4
#define SYS_CLEAR 5
#define SYS_EXIT 6
#define SYS_WRITE_FILE 7
#define SYS_READ_FILE 8
#define SYS_LIST_FILES 9
#define SYS_SLEEP 10

int handle_syscall(registers_t *regs) {
    int sysno = regs->eax;

    switch (sysno) {
        case SYS_PRINT: {
            const char *s = (const char *)regs->ebx;
            if (s) { while (*s) putchar(*s++); }
            break;
        }
        case SYS_GETPID: {
            return get_current_pid();
        }
        case SYS_READKEY: {
            return keyboard_read_char();
        }
        case SYS_KBD_AVAIL: {
            return keyboard_available() ? 1 : 0;
        }
        case SYS_CLEAR: {
            clear_screen();
            break;
        }
        case SYS_EXIT: {
            int pid = get_current_pid();
            if (pid != 0) kill_process(pid);
            break;
        }
        case SYS_WRITE_FILE: {
            const char *name = (const char *)regs->ebx;
            void *buf = (void *)regs->ecx;
            u32 size = (u32)regs->edx;
            return write_file(name, buf, size);
        }
        case SYS_READ_FILE: {
            const char *name = (const char *)regs->ebx;
            void *buf = (void *)regs->ecx;
            u32 size = (u32)regs->edx;
            return read_file(name, buf, size);
        }
        case SYS_LIST_FILES: {
            list_files();
            break;
        }
        case SYS_SLEEP: {
            u32 ticks = (u32)regs->ebx;
            sleep_ticks(ticks);
            break;
        }
        default:
            printk("Unknown syscall: %d\n", sysno);
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

void sys_print(const char *s) { syscall(SYS_PRINT, (int)s, 0, 0); }
int sys_getpid(void) { return syscall(SYS_GETPID, 0, 0, 0); }
int sys_readkey(void) { return syscall(SYS_READKEY, 0, 0, 0); }
int sys_kbd_avail(void) { return syscall(SYS_KBD_AVAIL, 0, 0, 0); }
void sys_clear(void) { syscall(SYS_CLEAR, 0, 0, 0); }
void sys_exit(void) { syscall(SYS_EXIT, 0, 0, 0); }
int sys_write_file(const char *n, void *b, u32 s) { return syscall(SYS_WRITE_FILE, (int)n, (int)b, s); }
int sys_read_file(const char *n, void *b, u32 s) { return syscall(SYS_READ_FILE, (int)n, (int)b, s); }
void sys_list_files(void) { syscall(SYS_LIST_FILES, 0, 0, 0); }
void sys_sleep(u32 t) { syscall(SYS_SLEEP, t, 0, 0); }
