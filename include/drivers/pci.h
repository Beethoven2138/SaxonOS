#ifndef __DRIVERS_PCI_H
#define __DRIVERS_PCI_H

#include <common.h>

/*specifies the address into the configuration space*/
#define CONFIG_ADDRESS 0xCF8
/*generates the configuration access and transfers
  data into the CONFIG_DATA register*/
#define CONFIG_DATA 0xCFC

/*Base address registers*/
#define BAR0 0x10
#define BAR1 0x14
#define BAR2 0x18
#define BAR3 0x1C
#define BAR4 0x20
#define BAR5 0x24

typedef struct pci_dev
{
	uint32_t port_base;
	uint32_t interrupt;

	uint16_t bus;
	uint16_t dev;
	uint16_t func;

	uint16_t vendor_id;
	uint16_t dev_id;

	uint8_t class_id;
	uint8_t subclass_id;
	uint8_t interface_id;

	uint8_t revision;
} pci_dev_t;

uint32_t pci_config_read_double(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset);
uint16_t pci_config_read_word(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset);
uint8_t pci_config_read_byte(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset);
uint16_t pci_get_vendor(uint8_t bus, uint8_t slot);
uint16_t pci_get_device(uint8_t bus, uint8_t slot);
uint16_t pci_get_status(uint8_t bus, uint8_t slot);
uint16_t pci_get_command(uint8_t bus, uint8_t slot);

uint32_t pci_get_bar(uint8_t bus, uint8_t slot, uint8_t bar);

#endif
