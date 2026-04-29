#include "minios.h"

#define MAX_PROCESSES 16
#define QUANTUM_TICKS 10

extern u32 jiffies;

static process_t processes[MAX_PROCESSES];
static int num_processes = 0;
static int current_pid = 0;
static int scheduler_enabled = 0;

void init_process(void) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        processes[i].pid = -1;
        processes[i].state = PROC_FREE;
        processes[i].name[0] = '\0';
        processes[i].page_dir = NULL;
    }

    process_t *idle = &processes[0];
    idle->pid = 0;
    idle->name[0] = 'i'; idle->name[1] = 'd'; idle->name[2] = 'l'; idle->name[3] = 'e'; idle->name[4] = '\0';
    idle->state = PROC_RUNNING;
    idle->priority = 0;
    idle->ticks = 0;
    idle->eip = 0;
    idle->eflags = 0;
    idle->esp = (u32)&idle->kernel_stack[1023];
    idle->ebp = (u32)&idle->kernel_stack[1023];
    idle->page_dir = NULL;

    num_processes = 1;
    current_pid = 0;

    printk("Process manager initialized (max: %d)\n", MAX_PROCESSES);
}

int create_process(const char *name, void (*func)(void), int priority) {
    if (!func || num_processes >= MAX_PROCESSES) return -1;

    int slot = -1;
    for (int i = 1; i < MAX_PROCESSES; i++) {
        if (processes[i].pid == -1) {
            slot = i;
            break;
        }
    }
    if (slot == -1) return -1;

    process_t *proc = &processes[slot];

    proc->pid = slot;
    for (int i = 0; i < 31 && name[i]; i++) proc->name[i] = name[i];
    proc->name[31] = '\0';

    for (int i = 0; i < 1024; i++) proc->kernel_stack[i] = 0;

    u32 stack_top = (u32)&proc->kernel_stack[1024];

    stack_top -= sizeof(u32);
    *(u32 *)(stack_top) = 0;

    stack_top -= sizeof(u32);
    *(u32 *)(stack_top) = (u32)func;

    stack_top -= 7 * sizeof(u32);
    u32 *regs = (u32 *)stack_top;
    regs[0] = 0;
    regs[1] = 0;
    regs[2] = 0;
    regs[3] = 0;
    regs[4] = 0;
    regs[5] = 0;
    regs[6] = 0;

    proc->esp = stack_top;
    proc->ebp = stack_top;
    proc->eip = (u32)func;
    proc->eflags = 0x202;
    proc->state = PROC_READY;
    proc->priority = priority > 0 ? priority : 1;
    proc->ticks = 0;
    proc->page_dir = NULL;

    num_processes++;

    printk("Created process: %s (PID: %d)\n", name, proc->pid);
    return proc->pid;
}

void switch_context(u32 *old_esp, u32 *new_esp) {
    __asm__ volatile (
        "mov %%esp, %0\n"
        "mov %1, %%esp\n"
        : "=r"(*old_esp) : "r"(*new_esp)
    );
}

int get_next_pid(void) {
    int best = -1;
    int best_prio = 999;

    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].state == PROC_READY || processes[i].state == PROC_RUNNING) {
            if (processes[i].priority < best_prio) {
                best_prio = processes[i].priority;
                best = i;
            } else if (processes[i].priority == best_prio && i != current_pid) {
                best = i;
            }
        }
    }

    if (best == -1) best = 0;
    return best;
}

void schedule(void) {
    if (!scheduler_enabled || num_processes <= 1) return;

    int old_pid = current_pid;
    if (processes[old_pid].state == PROC_RUNNING) {
        processes[old_pid].state = PROC_READY;
    }

    int next_pid = get_next_pid();

    if (next_pid == old_pid) return;

    u32 *old_esp = &processes[old_pid].esp;
    u32 *new_esp = &processes[next_pid].esp;

    processes[next_pid].state = PROC_RUNNING;
    processes[next_pid].ticks++;

    if (processes[next_pid].page_dir) {
        __asm__ volatile ("mov %0, %%cr3" : : "r"(processes[next_pid].page_dir->physicalAddr));
    }

    current_esp = *old_esp;
    current_pid = next_pid;

    switch_context(old_esp, new_esp);
}

int kill_process(int pid) {
    if (pid < 0 || pid >= MAX_PROCESSES || processes[pid].pid == -1) return -1;
    if (pid == 0) return -1;

    processes[pid].state = PROC_ZOMBIE;
    processes[pid].pid = -1;
    processes[pid].name[0] = '\0';
    num_processes--;

    if (current_pid == pid) {
        current_pid = 0;
        processes[0].state = PROC_RUNNING;
    }

    printk("Killed process %d\n", pid);
    return 0;
}

int get_current_pid(void) {
    return current_pid;
}

const char *get_current_process_name(void) {
    return processes[current_pid].name;
}

void scheduler_toggle(int enable) {
    scheduler_enabled = enable;
}

int get_process_list(process_t **list, int *count) {
    *list = processes;
    *count = MAX_PROCESSES;
    return num_processes;
}
