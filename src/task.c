#include <task.h>
#include <mm/paging.h>

static kthread_t main_thread;
static kthread_t *last_thread;
static kthread_t *running_thread;

page_directory_t kernel_page_dir __attribute__((aligned(4096)));

void init_tasking(void) {
	// Get EFLAGS and CR3
	asm volatile("movl %%cr3, %%eax; movl %%eax, %0;":"=m"(main_thread.regs.cr3)::"%eax");
	asm volatile("pushfl; movl (%%esp), %%eax; movl %%eax, %0; popfl;":"=m"(main_thread.regs.eflags)::"%eax");

	asm volatile("pushfl; movl (%%esp), %%eax; movl %%eax, %0; popfl;":"=m"(flags)::"%eax");
	asm volatile("movl %%cr3, %%eax; movl %%eax, %0;":"=m"(page_dir)::"%eax");

        main_thread.next = NULL;
	running_thread = &main_thread;
	last_thread = &main_thread;
}
 
void kthread_create(kthread_t *task, void (*main)()) {
	task->regs.eax = 0;
	task->regs.ebx = 0;
	task->regs.ecx = 0;
	task->regs.edx = 0;
	task->regs.esi = 0;
	task->regs.edi = 0;
	task->regs.eflags = flags;
	task->regs.eip = (uint32_t) main;
	task->regs.cr3 = (uint32_t) page_dir;
	task->virtual_addr = (uint32_t) kalloc_page();
	task->regs.esp = task->virtual_addr + 0x1000;
	task->next = 0;
	last_thread->next = task;
	last_thread = task;
	preempt();
}

void kthread_join(kthread_t *thread)
{
	for (kthread_t *tmp = &main_thread; tmp->next != thread; tmp = tmp->next)
	{
		tmp->next = thread->next;
		last_thread = tmp;
	}
	kfree_page(&kernel_page_dir, thread->virtual_addr);
}

void preempt(void)
{
	kthread_t *prev = running_thread;
	if (running_thread == last_thread)
		running_thread = &main_thread;
	else
		running_thread = running_thread->next;
	switch_task(&prev->regs, &running_thread->regs);
}


/*void switch_to_user_mode(void)
{
	// Set up a stack structure for switching to user mode.
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
     pop %eax; \
     or %eax, $0x200; \
     push %eax; \
     pushl $0x1B; \
     push $1f; \
     iret; \
   1: \
     ");
     }*/

/*tss_entry_t tss_entry;

void write_tss(void)
{
	gdt_set_gate(5, &tss_entry, (uint32_t)&tss_entry + sizeof(tss_entry), 0xE9, 0x00);

	memset(&tss_entry, 0, sizeof(tss_entry));
	tss_entry.ss0 = KERNEL_DATA_SEG;
	tss_entry.cs = 0x0B;
	tss_entry.ss = 0x13;
	tss_entry.ds = 0x13;
	tss_entry.es = 0x13;
	tss_entry.fs = 0x13;
	tss_entry.gs = 0x13;
}

void set_kernel_stack(uint32_t stack)
{
	tss_entry.esp0 = stack;
}
*/
