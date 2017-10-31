#ifndef __FS__VFS__VFS_H
#define __FS__VFS__VFS_H

#include <common.h>
#include <mm/heap.h>

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

typedef uint32_t gid_t;
typedef uint32_t uid_t;

struct inode;
struct file;
struct super_block;

struct file_operations
{
	ssize_t (*write) (struct file *, const char *, size_t, size_t *);
	ssize_t (*read) (struct file *, char *, size_t, size_t *);
	int (*open) (struct inode *, struct file *);
	int (*release) (struct inode *, struct file *);
};

struct inode_operations
{
	struct file_operations *default_file_ops;
	int (*create) (struct inode *dir, const char *filename, int len,
		       int mode, struct inode **result_inode);
	int (*lookup) (struct inode *dir, const char *filename, int len,
		       struct inode **result_inode);
	int (*link) (struct inode *oldinode, struct inode *dir,
		     const char *filename, int len);
	int (*unlink) (struct inode *dir, const char *filename, int len);
	int (*symlink) (struct inode *dir, const char *filename, int len,
			const char *destname);
	int (*mkdir) (struct inode *dir, const char *filename, int len,
		      int mode);
	int (*rmdir) (struct inode *dir, const char *filename, int len);
	int (*mknod) (struct inode *dir, const char *filename, int len,
		      int mode, int rdev);
	int (*rename) (struct inode *olddir, const char *oldname, int oldlen,
		       struct inode *newdir, const char *newname,
		       int newlen);
	int (*readlink) (struct inode *inode1, char *buf, int size);
	int (*follow_link) (struct inode *dir, struct inode *inode1,
			    int flag, int mode, struct inode **result_inode);
	int (*bmap) (struct inode *inode1, int block);
	void (*truncate) (struct inode *inode1);
	int (*permission) (struct inode *inode1, int flag);
	int (*read_dir) (struct inode *dir, struct inode **result_inode);
};

struct inode
{
	//char *name;
	uint32_t dev;
	uint32_t i_no;/*inode number from concrete filesystem*/
	uint32_t size;
	struct inode_operations *i_ops;
	struct super_block *i_sb;
	gid_t gid;
	uid_t uid;
	uint32_t flags;
	uint32_t permissions;
	bool mount_point;
	size_t counter;/*incremented when file opened, decremented when closed*/
	struct inode *next;
	struct inode *prev;
	/*union
	{
		uint32_t *ext2_blocks;
		};*/
};
//ROOT INODE
//extern struct inode *i_head;

struct super_operations
{
	/*fill up the inode passed to it*/
	void (*read_inode) (struct inode *);

	/*write the inode to the storage media*/
	void (*write_inode) (struct inode *);

	/* permanently delete file and remove its blocks if no links*/
	void (*put_inode) (struct inode *);

	/*free the superblock*/
	void (*put_super) (struct super_block *);

	/*write superblock to the storage media*/
	void (*write_super) (struct super_block *);

	/*fill statfs structure*/
	//void (*statfs) (struct super_block *, struct statfs *);

	/*entering the new attributes for the filesystem in superblock and restoring concistency*/
	int (*remount_fs) (struct super_block *, int *, char *);
};

struct super_block
{
	device_t dev; /* device for File System */

	struct file_system_type *fs_type; /* file system type */

	struct super_operations *sb_ops; /* super block operations */

	//uint32_t flags; /* flags */

	//unsigned long s_magic; /* file system identifier */

	//ptime_t change_time; /* time of change */

	struct inode *mount_point; /* mount point */

	struct inode *root; /* root inode */

	struct super_block *next;
};

//struct super_block *sb_head;

struct file_system_type
{
	/* mount interface*/
	struct super_block *(*read_super)(struct super_block *, device_t dev);

	const char *name;

	//int requires_dev;

	struct file_system_type *next;
};

struct file
{
	struct inode *f_inode;
	char *name;
	size_t pos;
};

struct dentry
{
	char *name;
	struct inode *inode;
	struct super_block *sb;
	bool mounted;
	size_t count;
	struct dentry *child;
	struct dentry *next;
	struct dentry *prev;
};

//struct dentry d_head = {NULL, i_head, NULL, NULL};

void init_fs(struct file_system_type *root_fs, device_t dev);
int register_filesystem(struct file_system_type *fs);
int unregister_filesystem(struct file_system_type *fs);
struct file_system_type **find_file_system(const char *name, uint8_t len);
struct super_block *find_super_block(const char *name);

void add_inode(struct inode *inode);
void add_dir(struct dentry *dentry);
void add_sb(struct super_block *sb);

#endif
