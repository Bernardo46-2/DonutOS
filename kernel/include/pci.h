#ifndef _PCI_H_
#define _PCI_H_

#include "../../libc/include/types.h"

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
void pci_device_debug(pci_device_t device);

#endif
