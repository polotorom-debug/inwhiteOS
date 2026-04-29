#include "minios.h"

#define MINIX_MAGIC 0x4D5A
#define MINIX_BLOCK_SIZE 1024
#define MINIX_MAX_FILES 64
#define MINIX_MAX_FILENAME 30
#define MINIX_NUM_ZONE_PTRS 7

typedef struct minix_super_block {
    u16 s_ninodes;
    u16 s_nzones;
    u16 s_imap_blocks;
    u16 s_zmap_blocks;
    u16 s_firstdatazone;
    u16 s_log_zone_size;
    u32 s_max_size;
    u16 s_magic;
    u16 s_state;
} __attribute__((packed)) minix_super_block_t;

typedef struct minix_inode {
    u16 i_mode;
    u16 i_uid;
    u32 i_size;
    u32 i_mtime;
    u8 i_gid;
    u8 i_nlinks;
    u16 i_zone[MINIX_NUM_ZONE_PTRS];
} __attribute__((packed)) minix_inode_t;

typedef struct minix_dir_entry {
    u16 inode;
    char name[MINIX_MAX_FILENAME];
} __attribute__((packed)) minix_dir_entry_t;

typedef struct fs_file {
    char name[MINIX_MAX_FILENAME];
    int inode_num;
    u32 size;
    int in_use;
    char *data;
} fs_file_t;

static minix_super_block_t super_block;
static u8 *inode_bitmap;
static u8 *zone_bitmap;
static minix_inode_t inodes[MINIX_MAX_FILES];
static fs_file_t files[MINIX_MAX_FILES];
static int fs_initialized = 0;
static u8 ram_disk[MINIX_MAX_FILES * MINIX_BLOCK_SIZE];

void init_fs(void) {
    super_block.s_magic = MINIX_MAGIC;
    super_block.s_ninodes = MINIX_MAX_FILES;
    super_block.s_nzones = 512;
    super_block.s_imap_blocks = 1;
    super_block.s_zmap_blocks = 1;
    super_block.s_firstdatazone = 36;
    super_block.s_log_zone_size = 0;
    super_block.s_max_size = 0x7FFFFFFF;
    super_block.s_state = 1;

    inode_bitmap = (u8 *)kzalloc(1024);
    zone_bitmap = (u8 *)kzalloc(1024);

    for (int i = 0; i < MINIX_MAX_FILES; i++) {
        inodes[i].i_mode = 0;
        inodes[i].i_size = 0;
        inodes[i].i_uid = 0;
        inodes[i].i_gid = 0;
        inodes[i].i_nlinks = 0;
        inodes[i].i_mtime = 0;
        for (int j = 0; j < MINIX_NUM_ZONE_PTRS; j++) {
            inodes[i].i_zone[j] = 0;
        }

        files[i].name[0] = '\0';
        files[i].inode_num = -1;
        files[i].size = 0;
        files[i].in_use = 0;
        files[i].data = NULL;
    }

    for (u32 i = 0; i < MINIX_MAX_FILES * MINIX_BLOCK_SIZE; i++) {
        ram_disk[i] = 0;
    }

    fs_initialized = 1;

    printk("File system initialized (MINIX RAMFS)\n");
    printk("Inodes: %d, Zones: %d\n", super_block.s_ninodes, super_block.s_nzones);
}

static int find_free_inode(void) {
    for (int i = 1; i < super_block.s_ninodes; i++) {
        if (inodes[i].i_mode == 0) return i;
    }
    return -1;
}

static int find_file_by_name(const char *name) {
    for (int i = 0; i < MINIX_MAX_FILES; i++) {
        if (files[i].in_use) {
            int match = 1;
            for (int j = 0; j < MINIX_MAX_FILENAME; j++) {
                if (files[i].name[j] != name[j]) {
                    match = 0;
                    break;
                }
                if (name[j] == '\0' && files[i].name[j] == '\0') break;
                if (files[i].name[j] == '\0' || name[j] == '\0') {
                    match = 0;
                    break;
                }
            }
            if (match) return i;
        }
    }
    return -1;
}

