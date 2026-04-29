#include "minios.h"

#define KERNEL_HEAP_START 0x300000
#define KERNEL_HEAP_SIZE 0x400000
#define KMAGIC 0xCAFEBABE
#define KFREE_MAGIC 0xDEADBEEF

typedef struct block_header {
    u32 magic;
    struct block_header *next;
    struct block_header *prev;
    u32 size;
    int free;
    const char *file;
    int line;
} block_header_t;

static block_header_t *heap_start = NULL;
static block_header_t *heap_end = NULL;
static int heap_initialized = 0;
static u32 total_allocated = 0;
static u32 total_freed = 0;
static u32 alloc_count = 0;
static u32 free_count = 0;

static void coalesce(block_header_t *block) {
    if (!block || !block->free) return;

    if (block->next && block->next->free) {
        block_header_t *n = block->next;
        block->size += sizeof(block_header_t) + n->size;
        block->next = n->next;
        if (n->next) {
            n->next->prev = block;
        }
        n->magic = KFREE_MAGIC;
        n->size = 0;
        if (heap_end == n) heap_end = block;
    }

    if (block->prev && block->prev->free) {
        block_header_t *p = block->prev;
        p->size += sizeof(block_header_t) + block->size;
        p->next = block->next;
        if (block->next) {
            block->next->prev = p;
        }
        block->magic = KFREE_MAGIC;
        block->size = 0;
        if (heap_end == block) heap_end = p;
    }
}

void init_kmalloc(void) {
    heap_start = (block_header_t *)KERNEL_HEAP_START;
    heap_start->magic = KMAGIC;
    heap_start->next = NULL;
    heap_start->prev = NULL;
    heap_start->size = KERNEL_HEAP_SIZE - sizeof(block_header_t);
    heap_start->free = 1;
    heap_start->file = NULL;
    heap_start->line = 0;
    heap_end = heap_start;
    heap_initialized = 1;
    total_allocated = 0;
    total_freed = 0;
    alloc_count = 0;
    free_count = 0;

    printk("Memory manager initialized at 0x%x\n", KERNEL_HEAP_START);
    printk("Heap size: %d KB\n", KERNEL_HEAP_SIZE / 1024);
}

void *kmalloc(uint32_t size) {
    if (!heap_initialized || size == 0) {
        return NULL;
    }

    if (size % 4) {
        size += 4 - (size % 4);
    }

    block_header_t *current = heap_start;

    while (current) {
        if (current->magic != KMAGIC) {
            printk("ERROR: Heap corruption detected!\n");
            return NULL;
        }

        if (current->free && current->size >= size) {
            u32 remaining = current->size - size;
            if (remaining > sizeof(block_header_t) + 16) {
                block_header_t *new_block = (block_header_t *)((u8 *)current + sizeof(block_header_t) + size);
                new_block->magic = KMAGIC;
                new_block->size = remaining - sizeof(block_header_t);
                new_block->free = 1;
                new_block->next = current->next;
                new_block->prev = current;
                new_block->file = NULL;
                new_block->line = 0;

                if (current->next) {
                    current->next->prev = new_block;
                }
                current->next = new_block;
                current->size = size;

                if (heap_end == current) {
                    heap_end = new_block;
                }
            }

            current->free = 0;
            current->file = NULL;
            current->line = 0;
            total_allocated += current->size;
            alloc_count++;

            return (void *)((u8 *)current + sizeof(block_header_t));
        }

        current = current->next;
    }

    printk("ERROR: Out of memory!\n");
    return NULL;
}

void *kmalloc_aligned(uint32_t size, uint32_t align) {
    if (!heap_initialized || size == 0) return NULL;

    u32 adjusted_size = size + align;
    u8 *ptr = (u8 *)kmalloc(adjusted_size);
    if (!ptr) return NULL;

    u32 offset = (u32)ptr % align;
    if (offset) {
        ptr += align - offset;
    }

    return (void *)ptr;
}

void kfree(void *ptr) {
    if (!ptr || !heap_initialized) return;

    block_header_t *block = (block_header_t *)((u8 *)ptr - sizeof(block_header_t));

    if (block->magic != KMAGIC) {
        printk("ERROR: Invalid free or double free!\n");
        return;
    }

    if (block->free) {
        printk("ERROR: Double free detected!\n");
        return;
    }

    total_freed += block->size;
    free_count++;
    block->free = 1;
    block->file = NULL;
    block->line = 0;

    coalesce(block);
}

void *kzalloc(uint32_t size) {
    void *ptr = kmalloc(size);
    if (ptr) {
        u8 *p = (u8 *)ptr;
        for (u32 i = 0; i < size; i++) {
            p[i] = 0;
        }
    }
    return ptr;
}

void *krealloc(void *ptr, uint32_t new_size) {
    if (!ptr) return kmalloc(new_size);
    if (new_size == 0) {
        kfree(ptr);
        return NULL;
    }

    block_header_t *block = (block_header_t *)((u8 *)ptr - sizeof(block_header_t));

    if (block->magic != KMAGIC) {
        printk("ERROR: Invalid pointer in realloc!\n");
        return NULL;
    }

    if (new_size <= block->size) return ptr;

    void *new_ptr = kmalloc(new_size);
    if (new_ptr) {
        for (u32 i = 0; i < block->size; i++) {
            ((u8 *)new_ptr)[i] = ((u8 *)ptr)[i];
        }
        kfree(ptr);
    }

    return new_ptr;
}

void kmem_stats(u32 *total, u32 *used, u32 *free_mem) {
    if (!heap_initialized) {
        if (total) *total = 0;
        if (used) *used = 0;
        if (free_mem) *free_mem = 0;
        return;
    }

    u32 t = 0, u = 0, f = 0;
    block_header_t *current = heap_start;

    while (current) {
        if (current->magic == KMAGIC) {
            t += current->size + sizeof(block_header_t);
            if (current->free) {
                f += current->size;
            } else {
                u += current->size;
            }
        }
        current = current->next;
    }

    if (total) *total = t;
    if (used) *used = u;
    if (free_mem) *free_mem = f;
}
