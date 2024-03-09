#ifndef _PCI_H_
#define _PCI_H_

#include "../../libc/include/types.h"

typedef struct {
    uint16_t vendor_id;
    uint16_t device_id;
} pci_device_t;

void scan_pci_bus();

#endif
