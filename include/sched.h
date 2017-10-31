#ifndef __SCHED_H
#define __SCHED_H

#define TASK_RUNNING		0
#define TASK_INTERRUPTIBLE	1
#define TASK_UNINTERRUPTIBLE	2
#define TASK_ZOMBIE		3
#define TASK_STOPPED		4

#define MAX_OPEN_FILES 100
#define MAX_TASKS 100



struct task_struct
{
	uint32_t state;
	uint32_t priority;
	uint32_t counter; /*in jiffies*/
	uint32_t pid;
	struct task_struct father;
	struct file *filp[MAX_OPEN_FILES];
	struct tss_struct tss;
};

extern struct task_struct task[MAX_TASKS];
extern struct task_struct *current;

#endif
