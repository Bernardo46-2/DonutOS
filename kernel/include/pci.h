#pragma once

#include "../../libc/include/types.h"


#define PCI_CONFIG_ADDRESS 0xCF8 // Port to read/write the address
#define PCI_CONFIG_DATA    0xCFC // Port to read/write the data

#define PCI_VENDOR_ID 0x00
#define PCI_DEVICE_ID 0x02
#define PCI_COMMAND   0x04
#define PCI_STATUS    0x06
#define PCI_REVISION  0x08
#define PCI_PROG_IF   0x09
#define PCI_SUBCLASS  0x0A
#define PCI_CLASS     0x0B
#define PCI_CACHE_LINE_SIZE 0x0C
#define PCI_LATENCY_TIMER 0x0D
#define PCI_HEADER_TYPE 0x0E
#define PCI_BIST 0x0F
#define PCI_BAR0 0x10
#define PCI_INTERRUPT_LINE 0x3C
#define PCI_SECONDARY_BUS 0x19

#define PCI_HEADER_TYPE_MULTIFUNCTION 0x80

#define PCI_BAR_IO 0x01
#define PCI_BAR_MEM 0x00
#define PCI_BAR_MEM_PREFETCH 0x08

typedef struct {
    uint32_t bar[6];
} pci_bars;

typedef struct {
    uint8_t  bus;
    uint8_t  slot;
    uint8_t  func;
    uint16_t vendor_id;
    uint16_t device_id;
    pci_bars bars;
    uint8_t  irq;
} pci_device_t;

void pci_scan_bus();
uint32_t pci_read_bar(uint8_t bar, uint8_t slot, uint8_t func, uint8_t offset);
int pci_get_device(uint16_t vendor_id, uint16_t device_id, pci_device_t *target);
void pci_read_bars(pci_device_t pci_device, pci_bars *pci_bars);
void enable_bus_mastering(pci_device_t device);
void enable_memory_space(pci_device_t device);
void enable_io_space(pci_device_t device);
void pci_device_debug(pci_device_t device);
void pci_debug_all();
uint8_t pci_read_irq(pci_device_t device);
