#include "../include/pci.h"
#include "../include/asm.h"
#include "../include/tty.h"
#include "../../libc/include/itoa.h"
#include "../../libc/include/malloc.h"
#include "../../libc/include/printf.h"

pci_device_t *devices = NULL;
uint16_t devices_size = 0;

static inline void pci_config_write_addr(uint32_t addr) {
    outl(PCI_CONFIG_ADDRESS, addr);
}

static inline uint32_t pci_config_read_data() {
    return inl(PCI_CONFIG_DATA);
}

static inline void pci_config_write_data(uint32_t data) {
    outl(PCI_CONFIG_DATA, data);
}

static void pci_config_write(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t data) {
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;

    // 31     - Enable bit
    // 30-24  - Reserved
    // 23-16  - Bus  Number
    // 15-11  - Slot Number
    // 10-8   - Func Number
    // 7-0    - Register Offset
    address = (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));
    
    pci_config_write_addr(address);
    pci_config_write_data(data);
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

void pci_read_bars(pci_device_t pci_device, pci_bars *pci_bars) {
    for (int bar_num = 0; bar_num < 6; bar_num++) {
        pci_bars->bar[bar_num] = pci_config_read(pci_device.bus, pci_device.slot, pci_device.func, 0x10 + (bar_num * 4));
    }
}

static inline int pci_touch_device(uint8_t bus, uint8_t slot, uint8_t func) {
    return (pci_config_read(bus, slot, func, PCI_VENDOR_ID) & 0xffff) != 0xFFFF;
}

// Verify and register a PCI slot
static void pci_check_and_get_device(uint8_t bus, uint8_t slot, uint8_t func, pci_device_t *pci_device, uint16_t *counter) {
    uint16_t vendor_id;
    uint16_t device_id;

    if ((vendor_id = pci_config_read(bus, slot, func, PCI_VENDOR_ID) & 0xffff) == 0xFFFF) return;

    (*counter)++;

    device_id = pci_config_read(bus, slot, func, PCI_DEVICE_ID) >> 16;
    pci_device->vendor_id = vendor_id;
    pci_device->device_id = device_id;
    pci_device->bus       = bus;
    pci_device->slot      = slot;
    pci_device->func      = func;

    pci_read_bars(*pci_device, &(pci_device->bars));
    pci_device->irq = pci_config_read(bus, slot, func, 0x3C) & 0xFF;
}

// Scan the PCI bus and identify devices
void pci_scan_bus() {
    if (devices) {
        free(devices);
        devices = NULL;
        devices_size = 0;
    }

    uint16_t bus, slot, func, i = 0;
    pci_device_t tmp;

    for (bus = 0; bus < 256; bus++) {
        for (slot = 0; slot < 32; slot++) {
            for (func = 0; func < 8; func++) {
                devices_size += pci_touch_device(bus, slot, func);
            }
        }
    }

    devices = malloc(sizeof(pci_device_t) * devices_size);

    for (bus = 0; bus < 256; bus++) {
        for (slot = 0; slot < 32; slot++) {
            for (func = 0; func < 8; func++) {
                pci_check_and_get_device(bus, slot, func, &devices[i], &i);
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

// Setar o bit 2 para habilitar Bus Mastering
void enable_bus_mastering(pci_device_t device) {
    uint32_t command_reg = pci_config_read(device.bus, device.slot, device.func, PCI_COMMAND);
    command_reg |= 0x0004;
    pci_config_write(device.bus, device.slot, device.func, PCI_COMMAND, command_reg);
}

// Setar o bit 1 para habilitar Memory Space
void enable_memory_space(pci_device_t device) {
    uint32_t command_reg = pci_config_read(device.bus, device.slot, device.func, PCI_COMMAND);
    command_reg |= 0x0002;  
    pci_config_write(device.bus, device.slot, device.func, PCI_COMMAND, command_reg);
}

// Setar o bit 0 para habilitar I/O Space
void enable_io_space(pci_device_t device) {
    uint32_t command_reg = pci_config_read(device.bus, device.slot, device.func, PCI_COMMAND);
    command_reg |= 0x0001;
    pci_config_write(device.bus, device.slot, device.func, PCI_COMMAND, command_reg);
}

void pci_device_debug(pci_device_t device) {
    printf("Vendor_id = 0x%04x\n", device.vendor_id);
    printf("Device_id = 0x%04x\n", device.device_id);
    printf("Bus  = 0x%02x\n", device.bus);
    printf("Slot = 0x%02x\n", device.slot);
    printf("Func = 0x%02x\n", device.func);
    for (int i = 0; i < 6; i++) {
        printf("Bar%d = 0x%08x\n", i, device.bars.bar[i]);
    }
    printf("Irq  = 0x%02x\n", device.irq);
}

void pci_debug_all() {
    for (int i = 0; i < devices_size; i++) {
        printf("Vendor_id = 0x%04x | Device_id = 0x%04x\n", devices[i].vendor_id, devices[i].device_id);
    }
}

uint8_t pci_read_irq(pci_device_t device) {
    uint32_t irq_reg = pci_config_read(device.bus, device.slot, device.func, 0x3C);
    return (uint8_t)(irq_reg & 0xFF);
}
