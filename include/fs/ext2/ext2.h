#ifndef __FS__EXT2__EXT2_H
#define __FS__EXT2__EXT2_H

#include <common.h>
#include <drivers/ata.h>
#include <mm/heap.h>
#include <fs/vfs/vfs.h>
#include <fs/devfs/devfs.h>

/*The superblock is always located at byte 1024
  from the beginning of the volume and is 1024
  bytes long. Thus, the LBA of the superblock
  would be 2 to 3*/
#define SUPERBLOCK_ADDR 2

/* for fs_state*/
#define FS_CLEAN 1
#define FS_HAS_ERRORS 2

/* for error_handle_info*/
#define IGNORE_ERROR 1
#define REMOUNT_FS_RO 2
#define KPANIC 3

/* for os_id*/
#define LINUX 0
#define GNU_HURD 1
#define MASIX 2
#define FREEBSD 3
#define OTHER_LITES 4

#define BG_OF_INODE(inode) ((inode - 1) / inodes_per_group)
#define INODE_INDEX(inode) ((inode - 1) % inodes_per_group)
#define BLOCK_OF_INODE(index) ((index * inode_size) / block_size)
#define INODE_BLOCK_OFF(index) (inode_size * ((index) % inodes_per_block))

//#define ADDR_OF_BLOCK(block) ((block * block_size) / 512)
#define ADDR_OF_BLOCK(block) ((block_size / BLOCK_SIZE) * block)

#define REAL_INODE_BLOCK(inode) (ADDR_OF_BLOCK(bgdt_entries[BG_OF_INODE(inode)].inode_table \
					       + BLOCK_OF_INODE(INODE_INDEX(inode))) \
				 + (inode_size * (INODE_INDEX(inode) % inodes_per_block) / BLOCK_SIZE))

#define INODE_OFFSET(inode) (inode_size * (INODE_INDEX(inode) % inodes_per_block) % BLOCK_SIZE)

#define TYPE_FIFO 0x1000
#define TYPE_CHAR_DEV 0x2000
#define TYPE_DIRECTORY 0x4000
#define TYPE_BLOCK_DEV 0x6000
#define TYPE_REG_FILE 0x8000
#define TYPE_SYMBOL_LINK 0xA000
#define TYPE_UNIX_SOCKET 0xC000

#define PERMISSION_O_EXEC 00001
#define PERMISSION_O_WRITE 00002
#define PERMISSION_O_READ 00004
#define PERMISSION_G_EXEC 00010
#define PERMISSION_G_WRITE 00020
#define PERMISSION_G_READ 00040
#define PERMISSION_U_EXEC 00100
#define PERMISSION_U_WRITE 00200
#define PERMISSION_U_READ 00400
#define PERMISSION_STICKY_BIT 01000
#define PERMISSION_SET_GID 02000
#define PERMISSION_SET_UID 04000

#define FLAG_SYNCHRONOUS_UPDATES 0x08
#define FLAG_APPEND_ONLY 0x20
#define FLAG_NOT_DUMP 0x40
#define FLAG_LAT_NOT_UPDATE 0x80
#define FLAG_HASH_INDEXED_DIR 0x10000
#define FLAG_AFS_DIR 0x20000
#define FLAG_JOURNAL_FILE_DATA 0x40000

/* inode 2*/
#define ROOT_DIRECTORY 2

#define DIR_TYPE_UNKNOWN 0
#define DIR_TYPE_REGULAR 1
#define DIR_TYPE_DIRECTORY 2
#define DIR_TYPE_CHAR_DEV 3
#define DIR_TYPE_BLOCK_DEV 4
#define DIR_TYPE_FIFO 5
#define DIR_TYPE_SOCKET 6
#define DIR_TYPE_SOFT_LINK 7

#define DIR_ENTRY_HAS_TYPEFIELD 2
#define SKIP_DIR 0

#define MAX_BUFFERS 100

/*extern uint32_t block_size;
extern uint32_t fragment_size;
extern uint32_t inode_count;
extern uint32_t block_count;
extern uint32_t blocks_per_group;
extern uint32_t inodes_per_group;
extern uint32_t block_group_count;
extern uint32_t bgd_addr;
extern uint32_t inode_size;
extern uint32_t inodes_per_block;
extern uint8_t  dir_has_typefield;*/

