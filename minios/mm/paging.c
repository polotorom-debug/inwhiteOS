#include "minios.h"

extern void switch_page_directory(u32 pd_addr);
extern void enable_page_paging(void);

static page_directory_t *kernel_page_dir = NULL;
static page_directory_t *current_page_dir = NULL;

static u32 placement_addr = 0x200000;

static page_t *pages = NULL;
static u32 nframes = 0;

static void set_frame(u32 frame_addr) {
    u32 frame = frame_addr / PAGE_SIZE;
    pages[frame / 32] |= (1 << (frame % 32));
}

static void unset_frame(u32 frame_addr) {
    u32 frame = frame_addr / PAGE_SIZE;
    pages[frame / 32] &= ~(1 << (frame % 32));
}

static int test_frame(u32 frame_addr) {
    u32 frame = frame_addr / PAGE_SIZE;
    return (pages[frame / 32] & (1 << (frame % 32)));
}

static u32 first_free_frame(void) {
    for (u32 i = 0; i < nframes / 32; i++) {
        if (pages[i] != 0xFFFFFFFF) {
            for (u32 j = 0; j < 32; j++) {
                if (!(pages[i] & (1 << j))) {
                    return i * 32 + j;
                }
            }
        }
    }
    return (u32)-1;
}

void init_paging(void) {
    u32 mem_size = 16 * 1024 * 1024;
    nframes = mem_size / PAGE_SIZE;

    pages = (u32 *)0x1000;
    for (u32 i = 0; i < nframes / 32; i++) {
        pages[i] = 0;
    }

    set_frame(0x1000);

    kernel_page_dir = (page_directory_t *)kzalloc(sizeof(page_directory_t));
    current_page_dir = kernel_page_dir;

    for (u32 i = 0; i < PAGE_DIR_ENTRIES; i++) {
        kernel_page_dir->tables[i] = NULL;
        kernel_page_dir->tablesPhysical[i] = 0;
    }

    for (u32 i = 0; i < 4; i++) {
        u32 phys = 0x10000 + (i * PAGE_SIZE);
        set_frame(phys);
        kernel_page_dir->tables[i] = (u32 *)phys;
        kernel_page_dir->tablesPhysical[i] = phys | PAGE_PRESENT | PAGE_WRITE;

        for (u32 j = 0; j < 1024; j++) {
            kernel_page_dir->tables[i][j] = (i * 1024 + j) * PAGE_SIZE | PAGE_PRESENT | PAGE_WRITE;
        }
    }

    kernel_page_dir->physicalAddr = (u32)kernel_page_dir->tablesPhysical;

    switch_page_directory((u32)kernel_page_dir);
    enable_page_paging();

    printk("Paging initialized (4MB identity mapped)\n");
}

page_t *get_page(u32 address, int make, page_directory_t *dir) {
    address /= PAGE_SIZE;
    u32 table_idx = address / 1024;

    if (dir->tables[table_idx]) {
        return &dir->tables[table_idx][address % 1024];
    } else if (make) {
        u32 tmp;
        u32 *table = (u32 *)kmalloc_aligned(PAGE_SIZE, PAGE_SIZE);
        dir->tables[table_idx] = table;
        dir->tablesPhysical[table_idx] = (u32)table | PAGE_PRESENT | PAGE_WRITE;
        return &table[address % 1024];
    } else {
        return NULL;
    }
}

void alloc_page(u32 address, page_directory_t *dir) {
    page_t *page = get_page(address, 1, dir);
    if (page->present) return;
    u32 frame = first_free_frame();
    if (frame == (u32)-1) {
        printk("ERROR: No free frames!\n");
        return;
    }
    set_frame(frame * PAGE_SIZE);
    page->frame = frame;
    page->present = 1;
    page->rw = 1;
    page->user = 1;
}

void free_page(u32 address, page_directory_t *dir) {
    page_t *page = get_page(address, 0, dir);
    if (!page || !page->present) return;
    unset_frame(page->frame * PAGE_SIZE);
    page->present = 0;
    page->frame = 0;
}

page_directory_t *clone_page_directory(page_directory_t *src) {
    page_directory_t *dir = (page_directory_t *)kzalloc(sizeof(page_directory_t));

    for (u32 i = 0; i < PAGE_DIR_ENTRIES; i++) {
        if (!src->tables[i]) continue;

        u32 *table = (u32 *)kmalloc_aligned(PAGE_SIZE, PAGE_SIZE);
        dir->tables[i] = table;
        dir->tablesPhysical[i] = (u32)table | PAGE_PRESENT | PAGE_WRITE;

        for (u32 j = 0; j < 1024; j++) {
            if (src->tables[i][j] & PAGE_PRESENT) {
                alloc_page(i * 1024 * PAGE_SIZE + j * PAGE_SIZE, dir);
            }
        }
    }

    dir->physicalAddr = (u32)dir->tablesPhysical;
    return dir;
}
