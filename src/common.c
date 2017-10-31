#include <common.h>

void outb(uint16_t port, uint8_t value){
	asm volatile ("outb %1, %0" : : "d" (port), "a" (value));
}

void outw(uint16_t port, uint16_t value){
	asm volatile ("outw %1, %0" : : "dN" (port), "a" (value));
}

void outl(uint16_t port, uint32_t value){
	asm volatile ("outl %1, %0" : : "dN" (port), "a" (value));
}

uint8_t inb(uint16_t port){
	uint8_t ret;
	asm volatile("inb %1, %0" : "=a" (ret) : "dN" (port));
	return ret;
}

uint16_t inw(uint16_t port){
	uint16_t ret;
	asm volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));
	return ret;
}

uint32_t inl(uint16_t port){
	uint32_t ret;
	asm volatile ("inl %1, %0" : "=a" (ret) : "dN" (port));
	return ret;
}

size_t strlen(const char *str)
{
	size_t i = 0;
	for (str; str[i] != 0; i++);
	return i;
}

int strcmp(const char *s1, const char *s2)
{
	for (size_t i = 0; i; i++)
	{
		if (*(s1 + i) != *(s2 + i))
			return *(s1 + i) - *(s2 + i);
	}
	return 0;
}

int strncmp(const char *s1, const char *s2, size_t n)
{
	if (n == 0)
		return 0;
	for (size_t i = 0; i < n; i++)
	{
		if (*(s1 + i) != *(s2 + i))
			return *(s1 + i) - *(s2 + i);
	}
	return 0;	
}

void *memset(void *dest, char value, size_t count)
{
	char *tmp = (char*)dest;
	for (count; count != 0; count--)
		*tmp++ = value;

	return dest;
}


void *memcpy(void *dest, const void *src, size_t count)
{
	char *dest_tmp = (char*)dest;
	const char *src_tmp = (const char*)src;

	for (count; count > 0; count--)
		*dest_tmp++ = *src_tmp++;
	return dest;
}

void *memmove(void *dest, const void *src, size_t n)
{
        uint8_t tmp[n];
	memcpy((void*)tmp, src, n);
	memcpy(dest, (void*)tmp, n);
	return dest;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
	if (n == 0)
		return 0;
	
	for (size_t i = 0; i < n; i++)
	{
		if (*((uint8_t*)s1 + i) != *((uint8_t*)s2 + i))
			return *((uint8_t*)s1 + i) - *((uint8_t*)s2 + i);
	}
	return 0;
}

void kpanic(char *msg)
{
	terminal_setcolor(vga_entry_color(4, 1));
	terminal_writestring("KERNEL PANIC: ");
	terminal_writestring(msg);
	__asm__ __volatile__ ("cli");
	for(;;);
}
