#include "minios.h"

#define KERNEL_HEAP_START 0x100000
#define KERNEL_HEAP_SIZE 0x100000

typedef struct block_header {
    struct block_header *next;
    int size;
    int free;
} block_header_t;

static block_header_t *heap_start = NULL;
static block_header_t *heap_end = NULL;
static int heap_initialized = 0;

void init_kmalloc() {
    heap_start = (block_header_t*)KERNEL_HEAP_START;
    heap_start->next = NULL;
    heap_start->size = 0;
    heap_start->free = 0;
    heap_end = heap_start;
    heap_initialized = 1;
    
    printk("Memory manager initialized at 0x%x\n", KERNEL_HEAP_START);
    printk("Heap size: %d KB\n", KERNEL_HEAP_SIZE / 1024);
}

void *kmalloc(uint32_t size) {
    if (!heap_initialized) {
        return NULL;
    }
    
    if (size < sizeof(block_header_t)) {
        size = sizeof(block_header_t);
    }
    
    block_header_t *current = heap_start;
    
    while (current) {
        if (current->free && current->size >= size) {
            current->free = 0;
            return (void*)(current + 1);
        }
        
        if (!current->next) {
            break;
        }
        current = current->next;
    }
    
    block_header_t *new_block = (block_header_t*)((char*)heap_end + sizeof(block_header_t) + heap_end->size);
    
    if ((char*)new_block + sizeof(block_header_t) + size > (char*)KERNEL_HEAP_START + KERNEL_HEAP_SIZE) {
        printk("ERROR: Out of memory!\n");
        return NULL;
    }
    
    new_block->size = size;
    new_block->free = 0;
    new_block->next = NULL;
    
    if (heap_end->free && heap_end->size >= size) {
        heap_end->free = 0;
        return (void*)(heap_end + 1);
    }
    
    heap_end->next = new_block;
    heap_end = new_block;
    
    return (void*)(new_block + 1);
}

void kfree(void *ptr) {
    if (!ptr) return;
    
    block_header_t *block = (block_header_t*)ptr - 1;
    block->free = 1;
    
    block_header_t *current = heap_start;
    while (current && current->next) {
        if (current->free && current->next->free) {
            current->size += sizeof(block_header_t) + current->next->size;
            current->next = current->next->next;
            if (current->next == NULL) {
                heap_end = current;
            }
        } else {
            current = current->next;
        }
    }
}

void *kzalloc(uint32_t size) {
    void *ptr = kmalloc(size);
    if (ptr) {
        char *p = (char*)ptr;
        for (uint32_t i = 0; i < size; i++) {
            p[i] = 0;
        }
    }
    return ptr;
}