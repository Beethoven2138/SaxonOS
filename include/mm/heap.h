#ifndef __MM__HEAP_H
#define __MM__HEAP_H

#include <common.h>

uint32_t heap_start;
uint32_t heap_end;
uint32_t heap_size;

typedef struct mem_block
{
	bool used;

	struct mem_block *prev;
	struct mem_block *next;

	uint32_t size;
} mem_block_t;

struct mem_block *first;

void kheap_init(uint32_t start, uint32_t end);

/*These functions work the same way as the
  ones in the C standard library on GNU/Linux*/
void *kmalloc(size_t size);
void *krealloc(void *ptr, size_t size);
void *kcalloc(size_t nmemb, size_t size);
void kfree(void *ptr);
static void kfree_block(mem_block_t *block);

void *kmalloc_alligned(size_t size, uint32_t allignment);

#endif
