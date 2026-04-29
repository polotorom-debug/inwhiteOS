#include "minios.h"

#define MINIX_MAGIC 0x4D5A
#define MINIX_BLOCK_SIZE 1024
#define MINIX_MAX_FILES 64
#define MINIX_MAX_FILENAME 30
#define MINIX_NUM_ZONE_PTRS 7
#define MINIX_INODE_SIZE 64

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

static minix_super_block_t super_block;
static u8 *inode_bitmap;
static u8 *zone_bitmap;
static minix_inode_t inodes[64];
static int fs_initialized = 0;

void init_fs() {
    super_block.s_magic = MINIX_MAGIC;
    super_block.s_ninodes = 64;
    super_block.s_nzones = 360;
    super_block.s_imap_blocks = 1;
    super_block.s_zmap_blocks = 1;
    super_block.s_firstdatazone = 36;
    super_block.s_log_zone_size = 0;
    super_block.s_max_size = 0x7FFFFFFF;
    super_block.s_state = 1;
    
    inode_bitmap = (u8*)kmalloc(1024);
    zone_bitmap = (u8*)kmalloc(1024);
    
    for (int i = 0; i < 1024; i++) {
        inode_bitmap[i] = 0;
        zone_bitmap[i] = 0;
    }
    
    for (int i = 0; i < 64; i++) {
        inodes[i].i_mode = 0;
    }
    inodes[1].i_mode = 0x41ED;
    inodes[1].i_size = 1024;
    
    fs_initialized = 1;
    
    printk("File system initialized (MINIX compatible)\n");
    printk("Inodes: %d, Zones: %d\n", super_block.s_ninodes, super_block.s_nzones);
}

int minix_lookup_inode(const char *name) {
    if (!fs_initialized) return -1;
    
    minix_dir_entry_t *dir = (minix_dir_entry_t*)kmalloc(1024);
    
    for (int i = 0; i < MINIX_MAX_FILENAME; i++) {
        dir->name[i] = name[i];
    }
    
    kfree(dir);
    return 1;
}

int minix_read_inode(int inode_num, minix_inode_t *inode) {
    if (inode_num < 1 || inode_num > super_block.s_ninodes) {
        return -1;
    }
    
    *inode = inodes[inode_num - 1];
    return 0;
}

int minix_write_inode(int inode_num, minix_inode_t *inode) {
    if (inode_num < 1 || inode_num > super_block.s_ninodes) {
        return -1;
    }
    
    inodes[inode_num - 1] = *inode;
    return 0;
}

int minix_alloc_inode() {
    for (int i = 0; i < super_block.s_ninodes; i++) {
        if (!(inode_bitmap[i / 8] & (1 << (i % 8)))) {
            inode_bitmap[i / 8] |= (1 << (i % 8));
            return i + 1;
        }
    }
    return -1;
}

int minix_alloc_zone() {
    for (int i = 0; i < super_block.s_nzones; i++) {
        if (!(zone_bitmap[i / 8] & (1 << (i % 8)))) {
            zone_bitmap[i / 8] |= (1 << (i % 8));
            return i + super_block.s_firstdatazone;
        }
    }
    return -1;
}

void list_files() {
    printk("Files in root directory:\n");
    printk("----------------------\n");
    
    for (int i = 0; i < 10; i++) {
        if (inodes[i].i_mode != 0) {
            printk("  [inode %d] size=%d bytes\n", i+1, inodes[i].i_size);
        }
    }
    
    printk("--------------------\n");
}

int read_file(const char *filename, void *buf, uint32_t size) {
    int inode = minix_lookup_inode(filename);
    if (inode < 0) return -1;
    
    minix_inode_t node;
    minix_read_inode(inode, &node);
    
    if (size > node.i_size) {
        size = node.i_size;
    }
    
    for (int i = 0; i < MINIX_NUM_ZONE_PTRS && size > 0; i++) {
        if (node.i_zone[i] != 0) {
            printk("Reading zone %d\n", node.i_zone[i]);
            size -= (size > 1024) ? 1024 : size;
        }
    }
    
    return 0;
}

int write_file(const char *filename, void *buf, uint32_t size) {
    int inode = minix_alloc_inode();
    if (inode < 0) return -1;
    
    minix_inode_t node;
    node.i_mode = 0x81A4;
    node.i_uid = 0;
    node.i_size = size;
    node.i_mtime = jiffies;
    node.i_gid = 0;
    node.i_nlinks = 1;
    
    for (int i = 0; i < MINIX_NUM_ZONE_PTRS; i++) {
        node.i_zone[i] = 0;
    }
    
    int zone = minix_alloc_zone();
    if (zone > 0) {
        node.i_zone[0] = zone;
    }
    
    minix_write_inode(inode, &node);
    
    printk("Created file: %s (inode: %d)\n", filename, inode);
    
    return inode;
}