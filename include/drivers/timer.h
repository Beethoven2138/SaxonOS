#ifndef __DRIVERS__TIMER_H
#define __DRIVERS__TIMER_H

#include <common.h>
#include <irq.h>

void timer_handler(struct regs *r);
void timer_install();
void timer_set_interval(uint32_t hz);

#endif
