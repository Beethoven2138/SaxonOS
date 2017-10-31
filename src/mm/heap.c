#include <mm/heap.h>

void kheap_init(uint32_t start, uint32_t end)
{
	heap_start = start;
	heap_end = end;
	heap_size = heap_end - heap_start;

	memset((void*)heap_start, 0, heap_size);
	
	first = (mem_block_t*)heap_start;
	first->used = 0;
	first->prev = 0;
	first->next = 0;
	first->size = heap_size - sizeof(mem_block_t);
}

void *kmalloc(size_t size)
{
	if (size != 1)
		return kmalloc_alligned(size, 4);
        mem_block_t *result = 0;
	for (mem_block_t *block = first; block != 0 && result == 0; block = block->next)
	{
		if (block->size > size && !block->used)
			result = block;
	}

	if (result == 0)
		return 0;


	if (result->size >= size + sizeof(struct mem_block) + 1)
	{
		mem_block_t *tmp = (mem_block_t*)((size_t)result + sizeof(mem_block_t) + size);

		tmp->used = false;
		tmp->size = result->size - size - sizeof(mem_block_t);
		tmp->prev = result;
		tmp->next = result->next;
		if (tmp->next != 0)
		{
			tmp->next->prev = tmp;
		}

		result->size = size;
		result->next = tmp;
	}

	result->used = true;

	return (void*)(((size_t)result) + sizeof(mem_block_t));
}

/*
  Allocate memory at an alligned address
 */
void *kmalloc_alligned(size_t size, uint32_t allignment)
{
	mem_block_t *result = 0;
	for (mem_block_t *block = first; block != 0 && result == 0; block = block->next)
	{
		if (block->used == false)
		{
			if (((uint32_t)block + sizeof(mem_block_t)) % allignment != 0)
			{
				const uint32_t alligned_addr = CEILING_DIV((uint32_t)block + sizeof(mem_block_t), allignment) * allignment;
				if (alligned_addr + size > (uint32_t)block + sizeof(mem_block_t) + block->size)
					continue;

				if ((uint32_t)block + sizeof(mem_block_t) + block->size - alligned_addr >= size + sizeof(mem_block_t))
				{
					mem_block_t *old_next = block->next;

					block->next = (mem_block_t*)(alligned_addr - sizeof(mem_block_t));

					block->size = (uint32_t)block->next - (uint32_t)block - sizeof(mem_block_t);
					if (block->prev != 0)
					{
						block->prev->size += block->size + sizeof(mem_block_t);
						block->prev->next = block->next;
						block = block->prev;
					}
					(*((mem_block_t*)block->next)).prev = block;
					block = block->next;
					block->next = old_next;
					if (old_next != 0)
						old_next->prev = block;
					result = block;
				}
			}
			else if (block->size > size)
				result = block;
		}
	}

	if (result == 0)kpanic("can't allocate memory");
		//return 0;

	if (result->size >= size + sizeof(struct mem_block) + 1)
	{
		mem_block_t *tmp = (mem_block_t*)((size_t)result + sizeof(mem_block_t) + size);

		tmp->used = false;
		tmp->size = result->size - size - sizeof(mem_block_t);
		tmp->prev = result;
		tmp->next = result->next;
		if (tmp->next != 0)
		{
			tmp->next->prev = tmp;
		}

		result->size = size;
		result->next = tmp;
	}

	result->used = true;
	
	return (void*)(((size_t)result) + sizeof(mem_block_t));
}

void *krealloc(void *ptr, size_t size)
{
	if (ptr == 0)
	{
		return kmalloc(size);
	}
	if (size == 0)
	{
		kfree(ptr);
		return 0;
	}
	mem_block_t *old_block = (mem_block_t*)((size_t)ptr - sizeof(mem_block_t));

	const size_t old_size = old_block->size;
	uint8_t *tmp[old_size];
	memcpy(tmp, ptr, old_size);
	kfree_block(old_block);

        void *new_block = (void*)((size_t)kmalloc(size) + sizeof(mem_block_t));
	memcpy(new_block, ptr, old_size);
	return new_block;
}

void *kcalloc(size_t nmemb, size_t size)
{
	if (nmemb == 0 || size == 0)
		return NULL;
        void *new_block = (void*)((size_t)kmalloc(size * nmemb) + sizeof(mem_block_t));
	return new_block;
}

void kfree(void *ptr)
{
	mem_block_t *tmp = (mem_block_t*)((size_t)ptr - sizeof(mem_block_t));

	tmp->used = false;

	if (tmp->prev != NULL && !tmp->prev->used)
	{
		tmp->prev->size += sizeof(struct mem_block) + tmp->size;
		tmp->prev->next = tmp->next;

		if (tmp->next != NULL)
			tmp->next->prev = tmp->prev;
		tmp = tmp->prev;
	}

	if (tmp->next != NULL && !tmp->next->used)
	{
		tmp->size += sizeof(struct mem_block) + tmp->next->size;
		tmp->next = tmp->next->next;
		if (tmp->next != NULL)
			tmp->next->prev = tmp;
	}
}

static void kfree_block(mem_block_t *block)
{
        block->used = false;

	if (block->prev != NULL && block->prev->used == false)
	{
		block->prev->size += sizeof(struct mem_block) + block->size;
		block->prev->next = block->next;

		if (block->next != NULL)
			block->next->prev = block->prev;
		block = block->prev;
	}

	if (block->next != NULL && block->next->used == false)
	{
		block->size += sizeof(struct mem_block) + block->next->size;
		block->next = block->next->next;
		if (block->next != NULL)
			block->next->prev = block;
	}
}
