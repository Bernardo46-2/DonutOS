#include "../include/pci.h"
#include "../include/asm.h"
#include "../include/tty.h"
#include "../../libc/include/itoa.h"
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

static inline uint32_t pci_config_read_data() {
    return inl(CONFIG_DATA);
}

static uint32_t pci_config_read(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
    uint32_t tmp   = 0;

    // 31     - Enable bit
    // 30-24  - Reserved
    // 23-16  - Bus  Number
    // 15-11  - Slot Number
    // 10-8   - Func Number
    // 7-0    - Register Offset
    address = (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));

    pci_config_write_addr(address);
    return pci_config_read_data();
}

void pci_read_bars(pci_device_t pci_device) {
    printf("Lendo BARs do dispositivo VirtIO:\n");
    for (int bar_num = 0; bar_num < 6; bar_num++) {
        uint32_t bar_value = pci_config_read(pci_device.bus, pci_device.device, pci_device.func, 0x10 + (bar_num * 4));
        printf("BAR%d: 0x%X\n", bar_num, bar_value);
    }
}

static inline int pci_touch_device(uint8_t bus, uint8_t device, uint8_t func) {
    return (pci_config_read(bus, device, func, PCI_VENDOR_ID) & 0xffff) != 0xFFFF;
}

// Verify and register a PCI device
static void pci_check_device(uint8_t bus, uint8_t device, uint8_t func, pci_device_t *pci_device, uint16_t *counter) {
    uint16_t vendor_id;
    uint16_t device_id;

    if ((vendor_id = pci_config_read(bus, device, func, PCI_VENDOR_ID) & 0xffff) == 0xFFFF) return;

    (*counter)++;

    device_id = pci_config_read(bus, device, func, PCI_DEVICE_ID) >> 16;
    pci_device->vendor_id = vendor_id;
    pci_device->device_id = device_id;
    pci_device->bus       = bus;
    pci_device->device    = device;
    pci_device->func      = func;
}

// Scan the PCI bus and identify devices
void pci_scan_bus() {
    if (devices) {
        free(devices);
        devices = NULL;
        devices_size = 0;
    }

    uint16_t bus, device, func, i = 0;
    pci_device_t tmp;

    for (bus = 0; bus < 256; bus++) {
        for (device = 0; device < 32; device++) {
            for (func = 0; func < 8; func++) {
                devices_size += pci_touch_device(bus, device, func);
            }
        }
    }

    devices = malloc(sizeof(pci_device_t) * devices_size);

    for (bus = 0; bus < 256; bus++) {
        for (device = 0; device < 32; device++) {
            for (func = 0; func < 8; func++) {
                pci_check_device(bus, device, func, &devices[i], &i);
            }
        }
    }
}

int pci_get_device(uint16_t vendor_id, uint16_t device_id, pci_device_t *target) {
    for (int i = 0; i < devices_size; i++) {
        if (devices[i].vendor_id == vendor_id && devices[i].device_id == device_id) {
            (*target) = devices[i];
            return 1;
        }
    }
    return 0;
}
