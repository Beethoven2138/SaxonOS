#ifndef _TASK_H
#define _TASK_H

#include <gdt.h>
#include <common.h>
#include <mm/paging.h>

typedef struct
{
	uint32_t eax, ebx, ecx, edx, esi, edi, esp, ebp, eip, eflags, cr3;
} Registers;

typedef struct kthread
{
	Registers regs;
	uint32_t virtual_addr;
	struct kthread *next;
} kthread_t;

static uint32_t flags, page_dir;

void init_tasking(void);
void kthread_create(kthread_t* task, void(*main)());
void kthread_join(kthread_t *thread);

void preempt(); // Switch task frontend

extern void switch_task(Registers *old, Registers *new); // The function which actually switches

#endif
