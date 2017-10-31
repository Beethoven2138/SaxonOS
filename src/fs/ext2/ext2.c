#include <fs/ext2/ext2.h>

struct inode root_inode;

static struct ext2_extended_superblock ext2_sb;

static uint32_t block_size;
static uint32_t fragment_size;
static uint32_t inode_count;
static uint32_t block_count;
static uint32_t blocks_per_group;
static uint32_t inodes_per_group;
static uint32_t block_group_count;
static uint32_t bgd_addr;
static uint32_t inode_size;
static uint32_t inodes_per_block;
static uint8_t  dir_has_typefield;

//static struct file_system_type *ext2_fs_type;
static device_t *ext2_dev;

//static ext2_buffer_t buffer[MAX_BUFFERS];
//static size_t buffer_index;

static struct super_block *super_block;

struct file_operations ext2_file_ops = {0, &ext2_read, 0, 0};
extern struct inode_operations ext2_inode_ops = {&ext2_file_ops,0,&ext2_lookup,0,0,0,0,0,
					  0,0,0,0,0,0,0,0};
struct super_operations ext2_super_operations = {ext2_read_inode, 0,0,0,0,0};

void ext2_parse_superblock(ext2_extended_superblock_t *dest, device_t dev)
{
	//ata_read_sects_28_pio(dest, 2, find_bdev(dev), SUPERBLOCK_ADDR);

	struct buffer_head *bh;
	bh = get_block(SUPERBLOCK_ADDR, dev);
	memcpy(dest, bh->buff, BLOCK_SIZE);
	put_block(bh);
	bh = get_block(SUPERBLOCK_ADDR + 1, dev);
	memcpy(((char*)dest) + BLOCK_SIZE, bh->buff, BLOCK_SIZE);
	put_block(bh);

	if (dest->base_sb.ext2_signature != 0xEF53)
		kpanic("not ext2");

	block_size = 0x400 << dest->base_sb.block_size;

	fragment_size = 0x400 << dest->base_sb.fragment_size;
	inode_count = dest->base_sb.inode_count;
	block_count = dest->base_sb.block_count;
	blocks_per_group = dest->base_sb.blocks_per_group;
	inodes_per_group = dest->base_sb.inodes_per_group;

        block_group_count = CEILING_DIV(block_count, blocks_per_group);

	bgd_addr = (block_size == 0x400) ? 2 : 1;

	if (dest->base_sb.major_version >= 1)
	        inode_size = dest->inode_size;
	else
		inode_size = 128;

	inodes_per_block = block_size / inode_size;

	dir_has_typefield = (dest->required_features & DIR_ENTRY_HAS_TYPEFIELD) ? true : false;

	read_bgd_table(dev);
}

static struct buffer_head *read_inode(ext2_inode_t *dest, uint32_t inode, device_t dev)
{
	const uint32_t inode_block_group = BG_OF_INODE(inode);
	terminal_writestring("Reading inode");

	const uint32_t index = INODE_INDEX(inode);
	const uint32_t block_of_inode = bgdt_entries[inode_block_group].inode_table + BLOCK_OF_INODE(index);

	struct buffer_head *bh;
	terminal_printhex32(ADDR_OF_BLOCK(block_of_inode));

	terminal_writestring("\n\n\n");
	//bh = get_block(ADDR_OF_BLOCK(block_of_inode), dev);
	bh = get_block(ADDR_OF_BLOCK(block_of_inode) + ((inode_size * (index % inodes_per_block)) / BLOCK_SIZE), dev);
	if (dest)
	{
		for (uint8_t i = 0; i < sizeof(ext2_inode_t); i++)
			*(((uint8_t*)dest) + i) = bh->buff[i + ((inode_size
								 * (index % inodes_per_block))
								% BLOCK_SIZE)];
	}
	//put_block(bh);
	return bh;
}


void read_bgd_table(device_t dev)
{
	struct buffer_head *bh;
	bh = get_block(ADDR_OF_BLOCK(bgd_addr), dev);
        bgdt_entries = (ext2_bgd_t*)bh->buff;
}


