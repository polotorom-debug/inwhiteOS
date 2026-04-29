#include "minios.h"

#define MAX_PROCESSES 16

typedef struct process {
    int pid;
    char name[32];
    u32 esp;
    u32 ebp;
    u32 eip;
    int state;
    int priority;
    u32 stack[1024];
} process_t;

extern u32 jiffies;

static process_t processes[MAX_PROCESSES];
static int num_processes = 0;
static int current_pid = 0;
uint32_t current_esp = 0;
uint32_t next_process_esp = 0;

void init_process() {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        processes[i].pid = -1;
    }
    
    process_t *idle = &processes[0];
    idle->pid = 0;
    for (int i = 0; i < 32; i++) idle->name[i] = 0;
    idle->name[0] = 'i';
    idle->name[1] = 'd';
    idle->name[2] = 'l';
    idle->name[3] = 'e';
    idle->state = 1;
    idle->priority = 0;
    idle->esp = (u32)&idle->stack[1023];
    idle->ebp = (u32)&idle->stack[1023];
    
    num_processes = 1;
    current_pid = 0;
    
    printk("Process manager initialized with %d processes\n", num_processes);
}

int create_process(const char *name, void (*func)()) {
    if (num_processes >= MAX_PROCESSES) {
        return -1;
    }
    
    process_t *proc = &processes[num_processes];
    
    proc->pid = num_processes;
    for (int i = 0; i < 31 && name[i]; i++) {
        proc->name[i] = name[i];
    }
    proc->name[31] = 0;
    
    proc->stack[1023] = (u32)func;
    proc->stack[1022] = 0;
    proc->stack[1021] = 0;
    proc->stack[1020] = 0;
    proc->stack[1019] = 0;
    proc->stack[1018] = 0;
    proc->stack[1017] = 0;
    proc->esp = (u32)&proc->stack[1017];
    proc->ebp = (u32)&proc->stack[1017];
    proc->eip = (u32)func;
    proc->state = 1;
    proc->priority = 1;
    
    num_processes++;
    
    printk("Created process: %s (PID: %d)\n", name, proc->pid);
    
    return proc->pid;
}

void schedule() {
    if (num_processes <= 1) return;
    
    int next_pid = (current_pid + 1) % num_processes;
    process_t *next = &processes[next_pid];
    
    if (next->pid == -1) {
        next_pid = 0;
        next = &processes[0];
    }
    
    __asm__ volatile (
        "mov %%esp, %0"
        : "=r" (current_esp)
    );
    
    processes[current_pid].esp = current_esp;
    
    current_pid = next_pid;
    next_process_esp = next->esp;
    
    __asm__ volatile (
        "mov %0, %%esp"
        :
        : "r" (next_process_esp)
    );
}

int get_current_pid() {
    return current_pid;
}

const char *get_current_process_name() {
    return processes[current_pid].name;
}