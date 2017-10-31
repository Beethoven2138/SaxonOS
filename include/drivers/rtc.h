#ifndef __DRIVERS__RTC_H
#define __DRIVERS__RTC_H

#include <common.h>
#include <irq.h>

#define CMOS_ADDRESS 0x70
#define CMOS_DATA 0x71

#define CURRENT_YEAR 2017

void rtc_install(void);
void rtc_uninstall(void);

void rtc_handler(struct regs *r);

static uint8_t get_rtc_register(uint32_t reg);
static int update_in_progress_flag(void);
static void read_rtc(void);

extern uint32_t get_year(void);
extern uint8_t get_month(void);
extern uint8_t get_day(void);
extern uint8_t get_hour(void);
extern uint8_t get_minute(void);
extern uint8_t get_second(void);

#endif
