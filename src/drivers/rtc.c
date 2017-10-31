#include <drivers/rtc.h>

/* set by ACPI table parsing code*/
uint32_t century_register = 0x00;

static uint8_t second;
static uint8_t minute;
static uint8_t hour;
static uint8_t day;
static uint8_t month;
static uint32_t year;
static uint8_t century;

void rtc_install(void)
{
	//Interrupts must be disabled here
	outb(CMOS_ADDRESS, 0x8B);//disable Non-maskable interrupts
	char prev = inb(CMOS_DATA);
	outb(CMOS_ADDRESS, 0x8B);
	outb(CMOS_DATA, prev | 0x40);
	irq_install_handler(8, rtc_handler);
}

void rtc_uninstall(void)
{
	irq_uninstall_handler(8);
}

void rtc_handler(struct regs *r)
{
	outb(CMOS_ADDRESS, 0x0C);
	inb(CMOS_DATA);
	read_rtc();
}

static uint8_t get_rtc_register(uint32_t reg)
{
	outb(CMOS_ADDRESS, reg);
	return inb(CMOS_DATA);
}


static int update_in_progress_flag(void)
{
	outb(CMOS_ADDRESS, 0x0A);
	return (inb(CMOS_DATA) & 0x80);
}

static void read_rtc(void)
{
	while (update_in_progress_flag());

	second = get_rtc_register(0x00);
	minute = get_rtc_register(0x02);
	hour = get_rtc_register(0x04);
	day = get_rtc_register(0x07);
	month = get_rtc_register(0x08);
	year = get_rtc_register(0x09);

	if (century_register != 0)
		century = get_rtc_register(century_register);

	uint32_t registerB = get_rtc_register(0x0B);

	/*convert BCD to binary*/
	if (!(registerB & 0x04))
	{
		/*binary = ((bcd / 16) * 10) + (bcd & 0xf)*/
		second = (second & 0x0F) + ((second / 16) * 10);
		minute = (minute & 0x0F) + ((minute / 16) * 10);
		hour = ( (hour & 0x0F) + (((hour & 0x70) / 16) * 10) ) | (hour & 0x80);
		day = (day & 0x0F) + ((day / 16) * 10);
		month = (month & 0x0F) + ((month / 16) * 10);
		year = (year & 0x0F) + ((year / 16) * 10);
		if(century_register != 0)
			century = (century & 0x0F) + ((century / 16) * 10);
	}

	/*convert 12 hour clock to 24 hour clock*/
	if (!(registerB & 0x02) && (hour & 0x80))
	{
		hour = ((hour & 0x7F) + 12) % 24;
	}

	if (century_register != 0)
		year += century * 100;
	else
	{
		year += (CURRENT_YEAR / 100) * 100;
		if (year < CURRENT_YEAR)
			year += 100;
	}
}

extern uint32_t get_year(void)
{
	return year;
}

extern uint8_t get_month(void)
{
	return month;
}

extern uint8_t get_day(void)
{
	return day;
}

extern uint8_t get_hour(void)
{
	return hour;
}

extern uint8_t get_minute(void)
{
	return minute;
}

extern uint8_t get_second(void)
{
	return second;
}
