#include "minios.h"

static char input_buffer[256];
static int input_pos = 0;

extern int keyboard_available();
extern int keyboard_read_buffer(char *buf, int max_len);

void parse_command(const char *cmd) {
    if (cmd[0] == 0) return;
    
    const char *args[32];
    int arg_count = 0;
    
    const char *p = cmd;
    while (*p == ' ') p++;
    
    char current_arg[64];
    int current_len = 0;
    
    while (*p) {
        if (*p == ' ' || *p == '\n' || *p == '\r') {
            if (current_len > 0) {
                current_arg[current_len] = 0;
                args[arg_count] = (const char*)kmalloc(64);
                for (int i = 0; i <= current_len; i++) {
                    ((char*)args[arg_count])[i] = current_arg[i];
                }
                arg_count++;
                current_len = 0;
            }
        } else {
            if (current_len < 63) {
                current_arg[current_len++] = *p;
            }
        }
        p++;
    }
    
    if (current_len > 0) {
        current_arg[current_len] = 0;
        args[arg_count] = (const char*)kmalloc(64);
        for (int i = 0; i <= current_len; i++) {
            ((char*)args[arg_count])[i] = current_arg[i];
        }
        arg_count++;
    }
    
    if (arg_count == 0) return;
    
    const char *command = args[0];
    
    if (command[0] == 'h' && command[1] == 'e' && command[2] == 'l' && command[3] == 'p') {
        printk("MiniOS v0.01 - Available commands:\n");
        printk("  help     - Show this help message\n");
        printk("  clear    - Clear the screen\n");
        printk("  ls       - List files in current directory\n");
        printk("  echo     - Print text to screen\n");
        printk("  uptime   - Show system uptime\n");
        printk("  meminfo  - Show memory info\n");
        printk("  ps       - Show processes\n");
        printk("  reboot   - Reboot the system\n");
    }
    else if (command[0] == 'c' && command[1] == 'l' && command[2] == 'e' && command[3] == 'a' && command[4] == 'r') {
        clear_screen();
        printk("MiniOS v0.01\r\n");
    }
    else if (command[0] == 'l' && command[1] == 's') {
        list_files();
    }
    else if (command[0] == 'e' && command[1] == 'c' && command[2] == 'h' && command[3] == 'o') {
        for (int i = 1; i < arg_count; i++) {
            printk("%s ", args[i]);
        }
        printk("\n");
    }
    else if (command[0] == 'u' && command[1] == 'p' && command[2] == 't' && command[3] == 'i' && command[4] == 'm' && command[5] == 'e') {
        extern uint32_t jiffies;
        printk("System uptime: %d seconds (jiffies: %d)\n", jiffies / 100, jiffies);
    }
    else if (command[0] == 'm' && command[1] == 'e' && command[2] == 'm' && command[3] == 'i' && command[4] == 'n' && command[5] == 'f' && command[6] == 'o') {
        printk("Memory Info:\n");
        printk("  Kernel heap start: 0x100000\n");
        printk("  Kernel heap size:  0x100000 (1 MB)\n");
        printk("  VGA memory:        0xB8000\n");
    }
    else if (command[0] == 'p' && command[1] == 's') {
        printk("Process List:\n");
        printk("  PID | Name    | State\n");
        printk("  ----|---------|------\n");
        printk("  0   | idle    | Running\n");
        printk("  1   | shell   | Running\n");
    }
    else if (command[0] == 'r' && command[1] == 'e' && command[2] == 'b' && command[3] == 'o' && command[4] == 'o' && command[5] == 't') {
        printk("Rebooting...\n");
        for (int i = 0; i < 1000000; i++);
        __asm__ volatile ("int $0x19");
    }
    else if (command[0] == 'c' && command[1] == 'a' && command[2] == 't') {
        if (arg_count > 1) {
            char buf[1024];
            if (read_file(args[1], buf, 1024) == 0) {
                printk("%s\n", buf);
            } else {
                printk("File not found: %s\n", args[1]);
            }
        } else {
            printk("Usage: cat \n");
        }
    }
    else {
        printk("Unknown command: %s\n", command);
        printk("Type 'help' for available commands.\n");
    }
    
    for (int i = 0; i < arg_count; i++) {
        kfree((void*)args[i]);
    }
}

void shell_init() {
    input_pos = 0;
    printk("\n");
    printk("  ___  ___  ___  ___  ___  ___\n");
    printk(" | _ \\/ __|/ _ \\| __||_ _|| _ \\\n");
    printk(" |  _/\\__ \\ (_) | _|  | | |   /\n");
    printk(" |_|  |___/\\___/|_|  |___||_|_\\\n");
    printk("\n");
    printk("MiniOS v0.01 - Type 'help' for commands\n");
    printk("\n");
}

void shell_execute(const char *cmd) {
    parse_command(cmd);
}

void shell_mainloop() {
    printk("root@minios:~$ ");
    
    while (1) {
        if (keyboard_available()) {
            int len = keyboard_read_buffer(input_buffer, 255);
            if (len > 0) {
                input_buffer[len] = 0;
                
                printk("%s\n", input_buffer);
                
                shell_execute(input_buffer);
                
                input_pos = 0;
                printk("root@minios:~$ ");
            }
        }
        
        for (int i = 0; i < 1000; i++);
    }
}