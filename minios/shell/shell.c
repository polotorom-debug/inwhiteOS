#include "minios.h"

static char input_buffer[256];
static int input_pos = 0;

static int strlen_simple(const char *s) {
    int len = 0;
    if (!s) return 0;
    while (s[len]) len++;
    return len;
}

static void print_prompt(void) {
    printk("root@minios:~$ ");
}

static int strcmp_simple(const char *a, const char *b) {
    if (!a || !b) return 1;
    while (*a && *b) {
        if (*a != *b) return 1;
        a++; b++;
    }
    return (*a != *b);
}

static void cmd_help(void) {
    printk("MiniOS v0.02 - Available commands:\n");
    printk("  help          - Show this help\n");
    printk("  clear         - Clear screen\n");
    printk("  ls            - List files\n");
    printk("  echo <text>   - Print text\n");
    printk("  uptime        - Show system uptime\n");
    printk("  meminfo       - Show memory info\n");
    printk("  ps            - Show processes\n");
    printk("  reboot        - Reboot system\n");
    printk("  cat <file>    - Read file\n");
    printk("  touch <file>  - Create file\n");
    printk("  rm <file>     - Delete file\n");
    printk("  write <f> <t> - Write text to file\n");
    printk("  version       - Show version\n");
}

static void cmd_meminfo(void) {
    u32 total, used, free_mem;
    kmem_stats(&total, &used, &free_mem);
    printk("Memory Info:\n");
    printk("  Total heap:  %d bytes (%d KB)\n", total, total / 1024);
    printk("  Used:        %d bytes (%d KB)\n", used, used / 1024);
    printk("  Free:        %d bytes (%d KB)\n", free_mem, free_mem / 1024);
    printk("  VGA memory:  0xB8000 (4KB)\n");
    printk("  Kernel base: 0x1000\n");
}

static void cmd_ps(void) {
    printk("Process List:\n");
    printk("  PID | Name               | State   | Pri\n");
    printk("  ----|--------------------|---------|----\n");
    process_t *procs;
    int count, num;
    num = get_process_list(&procs, &count);
    (void)num;

    for (int i = 0; i < count; i++) {
        if (procs[i].pid >= 0) {
            const char *state_str = "?";
            if (procs[i].state == PROC_FREE) state_str = "Free";
            else if (procs[i].state == PROC_RUNNING) state_str = "Running";
            else if (procs[i].state == PROC_READY) state_str = "Ready";
            else if (procs[i].state == PROC_SLEEPING) state_str = "Sleep";
            else if (procs[i].state == PROC_ZOMBIE) state_str = "Zombie";

            printk("  %-4d| %-20s | %-8s| %d\n",
                   procs[i].pid, procs[i].name, state_str, procs[i].priority);
        }
    }
}

static void cmd_uptime(void) {
    u32 secs = jiffies / 100;
    u32 mins = secs / 60;
    secs = secs % 60;
    printk("System uptime: %d min %d sec (jiffies: %d)\n", mins, secs, jiffies);
}

static void cmd_reboot(void) {
    printk("Rebooting...\n");
    sleep_ms(500);
    __asm__ volatile ("int $0x19");
}

static void cmd_version(void) {
    printk("MiniOS v0.02 - Educational OS\n");
    printk("Architecture: i386\n");
    printk("Mode: Protected 32-bit\n");
    printk("Memory: Paging enabled\n");
    printk("Scheduler: Round-Robin\n");
}