typedef struct __attribute__((packed))ext2_superblock
{
	uint32_t inode_count;
	uint32_t block_count;
	uint32_t res_block_count;
	uint32_t free_block_count;
	uint32_t free_inode_count;
	uint32_t superblock_num;
	uint32_t block_size;
	uint32_t fragment_size;
	uint32_t blocks_per_group;
	uint32_t fragments_per_group;
	uint32_t inodes_per_group;
	ptime_t last_mount_time;
        ptime_t last_written_time;
	uint16_t mount_count;
	uint16_t allowed_mount_count;
	uint16_t ext2_signature;
	uint16_t fs_state;
	uint16_t error_handle_info;
	uint16_t minor_version;
	ptime_t last_consistency_check;
	uint32_t check_interval;
	uint32_t os_id;
	uint32_t major_version;
	uint16_t res_uid;
	uint16_t res_gid;
} ext2_superblock_t;

typedef struct __attribute__((packed))ext2_extended_superblock
{
	ext2_superblock_t base_sb;
	uint32_t first_inode;
	uint16_t inode_size;
	uint16_t partof_block_group;
	uint32_t optional_features;
	uint32_t required_features;
	uint32_t features_RO;
	uint8_t fs_id[16];
	uint8_t  vol_name[16];
	uint8_t  last_mounted_vol[64];
	uint32_t compression_algorithms;
	uint8_t  file_prealloc_blocks;
	uint8_t  directory_prealloc_blocks;
	uint16_t unused;
	uint8_t  journal_id[16];
	uint32_t journal_inode;
	uint32_t journal_device;
	uint32_t orphan_inode_head;
	uint8_t extra[788];
} ext2_extended_superblock_t;

//extern extended_superblock_t superblock;

typedef struct __attribute__((packed))ext2_bgd
{
	uint32_t block_usage_bitmap;
	uint32_t inode_usage_bitmap;
	uint32_t inode_table;
	uint16_t free_block_count;
	uint16_t free_inode_count;
	uint16_t directory_count;
	uint8_t  unused[14];
} ext2_bgd_t;

ext2_bgd_t *bgdt_entries;

/* 
   UNLIKE BLOCK ADDRESSES
   INODE ADDRESSES START AT 1!
*/
typedef struct __attribute__((packed)) ext2_inode
{
	uint16_t type_permissions;
	uint16_t uid;
	int32_t size_low;
	ptime_t last_access;
	ptime_t creation;
	ptime_t last_mod;
	ptime_t deletion;
	uint16_t gid;
	uint16_t hard_links_count;
	uint32_t used_sectors_count;
	uint32_t flags;
	uint32_t reserved0;
	uint32_t direct_block[12];
	uint32_t singly_indirect_block;
	uint32_t doubly_indirect_block;
	uint32_t triply_indirect_block;
	uint32_t gen_number;
	uint32_t reserved1;
	uint32_t reserved2;
	uint32_t fragment_blockaddr;
	//uint32_t fragment_number;
	uint32_t os_spec[3];
} ext2_inode_t;

typedef struct __attribute__((packed)) ext2_directory_entry
{
	uint32_t inode;
	uint16_t entry_size;
	uint8_t  name_length;
	uint8_t  type;
} ext2_directory_entry_t;

void ext2_parse_superblock(ext2_extended_superblock_t *dest, device_t dev);
static struct buffer_head *read_inode(ext2_inode_t *dest, uint32_t inode, device_t dev);
static void read_bgd_table(device_t dev);
//gbdt_t get_bgd(uint32_t block_group)

int ext2_lookup(struct inode *dir, const char *filename, int len, struct inode **result_inode);

void ext2_list_directory(ext2_inode_t *dir, device_t *dev);

void read_file(ext2_inode_t *file, device_t *dev);
size_t write_file(void *ptr, size_t size, size_t nmemb, ext2_inode_t *file, uint32_t inode, device_t *dev);
static void change_file_size(uint32_t inode, int32_t size, device_t dev);
//static ext2_buffer_t *find_buffer(uint32_t block_num);

void ext2_read_inode(struct inode *dest);

struct super_block *ext2_read_super(struct super_block *sb, device_t dev);

ssize_t ext2_read(struct file *filep, char *buff, size_t count, size_t *off);
ssize_t ext2_write(struct file *filep, const char *buff, size_t count, size_t *off);
#endif
