#include <common.h>
#include <multiboot.h>
#include <gdt.h>
#include <idt.h>
#include <irq.h>
#include <drivers/keyboard.h>
#include <drivers/timer.h>
#include <mm/frame.h>
#include <mm/paging.h>
#include <mm/heap.h>
#include <drivers/pci.h>

#include <fs/devfs/devfs.h>
#include <drivers/ata.h>
#include <fs/ext2/ext2.h>
#include <fs/mbr.h>
#include <fs/vfs/vfs.h>
#include <task.h>
#include <userspace/syscall.h>
#include <drivers/rtc.h>
#include <tss.h>



/* Hardware text mode color constants. */
enum vga_color {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};
 
/*static inline*/ uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg)
{
	return fg | bg << 4;
}
 
static inline uint16_t vga_entry(unsigned char uc, uint8_t color)
{
	return (uint16_t) uc | (uint16_t) color << 8;
}
 
static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;
 
void terminal_initialize(void)
{
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	terminal_buffer = (uint16_t*) 0xC00B8000;
	for (size_t y = 0; y < VGA_HEIGHT; y++)
	{
		for (size_t x = 0; x < VGA_WIDTH; x++)
		{
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}
 
void terminal_setcolor(uint8_t color)
{
	terminal_color = color;
}
 
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y)
{
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}

void terminal_putchar(char c)
{
	if (c == 10)
	{
		++terminal_row;
		terminal_column = 0;
	}

	else if (c == 8)
	{
		if (terminal_column > 0)
		{
			terminal_column--;
			terminal_buffer[terminal_row * VGA_WIDTH + terminal_column] = 0;	
		}
		else if (terminal_row > 0)
		{
		        //TODO: add this
		}
	}
	
	else
	{
		terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
		if (++terminal_column == VGA_WIDTH)
		{
			terminal_column = 0;
			terminal_row++;
			/*if (++terminal_row == VGA_HEIGHT)
			  terminal_row = 0;*/
		}
	}

	if (terminal_row == VGA_HEIGHT)
	{
		uint32_t y;
		for (y = 0; y < VGA_HEIGHT - 1; y++)
		{
			for (uint32_t x = 0; x <= VGA_WIDTH; x++)
			{
				terminal_buffer[y * VGA_WIDTH + x] = terminal_buffer[(y+1) * VGA_WIDTH + x];
			}
		}
	        for (uint32_t x = 0; x <= VGA_WIDTH; x++)
			terminal_buffer[y * VGA_WIDTH + x] = 0;
		
		terminal_row--;
		terminal_column = 0;
	}
}

void terminal_printhex(char key)
{
	char value[2] = "00";
	const char *hex = "0123456789ABCDEF";

	value[0] = hex[(key & 0xF0) >> 4];
	value[1] = hex[key & 0x0F];
	terminal_putchar(value[0]);
	terminal_putchar(value[1]);
}

void terminal_printhex16(uint16_t key)
{
	terminal_printhex((key & 0xFF00) >> 8);
	terminal_printhex(key & 0x00FF);
}

void terminal_printhex32(uint32_t key)
{
	terminal_printhex16((key & 0xFFFF0000) >> 16);
	terminal_printhex16(key & 0x0000FFFF);
}

void terminal_write(const char* data, size_t size)
{
	for (size_t i = 0; i < size; i++)
		terminal_putchar(data[i]);
}
 
void terminal_writestring(const char* data)
{
	terminal_write(data, strlen(data));
}

uint32_t memupper_size;
extern uint32_t endkernel;
extern uint32_t sbss;
extern uint32_t ebss;

void test(void)
{
	for (;;)
	{
		terminal_writestring("In thread 2");
		preempt();
	}
}

ext2_extended_superblock_t superblock;


void usermode_switch(void);

uint32_t get_year(void);
uint8_t get_day(void);

extern volatile uint32_t jiffies;

struct inode_operations ext2_inode_ops;

void kernel_main(multiboot_info_t* mbd, unsigned int magic, uint32_t kernel_stack)
{
	/* Initialize terminal interface */
	terminal_initialize();

	jiffies = 0;
	
	//Get contiguous memory size in kibibytes
	if (mbd->flags)
	{
		memupper_size = mbd->mem_upper;
	}

	gdt_install();
	idt_install();
	keyboard_install();
	timer_install();
	timer_set_interval(HZ);
	rtc_install();
	syscall_install();
	irq_install();

	init_frames((uint32_t)&endkernel, memupper_size);
	init_paging((uint32_t)&endkernel);
	terminal_writestring("paging initialized");
	uint32_t heap_start = kalloc_page()/* | KERNEL_VIRTUAL_BASE*/;

	kalloc_page();
	kalloc_page();
        kalloc_page();
	kalloc_page();
	kalloc_page();
	kalloc_page();
	kalloc_page();
	kalloc_page();
	kalloc_page();
	kalloc_page();
	kalloc_page();
	kalloc_page();
	kalloc_page();
	kalloc_page();

        kheap_init(heap_start, heap_start + 0xF000);

	//terminal_printhex16(pci_check_vendor(0,0));

	init_tasking();
	kthread_t *kthread1 = (kthread_t*)kmalloc(sizeof(kthread_t));
	kthread_create(kthread1, test);
	kthread_join(kthread1);
	kfree(kthread1);

        bdev_t dev;
	dev.dev_ctl = 0x3F6;
	dev.base = PRIMARY_BUS;
	dev.start_lba = 0;
	dev.slave_bit = 0;
	//terminal_writestring("ATA DRIVE:\n");
	//terminal_printhex(detect_device_type(0, &dev));

	//terminal_printhex16(buffer[3]);
	//ata_read_28_pio((void*)buffer, 1, dev, 1);
	//ata_write_28_pio((void*)buffer, 1, dev, 0);
	//ata_read_28_pio((void*)buffer, 1 , dev, 0);
	//terminal_printhex16(buffer[3]);
	outb(0x1F3, 0x88);
	if (inb(0x1F3) == 0x88)
		terminal_writestring("Primary controller exists\n");
	outb(0x1F6, 0xA0);
        inb(0x1F7);
	inb(0x1F7);
	inb(0x1F7);
	if (inb(0x1F7) & 0x40)
		terminal_writestring("Primary slave drive exists\n");

	parse_mbr(&dev);

	device_t dev_num = add_bdev(&dev);
	
	init_buffer();

	struct file_system_type root_fs;
	root_fs.read_super = &ext2_read_super;
	root_fs.name = "ext2";
	root_fs.next = NULL;
	init_fs(&root_fs, dev_num);
	struct inode inode_tmp;
	inode_tmp.i_sb = find_super_block("ext2");
	inode_tmp.dev = dev_num;
	inode_tmp.i_no = 2;
	inode_tmp.i_ops = &ext2_inode_ops;
	struct inode *dest = (struct inode*)kmalloc(sizeof(struct inode));
	inode_tmp.i_ops->lookup(&inode_tmp, "file1.txt", 9, &dest);
	struct file tmp_file = {dest, "file1.txt", 0};
	char tmp_buff[512];
	dest->dev = inode_tmp.dev;
	dest->i_sb = find_super_block("ext2");
	//terminal_printhex32(dest->i_no);for(;;);
	inode_tmp.i_ops->default_file_ops->read(&tmp_file, tmp_buff, 100, &(tmp_file.pos));
	terminal_writestring(tmp_buff);
	terminal_putchar('\n');
	terminal_printhex32(dest->i_no);for(;;);
	terminal_writestring(tmp_buff);
	for(;;);
/*
	//extended_superblock_t *sb = (extended_superblock_t*)kmalloc(sizeof(superblock_t));

	struct file_system_type ext2 = {ext2_read_super, "ext2", NULL};
	
	if (register_filesystem(&ext2) != 0)
		kpanic("zero");

	struct inode *test = (struct inode*)kmalloc(sizeof(struct inode));

	struct super_block testsb;
	ext2.read_super(&testsb, &dev);
	terminal_writestring(testsb.fs_type->name);

	test->i_no = 2;

	testsb.sb_ops->read_inode(test);

	struct file filp;
	filp.f_inode = test;
	filp.name = "root";

	char ibuff[512];

	size_t off = 0;

        test->i_ops->default_file_ops->read(&filp, ibuff, 10, &off);


	for(;;);

	//ext2_parse_superblock(&superblock, &dev);

	uint8_t buffer[512];

	ext2_inode_t *inode = (ext2_inode_t*)kmalloc(sizeof(ext2_inode_t));

	read_inode(inode, 0x2, &dev);

	terminal_writestring("Time stuff");
	terminal_printhex32(inode->last_access);
	terminal_writestring("directory");
	terminal_printhex32(inode->direct_block[0]);

	//ext2_list_directory(inode, &dev);
        //read_file(inode, &dev);
	//for (;;);
	//char test[512] = "HEllo from SaxonOS";
	//ata_write_sects_28_pio(test, 1, dev, ADDR_OF_BLOCK(inode->direct_block[0]));

	//write_file("Hello from SaxonOS\n", 1, 19, inode, 0x0C, &dev);

	kfree(inode);
*/
	__asm__ __volatile__ ("sti");

	//terminal_putchar('\n');
	//terminal_printhex32(stack);
	//set_kernel_stack(tmp_stack);
        //usermode_switch();
	//switch_to_user_mode();
	//syscall_sys_read();

	//syscall_sys_write("in userland");
	//terminal_writestring("\nIn usermode");

	for(;;);
}

//extern void enter_usermode();

void usermode_switch(void)
{
	uint32_t stack;
	__asm__ __volatile__("movl %%esp, %0;":"=m"(stack)::"%esp");
	terminal_writestring("stack");
	terminal_printhex32(stack);
	set_kernel_stack(stack);
	enter_usermode();
}