static int minix_alloc_zone(void) {
    for (int i = 0; i < super_block.s_nzones; i++) {
        if (!(zone_bitmap[i / 8] & (1 << (i % 8)))) {
            zone_bitmap[i / 8] |= (1 << (i % 8));
            return i;
        }
    }
    return -1;
}

int create_file(const char *filename) {
    if (!fs_initialized || !filename) return -1;

    if (find_file_by_name(filename) >= 0) return -2;

    int idx = find_free_inode();
    if (idx < 0) return -1;

    int slot = -1;
    for (int i = 0; i < MINIX_MAX_FILES; i++) {
        if (!files[i].in_use) {
            slot = i;
            break;
        }
    }
    if (slot == -1) return -1;

    for (int i = 0; i < MINIX_MAX_FILENAME - 1 && filename[i]; i++) {
        files[slot].name[i] = filename[i];
    }
    files[slot].name[MINIX_MAX_FILENAME - 1] = '\0';
    files[slot].inode_num = idx;
    files[slot].size = 0;
    files[slot].in_use = 1;
    files[slot].data = (char *)&ram_disk[slot * MINIX_BLOCK_SIZE];

    inodes[idx].i_mode = 0x81A4;
    inodes[idx].i_uid = 0;
    inodes[idx].i_size = 0;
    inodes[idx].i_mtime = jiffies;
    inodes[idx].i_gid = 0;
    inodes[idx].i_nlinks = 1;
    for (int i = 0; i < MINIX_NUM_ZONE_PTRS; i++) {
        inodes[idx].i_zone[i] = 0;
    }

    int zone = minix_alloc_zone();
    if (zone >= 0) {
        inodes[idx].i_zone[0] = (u16)zone;
    }

    return idx;
}

int delete_file(const char *filename) {
    if (!fs_initialized || !filename) return -1;

    int slot = find_file_by_name(filename);
    if (slot < 0) return -1;

    int idx = files[slot].inode_num;

    for (int i = 0; i < MINIX_NUM_ZONE_PTRS && inodes[idx].i_zone[i]; i++) {
        int zone = inodes[idx].i_zone[i];
        zone_bitmap[zone / 8] &= ~(1 << (zone % 8));
    }

    inodes[idx].i_mode = 0;
    files[slot].in_use = 0;
    files[slot].name[0] = '\0';
    files[slot].size = 0;

    return 0;
}

int read_file(const char *filename, void *buf, uint32_t size) {
    if (!fs_initialized || !filename || !buf) return -1;

    int slot = find_file_by_name(filename);
    if (slot < 0) return -1;

    if (!files[slot].data || files[slot].size == 0) return -1;

    u32 to_read = size < files[slot].size ? size : files[slot].size;
    for (u32 i = 0; i < to_read; i++) {
        ((u8 *)buf)[i] = ((u8 *)files[slot].data)[i];
    }

    return (int)to_read;
}

int write_file(const char *filename, void *buf, uint32_t size) {
    if (!fs_initialized || !filename || !buf) return -1;

    int slot = find_file_by_name(filename);
    if (slot < 0) {
        int ret = create_file(filename);
        if (ret < 0) return ret;
        slot = find_file_by_name(filename);
    }

    if (size > MINIX_BLOCK_SIZE) size = MINIX_BLOCK_SIZE;

    for (u32 i = 0; i < size; i++) {
        ((u8 *)files[slot].data)[i] = ((u8 *)buf)[i];
    }

    files[slot].size = size;
    int idx = files[slot].inode_num;
    inodes[idx].i_size = size;
    inodes[idx].i_mtime = jiffies;

    return (int)size;
}

void list_files(void) {
    printk("Files in root directory:\n");
    printk("----------------------\n");

    int count = 0;
    for (int i = 0; i < MINIX_MAX_FILES; i++) {
        if (files[i].in_use) {
            printk("  %-20s %d bytes (inode %d)\n",
                   files[i].name, files[i].size, files[i].inode_num);
            count++;
        }
    }

    if (count == 0) {
        printk("  (empty directory)\n");
    }
    printk("----------------------\n");
    printk("  %d file(s)\n", count);
}
