#ifndef __MM_PAGING_H
#define __MM_PAGING_H

#include <common.h>
#include <mm/frame.h>

#define KERNEL_VIRTUAL_BASE 0xC0000000
#define KERNEL_PAGE_NUMBER KERNEL_VIRTUAL_BASE >> 22

#define PAGE_PRESENT(page) (page & 0x01)
#define PAGE_RW_O(page) (page & 0x02)
#define PAGE_SUPERVISOR_O(page) (!(page & 0x04))
#define PAGE_WRITE_THROUGH(page) (page & 0x08)
#define PAGE_BLOCK_BACK(page) (!(page & 0x08))
#define PAGE_TABLE_CACHE_ENABLED(page) (!(page & 0x10))
#define PAGE_TABLE_CACHE_DISABLED(page) (page & 0x10)
#define PAGE_ACCESSED(page) (page & 0x20)
#define PAGE_TABLE_SIZE(page) ((page & 0x80) ? 0x100000 : 0x400)

extern void loadPageDir(uint32_t*);
extern void enablePaging();

typedef struct page_table
{
	uint32_t pt_entry[1024];
} page_table_t;

typedef struct page_directory
{
        uint32_t pd_entry[1024];
} page_directory_t;

//uint32_t page_directory[1024] __attribute__((aligned(4096)));

//uint32_t first_page_table[1024] __attribute__((aligned(4096)));

extern page_directory_t kernel_page_dir __attribute__((aligned(4096)));
//static page_table_t first_page_table __attribute__((aligned(4096)));

static uint32_t next_free_page;

void init_paging(uint32_t end_kernel);

uint32_t kalloc_page(void);
void kfree_page(page_directory_t *dir, uint32_t virtual_addr);

static uint32_t find_free_page(page_table_t *table);

pageframe_t get_phys_addr(page_directory_t *dir, uint32_t virtual_addr);

#endif