/*void ext2_list_directory(ext2_inode_t *dir, device_t *dev)
{
	terminal_writestring("in list_directory\n");

	char tmp_buff[512];
	ext2_directory_entry_t *dir_entry;
	
	for (uint8_t i = 0; i < 12; i++)
	{
		if (dir->direct_block[i] == 0)
			break;
		ata_read_sects_28_pio(tmp_buff, 1, dev, ADDR_OF_BLOCK(dir->direct_block[i]));
		dir_entry = (ext2_directory_entry_t*)tmp_buff;

		while (((uint8_t*)dir_entry) + dir_entry->entry_size < tmp_buff + 512)
		{
			if (dir_entry->inode != SKIP_DIR)
			{
				terminal_writestring("Directory entry name: ");
				uint16_t name_len = (dir_has_typefield) ?
					dir_entry->name_length : dir_entry->name_length |
					((uint16_t)dir_entry->type) << 8;

				uint8_t *c = ((uint8_t*)dir_entry) + 8;
				for (uint16_t j = 0; j < name_len; j++, c++)
				{
					terminal_putchar(*c);
				}
				terminal_putchar('\n');
				terminal_writestring("Directory entry inode number: ");
				terminal_printhex32(dir_entry->inode);
				terminal_putchar('\n');
				if (dir_has_typefield)
				{
					terminal_writestring("directory entry type: ");
					terminal_printhex(dir_entry->type);
					terminal_putchar('\n');
				}
			}
			terminal_writestring("size");
			terminal_printhex32(dir_entry->entry_size);

			dir_entry = dir_entry->entry_size + ((uint8_t*)dir_entry);
		}
	}
	}*/


void read_file(ext2_inode_t *file, device_t *dev)
{
	terminal_writestring("In read_file\n");
	if (file->type_permissions & 0xF000 != TYPE_REG_FILE)
		kpanic("not a file");

	char tmp_buff[512];

	for (uint8_t i = 0; i < 12; i++)
	{
		terminal_writestring("direct block: ");
		terminal_printhex32(file->direct_block[i]);
		if (i * 512 >= file->size_low || file->direct_block[i] == 0 || file->direct_block[i] >= block_count)
			break;
		ata_read_sects_28_pio(tmp_buff, 1, dev, ADDR_OF_BLOCK(file->direct_block[i]));

		terminal_writestring(tmp_buff);
	}
}

size_t write_file(void *ptr, size_t size, size_t nmemb, ext2_inode_t *file, uint32_t inode, device_t *dev)
{
	terminal_writestring("In write file\n");
	if (file->type_permissions & 0xF000 != TYPE_REG_FILE)
		kpanic("not a file");

	
	char tmp_buff[512];

	size_t i = 0;
	while (i < nmemb * size)
	{
		memset(tmp_buff, 0, 512);
		const uint32_t count = (nmemb * size - i);
		memcpy(tmp_buff, (uint8_t*)ptr + i, (count >= 512) ? 512 : count);
		ata_write_sects_28_pio(tmp_buff, 1, dev, ADDR_OF_BLOCK(file->direct_block[i / 512]));		
		if (i + 512 >= 0x1800 || file->direct_block[i / 512] >= block_count)
		{
			//change_file_size(inode, dev, i);
			return i / size;
		}
		i += 512;
	}
	//change_file_size(inode, dev, nmemb * size);
	return nmemb;
}

static void change_file_size(uint32_t inode, int32_t size, device_t dev)
{
	const uint32_t inode_block_group = BG_OF_INODE(inode);

	const uint32_t index = INODE_INDEX(inode);
	const uint32_t block_of_inode = bgdt_entries[inode_block_group].inode_table + BLOCK_OF_INODE(index);

	struct buffer_head *bh;
	bh = get_block(REAL_INODE_BLOCK(inode), dev);

	ext2_inode_t *tmp_inode = bh->buff + INODE_OFFSET(inode);
	bh->dirty = true;
	tmp_inode->size_low = size;

	put_block(bh);
}


