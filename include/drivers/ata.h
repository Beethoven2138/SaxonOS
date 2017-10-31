#ifndef __DRIVERS__ATA_H
#define __DRIVERS__ATA_H

#include <fs/devfs/devfs.h>
#include <common.h>
//#include <fs/mbr.h>

#define PRIMARY_BUS 0x1F0
#define SECONDARY_BUS 0x170

#define MASTER_DRIVE 0xA0
#define SLAVE_DRIVE  0xB0

#define REG_DATA_PORT 0x00
#define REG_INFO_PORT 0x01
#define REG_SECT_CNT_PORT 0x02
#define REG_LBA_LOW_PORT 0x03
#define REG_LBA_MID_PORT 0x04
#define REG_LBA_HIGH_PORT 0x05
#define REG_DEV_SEL_PORT 0x06
#define REG_CMD_PORT 0x07

/*
always the same value as the REG_CMD_PORT but
reading this port doesn't effect interrupts
*/
#define REG_DEV_CTL_PORT 0x3F6

#define STATUS_ERR 0x01
#define STATUS_DRQ 0x08
#define STATUS_SRV 0x10
#define STATUS_DF  0x20
#define STATUS_RDY 0x40
#define STATUS_BSY 0x80

#define ATADEV_UNKNOWN 0x00
#define ATADEV_PATAPI 0x01
#define ATADEV_SATAPI 0x02
#define ATADEV_PATA 0x03
#define ATADEV_SATA 0x04

#define SOFT_RESET 0x04

#define IDENTIFY_CMD 0xEC
#define READ_SECTS_CMD 0x20
#define WRITE_SECTS_CMD 0x30
#define CACHE_FLUSH 0xE7

/*typedef struct device
{
	uint16_t base;
	uint16_t dev_ctl;
	uint8_t slave_bit;
	//uint32_t abs_lba;
	//partition_t partition;
	uint32_t start_lba;
	uint32_t part_size;
	} device_t;*/

typedef struct bdev bdev_t;

void ata_soft_reset(uint16_t cntrl);

//uint8_t detect_device_type(uint8_t slave_bit, bdev_t *dev);

//void ata_read_28_pio(void *dest, uint32_t count, bdev_t dev, uint32_t rel_lba);

void ata_read_sects_28_pio(void *dest, uint8_t sect_count, bdev_t *dev, uint32_t rel_lba);
void ata_write_sects_28_pio(void *src, uint8_t sect_count, bdev_t *dev, uint32_t rel_lba);

void ata_flush_cache(bdev_t *dev);

//void ata_read_sect_lba28_pio(uint16_t *dest, bdev_t dev, uint32_t lba);
//void ata_write_sect_lba28_pio(uint16_t *src, bdev_t dev, uint32_t lba);

void ata_read_28_pio_offw(void *dest, uint32_t sect_off/*should be in words*/, uint32_t count, bdev_t dev, uint32_t rel_lba);

#endif
