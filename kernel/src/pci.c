#include "../include/pci.h"
#include "../include/asm.h"
#include "../../libc/include/malloc.h"
#include "../../libc/include/printf.h"

#define CONFIG_ADDRESS    0xCF8
#define CONFIG_DATA       0xCFC

// Constantes para identificação PCI
#define PCI_VENDOR_ID     0x00
#define PCI_DEVICE_ID     0x02
#define PCI_CLASS_CODE    0x0b
#define PCI_SUBCLASS      0x0a
#define PCI_PROG_IF       0x09
#define PCI_HEADER_TYPE   0x0e
#define PCI_SECONDARY_BUS 0x19

pci_device_t *devices = NULL;
uint16_t devices_size = 0;

static inline void pci_config_write_addr(uint32_t addr) {
    outl(CONFIG_ADDRESS, addr);
}

static inline void pci_config_write_data(uint32_t data) {
    outl(CONFIG_DATA, data);
}

static inline uint32_t pci_config_read_data() {
    return inl(CONFIG_DATA);
}

static uint32_t pci_config_read(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
    uint32_t tmp = 0;

    // Configuration address
    address = (uint32_t)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));

    // Write the address
    pci_config_write_addr(address);

    return pci_config_read_data();
}

static inline int pci_touch_device(uint8_t bus, uint8_t device) {
    return ((pci_config_read(bus, device, 0, PCI_VENDOR_ID) & 0xffff) == 0xFFFF) ? 0 : 1;
}

// Verify and register a PCI device
static void pci_check_device(uint8_t bus, uint8_t device, pci_device_t *pci_device, uint16_t *counter) {
    uint16_t vendor_id;
    uint16_t device_id;

    if ((vendor_id = pci_config_read(bus, device, 0, PCI_VENDOR_ID) & 0xffff) == 0xFFFF) return;

    (*counter)++;

    device_id = pci_config_read(bus, device, 0, PCI_DEVICE_ID) >> 16;
    pci_device->vendor_id = vendor_id;
    pci_device->device_id = device_id;
}

// Scan the PCI bus and identify devices
void scan_pci_bus() {
    if (devices) {
        free(devices);
        devices = NULL;
        devices_size = 0;
    }

    uint16_t bus, device, i = 0;
    pci_device_t tmp;

    for (bus = 0; bus < 256; bus++) {
        for (device = 0; device < 32; device++) {
            devices_size += pci_touch_device(bus, device);
        }
    }

    devices = malloc(sizeof(pci_device_t) * devices_size);

    for (bus = 0; bus < 256; bus++) {
        for (device = 0; device < 32; device++) {
            pci_check_device(bus, device, &devices[i], &i);
        }
    }

    for (int j = 0; j < devices_size; j++) {
        printf("Vendor = 0x%x, Device = 0x%x\n", devices[j].vendor_id, devices[j].device_id);
    }
}
