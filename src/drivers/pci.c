#include <drivers/pci.h>

uint32_t pci_config_read_double(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset)
{
	uint32_t address;
	uint32_t lbus  = (uint32_t)bus;
	uint32_t ldevice = (uint32_t)device;
	uint32_t lfunc = (uint32_t)func;
	uint16_t tmp = 0;
 

	address = (uint32_t)((lbus << 16) | (ldevice << 11) |
			     (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));
 
	/* write out the address */
        outl(0xCF8, address);
	/* read in the data */
	/* (offset & 2) * 8) = 0 will choose the first word of the 32 bits register */
	tmp = (uint32_t)((inl(0xCFC) >> ((offset & 2) * 8)) & 0xffff);
	return (tmp);
}

uint16_t pci_config_read_word(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset)
{
	uint32_t address;
	uint32_t lbus  = (uint32_t)bus;
	uint32_t ldevice = (uint32_t)device;
	uint32_t lfunc = (uint32_t)func;
	uint16_t tmp = 0;
 

	address = (uint32_t)((lbus << 16) | (ldevice << 11) |
			     (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));
 
	/* write out the address */
        outl(0xCF8, address);
	/* read in the data */
	/* (offset & 2) * 8) = 0 will choose the first word of the 32 bits register */
	tmp = (uint16_t)((inl(0xCFC) >> ((offset & 2) * 8)) & 0xffff);
	return (tmp);
}

uint8_t pci_config_read_byte(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset)
{
	uint32_t address;
	uint32_t lbus  = (uint32_t)bus;
	uint32_t ldevice = (uint32_t)device;
	uint32_t lfunc = (uint32_t)func;
	uint16_t tmp = 0;
 

	address = (uint32_t)((lbus << 16) | (ldevice << 11) |
			     (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));
 

        outl(0xCF8, address);

	tmp = (uint8_t)((inl(0xCFC) >> ((offset & 2) * 8)) & 0xffff);
	return (tmp);
}

uint16_t pci_get_vendor(uint8_t bus, uint8_t slot)
{
	/* try and read the first configuration register. Since there are no */
	/* vendors that == 0xFFFF, it must be a non-existent device. */
	return pci_config_read_word(bus, slot, 0, 0);
}

uint16_t pci_get_device(uint8_t bus, uint8_t slot)
{
        return pci_config_read_word(bus, slot, 0, 2);
}

uint16_t pci_get_status(uint8_t bus, uint8_t slot)
{
	return pci_config_read_word(bus, slot, 0, 4);
}

uint16_t pci_get_command(uint8_t bus, uint8_t slot)
{
	return pci_config_read_word(bus, slot, 0, 6);
}

uint32_t pci_get_bar(uint8_t bus, uint8_t slot, uint8_t bar)
{
	return pci_config_read_word(bus, slot, 0, bar);
}

pci_dev_t fill_device_descriptor(uint8_t bus, uint8_t device, uint8_t function)
{
	pci_dev_t result;

	result.bus = bus;
	result.dev = device;
	result.func = function;
	result.vendor_id = pci_config_read_word(bus, device, function, 0x00);
	result.dev_id = pci_config_read_word(bus, device, function, 0x02);
	result.class_id = pci_config_read_byte(bus, device, function, 0x0B);
	result.subclass_id = pci_config_read_byte(bus, device, function, 0x0A);
	result.interface_id = pci_config_read_byte(bus, device, function, 0x09);
	result.revision = pci_config_read_byte(bus, device, function, 0x08);
	result.interrupt = pci_config_read_double(bus, device, function, 0x3C);

	return result;
}
