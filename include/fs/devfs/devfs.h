#ifndef __FS__DEVFS__DEVFS_H
#define __FS__DEVFS__DEVFS_H

#include <fs/devfs/buffer.h>
#include <fs/vfs/vfs.h>

/*
 * 0 - unused (nodev)
 * 1 - /dev/mem
 * 2 - /dev/fd
 * 3 - /dev/hd
 * 4 - /dev/ttyx
 * 5 - /dev/tty
 * 6 - /dev/lp
 * 7 - unnamed pipes
 */

#define MAJOR_NUMBER(flags) ((uint8_t)(flags >> 8))
#define MINOR_NUMBER(flags) ((uint8_t)(flags & 0xFF))

#define BLOCK_DEV 3

typedef struct bdev
{
	uint8_t minor;
        uint32_t start_lba;
	uint32_t part_size;
	uint8_t slave_bit;
	uint16_t base;
	uint16_t dev_ctl;
        uint32_t permissions;
	struct bdev *next;
	struct bdev *prev;
} bdev_t;

struct super_block *devfs_read_super(struct super_block *sb);

bdev_t *find_bdev(device_t dev);

void devfs_read_inode(struct inode *dest);

int bdev_read(device_t dev, char *dest, int count, size_t *pos);
int bdev_write(device_t dev, const char *src, int count, size_t *pos);
int devfs_open(struct inode *inode, struct file *filp);
int devfs_release(struct inode *inode, struct file *filp);

device_t add_bdev(bdev_t *dev);

#endif