uint32_t alloc_inode(device_t *dev)
{
	uint32_t free_inode;
	size_t i;
	for (i = 0; i < block_group_count && free_inode == 0; i++)
	{
		if (/*descriptor->free_block_count == 0 || */bgdt_entries[i].free_inode_count == 0)
		{
			continue;
		}

		char tmp_buff[512];

		//uint32_t free_block;


		/*for (free_block = 0; free_block < blocks_per_group; free_block++)
		  {
		  if ((free_block * sizeof(bgd_t)) % 512 == 0)
		  ata_read_sects_28_pio(tmp_buff, 1, dev,
		  descriptor->block_usage_bitmap +
		  (free_block * sizeof(bgd_t)) / 512);
		  if (tmp_buff[free_block/512] >> (free_block % 512) == 0)
		  {
		  tmp_buff[free_block/512] |= 1 << free_block % 512;
		  ata_write_sects_28_pio(tmp_buff, 1, dev, descriptor->block_usage_bitmap +
		  (free_block * sizeof(bgd_t)) / 512);
		  }
		  }*/

		for (free_inode = 0; free_inode < inodes_per_group; free_inode++)
		{
			if ((free_inode * sizeof(ext2_bgd_t)) % 512 == 0)
				ata_read_sects_28_pio(tmp_buff, 1, dev,
						      bgdt_entries[i].inode_usage_bitmap +
						      (free_inode * sizeof(ext2_bgd_t)) / 512);
			if (tmp_buff[free_inode/512] >> (free_inode % 512) == 0)
			{
				tmp_buff[free_inode/512] |= 1 << free_inode % 512;
				ata_write_sects_28_pio(tmp_buff, 1, dev, bgdt_entries[i].inode_usage_bitmap +
						       (free_inode * sizeof(ext2_bgd_t)) / 512);
			}
		}
	}

	//free_block += blocks_per_group * block_group;
	free_inode += inodes_per_group * (i + 1);

	//inode_t *inode = (inode_t*)kmalloc(sizeof(inode_t));
	return free_inode;
}

ssize_t ext2_read(struct file *filep, char *buff, size_t count, size_t *off)
{
	terminal_writestring("in ext2_read");

	struct inode *inode = filep->f_inode;

	const device_t dev = inode->dev;

	if (strcmp(inode->i_sb->fs_type->name, "ext2") != 0)
		kpanic("ext2_read. wrong filesystem");

	if (*off + count >= inode->size)
		return -1;

	if (inode->flags & 0xF000 != TYPE_REG_FILE)
		return -1;

	ext2_inode_t *tmp_inode = (ext2_inode_t*)kmalloc(sizeof(ext2_inode_t));
	put_block(read_inode(tmp_inode, inode->i_no, dev));

	for(size_t i = 0; i < sizeof(ext2_inode_t); i++)
		terminal_printhex(*((uint8_t*)tmp_inode + i));

	if (!(tmp_inode->type_permissions & TYPE_REG_FILE))
        {
		/*terminal_putchar('\n');
		for(int i = 0; i < sizeof(ext2_inode_t); i++)
			terminal_printhex(*(((uint8_t*)tmp_inode) + i));
		kpanic("in ext2_read wrong");
		for(;;);*/
		return -1;
	}

	struct buffer_head *bh;
	uint32_t offset = *off % BLOCK_SIZE;
	size_t chars;
	size_t read = 0;
	size_t i = *off / BLOCK_SIZE;
	register char *ptr;

	while (i < 12 && tmp_inode->direct_block[i] != 0)
	{
		for (size_t k = 0; k < block_size / BLOCK_SIZE && count > 0; k++)
		{
			if (!(bh = get_block(ADDR_OF_BLOCK(tmp_inode->direct_block[i]) + k, dev)))
				return -1;
			chars = (count < BLOCK_SIZE) ? count : BLOCK_SIZE;
			ptr = offset + bh->buff;
			offset = 0;
			i++;
			*off += chars;
			read += chars;
			count -= chars;
			while (chars-- > 0)
			{
				terminal_putchar(*ptr);
				*(buff++) = *(ptr++);
			}
			put_block(bh);
		}
	}

	kfree(tmp_inode);

	return read;
}

