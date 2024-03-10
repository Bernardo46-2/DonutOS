#include "../include/virtio_net.h"
#include "../include/pci.h"
#include "../../libc/include/printf.h"


int virtio_init() {
    pci_device_t virtio_pci;
    if (!pci_get_device(VIRTIO_VENDOR_ID, VIRTIO_DEVICE_ID, &virtio_pci)) {
        return printf("device not found\n");
    } else {
        printf(">> bus = 0x%x, device = 0x%x, func = 0x%x, vendor_id = 0x%x, device_id = 0x%x\n", 
        virtio_pci.bus , virtio_pci.device , virtio_pci.func , virtio_pci.vendor_id , virtio_pci.device_id);

        pci_read_bars(virtio_pci);
    }

    // 1 Reset the device.
    // 2 Set the ACKNOWLEDGE status bit: the guest OS has noticed the device.
    // 3 Set the DRIVER status bit: the guest OS knows how to drive the device.
    // 4 Read device feature bits, and write the subset of feature bits understood by the OS and driver to the device. During this step the driver MAY read (but MUST NOT write) the device-specific configuration fields to check that it can support the device before accepting it.
    // 5 Set the FEATURES_OK status bit. The driver MUST NOT accept new feature bits after this step.
    // 6 Re-read device status to ensure the FEATURES_OK bit is still set: otherwise, the device does not support our subset of features and the device is unusable.
    // 7 Perform device-specific setup, including discovery of virtqueues for the device, optional per-bus setup, reading and possibly writing the device’s virtio configuration space, and population of virtqueues.
    // 8 Set the DRIVER_OK status bit. At this point the device is “live”.
    return 1;
}
