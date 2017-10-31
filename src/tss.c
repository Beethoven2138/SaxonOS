#include <tss.h>

struct tss_struct tss_entry;

void write_tss(void)
{
	gdt_set_gate(5, (uint32_t)&tss_entry, (uint32_t)&tss_entry + sizeof(struct tss_struct), 0xE9, 0x00);

	memset((void*)&tss_entry, 0, sizeof(struct tss_struct));
	tss_entry.ss0 = KERNEL_DATA_SEG;
	asm volatile("movl %%esp, %%eax; movl %%eax, %0;":"=m"(tss_entry.esp0)::"%eax");
	//tss_entry.esp0 = 0;
	tss_entry.cs = 0x0B;
	tss_entry.ss = 0x13;
	tss_entry.ds = 0x13;
	tss_entry.es = 0x13;
	tss_entry.fs = 0x13;
	tss_entry.gs = 0x13;

	tss_entry.iomap_base = sizeof(tss_entry);
}

void set_kernel_stack(uint32_t stack)
{
	tss_entry.esp0 = stack;
	//asm volatile("movl %%cr3, %%eax; movl %%eax, %0;":"=m"(tss_entry.esp0)::"%eax");
}

/*void switch_to_user_mode(void)
{
    asm volatile("  \
        cli; \
        mov $0x23, %ax; \
        mov %ax, %ds; \
        mov %ax, %es; \
        mov %ax, %fs; \
        mov %ax, %gs; \
                \ 
        mov %esp, %eax; \
        pushl $0x23; \
        pushl %eax; \
        pushf; \
        mov $0x200, %eax; \
        push %eax; \
        pushl $0x1B; \
        push $1f; \
        iret; \    
        1: \
    "); 
}
*/