/*this overrites the file instead of appending to it*/
ssize_t ext2_write(struct file *filep, const char *buff, size_t count, size_t *off)
{
	terminal_writestring("in ext2_write");

	struct inode *inode = filep->f_inode;

	const device_t dev = inode->dev;

	if (strcmp(inode->i_sb->fs_type->name, "ext2") != 0)
		kpanic("ext2_write. wrong filesystem");

	if (*off + count >= inode->size)
		return -1;

	if (inode->flags & 0xF000 != TYPE_REG_FILE)
		return -1;

	ext2_inode_t *tmp_inode = (ext2_inode_t*)kmalloc(sizeof(ext2_inode_t));

	struct buffer_head *inode_bh;
        inode_bh = read_inode(tmp_inode, inode->i_no, dev);

	if (!(tmp_inode->type_permissions & TYPE_REG_FILE))
		return -1;


	struct buffer_head *bh/*[block_size / BLOCK_SIZE]*/;
	uint32_t offset = *off % BLOCK_SIZE;
	size_t chars;
	size_t wrote = 0;
	size_t i = *off / BLOCK_SIZE;
	register char *ptr;

	while (tmp_inode->direct_block[i] != 0)
	{
		for (size_t k = 0; k < block_size / BLOCK_SIZE && count > 0; k++)
		{
			if (!(bh = get_block(ADDR_OF_BLOCK(tmp_inode->direct_block[i]) + k, dev)))
				goto exit;
			chars = (count < BLOCK_SIZE) ? count : BLOCK_SIZE;
			ptr = offset + bh->buff;
			offset = 0;
			i++;
			*off += chars;
			wrote += chars;
			count -= chars;

			while (chars-- > 0)
			{
				terminal_putchar(*ptr);
				*(ptr++) = *(buff++);
			}
			bh->dirty = true;
			put_block(bh);
		}
	}
exit:
	tmp_inode->size_low = wrote;
	inode_bh->dirty = true;
	put_block(inode_bh);
	kfree(tmp_inode);

	return wrote;
}

int ext2_lookup(struct inode *dir, const char *filename, int len, struct inode **result_inode)
{
	terminal_writestring("in ext2_lookup");
	size_t size = 0;
        ext2_directory_entry_t *de;
	struct buffer_head *bh[block_size/BLOCK_SIZE];
	ext2_inode_t inode;
	put_block(read_inode(&inode, dir->i_no, dir->dev));
	if (!(&inode))
		return -1;
	if (!len)
		return -1;
	if (!(inode.type_permissions && TYPE_DIRECTORY))
		return -1;
        for (uint8_t i = 0; i < 12; i++)
	{
		if (!inode.direct_block[i])
			return -1;

		for (uint8_t k = 0; k < block_size / BLOCK_SIZE; k++)
		{
			if (!(bh[k] = get_block(ADDR_OF_BLOCK(inode.direct_block[i]) + k, dir->dev)))
				return -1;

			de = (ext2_directory_entry_t*)bh[k]->buff;
			while (((uint32_t)de) + de->entry_size < (uint32_t)bh[k]->buff + BLOCK_SIZE)
			{
				if (de->inode != SKIP_DIR && de->name_length == len)
				{
					if (!strncmp(filename, (void*)(de + 1), len))
					{
						put_block(read_inode(&inode, de->inode, dir->dev));
						if (!(&inode))
							return NULL;
						(*result_inode)->i_no = de->inode;
						(*result_inode)->size = inode.size_low;
						(*result_inode)->permissions = inode.type_permissions;
						put_block(bh[k]);
						return 0;
					}
				}
				if ((size += de->entry_size) > inode.size_low)
					return -1;
				de = (ext2_directory_entry_t*)((uint8_t*)de + de->entry_size);
			
			}
			put_block(bh[k]);
		}
	}
	return -1;
}

void ext2_read_inode(struct inode *dest)
{
	terminal_writestring("in read inode");
	dest->i_ops = &ext2_inode_ops;
	dest->i_sb = &super_block;
	if (dest->i_no != 0)
	{
		ext2_inode_t *tmp_inode = (ext2_inode_t*)kmalloc(sizeof(ext2_inode_t));
		dest->size = tmp_inode->size_low;
		dest->gid = tmp_inode->gid;
		dest->uid = tmp_inode->uid;
		kfree(tmp_inode);
	}
}

struct super_block *ext2_read_super(struct super_block *sb, device_t dev)
{
	ext2_parse_superblock(&ext2_sb, dev);
	ext2_inode_t *tmp = (ext2_inode_t*)kmalloc(sizeof(ext2_inode_t));
	read_inode(tmp, 2, dev);
	struct inode *root = (struct inode*)kmalloc(sizeof(struct inode));
	root->i_no = 2;
	root->size = tmp->size_low;
	root->gid = tmp->gid;
	root->uid = tmp->uid;
	root->flags = tmp->flags;
	sb->root = root;
	sb->sb_ops = &ext2_super_operations;
	/*if (root_inode.i_no == 0)
	{
	        
		sb->mount_point = root;
	}
	else
		kpanic("in ext2_read_super. Unsuported");*/
	if (sb->fs_type == 0)
	{
		sb->fs_type = *find_file_system("ext2", 4);
	}
	//ext2_fs_type = sb->fs_type;
	sb->dev = dev;
	//ext2_dev = dev;
	super_block = sb;
	kfree(tmp);
	return sb;
}
