#include <mm/paging.h>

void terminal_printhex32(uint32_t);

page_directory_t kernel_page_dir __attribute__((aligned(4096)));
static page_table_t first_page_table __attribute__((aligned(4096)));

static uint32_t cur_page_table;
static uint32_t next_free_page;

void init_paging(uint32_t end_kernel)
{
	const uint32_t first_free_page = (end_kernel & 0x00FFF000) + 0x1000;
	for (uint32_t i = 0; i < 1024; i++)
	{
	        kernel_page_dir.pd_entry[i] = 0x00000002;
	}
	for (int i = 0; i < first_free_page / 0x1000; i++)
	{
		first_page_table.pt_entry[i] = (i * 0x1000) | 3 | 4;
	}
        kernel_page_dir.pd_entry[KERNEL_PAGE_NUMBER] = (((uint32_t)first_page_table.pt_entry) - KERNEL_VIRTUAL_BASE) | 3 | 4;
	cur_page_table = KERNEL_PAGE_NUMBER;
	loadPageDir((uint32_t)&kernel_page_dir.pd_entry[0] - KERNEL_VIRTUAL_BASE);
	next_free_page = (uint32_t)first_free_page + KERNEL_VIRTUAL_BASE;
}

uint32_t kalloc_page(void)
{
	const uint32_t frame = kalloc_frame();
	first_page_table.pt_entry[(next_free_page - KERNEL_VIRTUAL_BASE) / 0x1000] = frame | 3 | 4;
        uint32_t ret = next_free_page;
	next_free_page = find_free_page(&first_page_table);
	return ret;
}

void kfree_page(page_directory_t *dir, uint32_t virtual_addr)
{
	pageframe_t phys_addr = get_phys_addr(dir, virtual_addr);
	kfree_frame(phys_addr);

	uint32_t pd_index = (uint32_t)virtual_addr >> 22;
        uint32_t pt_index = (uint32_t)virtual_addr >> 12 & 0x03FF;

	uint32_t *pt = (uint32_t*)((dir->pd_entry[pd_index] & 0xFFFFF000) + KERNEL_VIRTUAL_BASE);

        pt[pt_index] = 0;
	next_free_page = virtual_addr;
}

static uint32_t find_free_page(page_table_t *table)
{
	for (int i = 0; i < 1024; i++)
	{
		if (!PAGE_PRESENT(table->pt_entry[i]))
			return (i * 0x1000) + KERNEL_VIRTUAL_BASE;
	}
	kpanic("Ran out of free pages");
	return -1;
}

pageframe_t get_phys_addr(page_directory_t *dir, uint32_t virtual_addr)
{
	uint32_t pd_index = (uint32_t)virtual_addr >> 22;
        uint32_t pt_index = (uint32_t)virtual_addr >> 12 & 0x03FF;

        if (!PAGE_PRESENT(dir->pd_entry[pd_index]))
		kpanic("Page table not present");
	uint32_t *pt = (uint32_t*)((dir->pd_entry[pd_index] & 0xFFFFF000) + KERNEL_VIRTUAL_BASE);
	if (!PAGE_PRESENT(/*first_page_table[pt_index]*/ pt[pt_index]))
		kpanic("Page table entry not present");

	return (pageframe_t)((pt[pt_index] & ~0xFFF) + ((uint32_t)virtual_addr & 0xFFF));
}

uint32_t alloc_page_table(page_directory_t *dir, uint32_t virtual_addr, uint32_t flags)
{
	if (dir->pd_entry[virtual_addr >> 22] == 0)
	{
		const uint32_t frame = kalloc_frame();
	        dir->pd_entry[virtual_addr >> 22] = frame | flags;
		memset(frame | KERNEL_VIRTUAL_BASE, 0, 4096);
	}
	return virtual_addr;
}
