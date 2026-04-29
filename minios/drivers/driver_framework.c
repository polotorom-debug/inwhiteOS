#include "minios.h"

#define MAX_DRIVERS 16

typedef struct {
    const char *name;
    int initialized;
    int (*init)(void);
    int (*read)(void *buf, int size);
    int (*write)(const void *buf, int size);
    int (*ioctl)(int cmd, void *arg);
} driver_t;

static driver_t drivers[MAX_DRIVERS];
static int num_drivers = 0;

int register_driver(const char *name,
                    int (*init_fn)(void),
                    int (*read_fn)(void *, int),
                    int (*write_fn)(const void *, int),
                    int (*ioctl_fn)(int, void *)) {
    if (num_drivers >= MAX_DRIVERS) return -1;

    driver_t *drv = &drivers[num_drivers];
    drv->name = name;
    drv->initialized = 0;
    drv->init = init_fn;
    drv->read = read_fn;
    drv->write = write_fn;
    drv->ioctl = ioctl_fn;

    num_drivers++;
    return num_drivers - 1;
}

int init_driver(int id) {
    if (id < 0 || id >= num_drivers) return -1;
    if (drivers[id].init) {
        int ret = drivers[id].init();
        if (ret == 0) {
            drivers[id].initialized = 1;
            printk("Driver '%s' initialized\n", drivers[id].name);
        }
        return ret;
    }
    return -1;
}

int driver_read(int id, void *buf, int size) {
    if (id < 0 || id >= num_drivers || !drivers[id].initialized) return -1;
    if (drivers[id].read) return drivers[id].read(buf, size);
    return -1;
}

int driver_write(int id, const void *buf, int size) {
    if (id < 0 || id >= num_drivers || !drivers[id].initialized) return -1;
    if (drivers[id].write) return drivers[id].write(buf, size);
    return -1;
}

void list_drivers(void) {
    printk("Registered drivers (%d):\n", num_drivers);
    printk("----------------------\n");
    for (int i = 0; i < num_drivers; i++) {
        printk("  [%d] %s %s\n", i, drivers[i].name,
               drivers[i].initialized ? "(active)" : "(inactive)");
    }
}
