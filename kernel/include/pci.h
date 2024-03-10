#ifndef _PCI_H_
#define _PCI_H_

#include "../../libc/include/types.h"

typedef struct {
    uint8_t  bus;
    uint8_t  device;
    uint8_t  func;
    uint16_t vendor_id;
    uint16_t device_id;
} pci_device_t;

void pci_scan_bus();
uint32_t pci_read_bar(uint8_t bar, uint8_t device, uint8_t func, uint8_t offset);
int pci_get_device(uint16_t vendor_id, uint16_t device_id, pci_device_t *target);
void pci_read_bars(pci_device_t pci_device);

#endif