static void parse_command(const char *cmd) {
    if (!cmd || cmd[0] == '\0' || cmd[0] == '\n') return;

    const char *p = cmd;
    while (*p == ' ') p++;

    if (strcmp_simple(p, "help") == 0) { cmd_help(); return; }
    if (strcmp_simple(p, "clear") == 0) { clear_screen(); printk("MiniOS v0.02\r\n"); return; }
    if (strcmp_simple(p, "ls") == 0) { list_files(); return; }
    if (strcmp_simple(p, "meminfo") == 0) { cmd_meminfo(); return; }
    if (strcmp_simple(p, "ps") == 0) { cmd_ps(); return; }
    if (strcmp_simple(p, "uptime") == 0) { cmd_uptime(); return; }
    if (strcmp_simple(p, "reboot") == 0) { cmd_reboot(); return; }
    if (strcmp_simple(p, "version") == 0) { cmd_version(); return; }

    if (p[0] == 'e' && p[1] == 'c' && p[2] == 'h' && p[3] == 'o' && p[4] == ' ') {
        printk("%s\n", p + 5);
        return;
    }

    if (p[0] == 't' && p[1] == 'o' && p[2] == 'u' && p[3] == 'c' && p[4] == 'h' && p[5] == ' ') {
        const char *fname = p + 6;
        int ret = create_file(fname);
        if (ret >= 0) printk("Created file: %s\n", fname);
        else if (ret == -2) printk("File already exists: %s\n", fname);
        else printk("Failed to create file\n");
        return;
    }

    if (p[0] == 'r' && p[1] == 'm' && p[2] == ' ') {
        const char *fname = p + 3;
        int ret = delete_file(fname);
        if (ret == 0) printk("Deleted: %s\n", fname);
        else printk("File not found: %s\n", fname);
        return;
    }

    if (p[0] == 'c' && p[1] == 'a' && p[2] == 't' && p[3] == ' ') {
        const char *fname = p + 4;
        char buf[1024];
        int ret = read_file(fname, buf, 1023);
        if (ret > 0) {
            buf[ret] = '\0';
            printk("%s\n", buf);
        } else {
            printk("File not found or empty: %s\n", fname);
        }
        return;
    }

    if (p[0] == 'w' && p[1] == 'r' && p[2] == 'i' && p[3] == 't' && p[4] == 'e' && p[5] == ' ') {
        const char *fname = p + 6;
        const char *space = fname;
        while (*space && *space != ' ') space++;
        if (*space == ' ') {
            int len = (int)(space - fname);
            char name[32];
            for (int i = 0; i < len && i < 31; i++) name[i] = fname[i];
            name[len] = '\0';
            const char *text = space + 1;
            int ret = write_file(name, (void *)text, (u32)strlen_simple(text));
            if (ret >= 0) printk("Wrote %d bytes to %s\n", ret, name);
            else printk("Failed to write to %s\n", name);
        }
        return;
    }

    printk("Unknown command: ");
    const char *t = p;
    while (*t && *t != ' ' && *t != '\n') putchar(*t++);
    printk("\nType 'help' for available commands.\n");
}

void shell_init(void) {
    input_pos = 0;
    input_buffer[0] = '\0';
    printk("\n");
    printk("  ___  ___  ___  ___  ___  ___\n");
    printk(" | _ \\/ __|/ _ \\| __||_ _|| _ \\\n");
    printk(" |  _/\\__ \\ (_) | _|  | | |   /\n");
    printk(" |_|  |___/\\___/|_|  |___||_|_\\\n");
    printk("\n");
    printk("MiniOS v0.02 - Type 'help' for commands\n");
    printk("\n");
}

void shell_execute(const char *cmd) {
    parse_command(cmd);
}

void shell_mainloop(void) {
    print_prompt();

    while (1) {
        if (keyboard_available()) {
            char c = (char)keyboard_read_char();
            if (c > 0) {
                if (c == '\n' || c == '\r') {
                    printk("\n");
                    input_buffer[input_pos] = '\0';

                    if (input_pos > 0) {
                        shell_execute(input_buffer);
                    }

                    input_pos = 0;
                    print_prompt();
                } else if (c == '\b') {
                    if (input_pos > 0) {
                        input_pos--;
                        putchar('\b');
                    }
                } else if (c >= 32 && c < 127) {
                    if (input_pos < 255) {
                        input_buffer[input_pos++] = c;
                        putchar(c);
                    }
                }
            }
        }

        for (volatile int i = 0; i < 1000; i++);
    }
}
