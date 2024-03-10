#include "../include/virtio_net.h"
#include "../include/pci.h"
#include "../include/asm.h"
#include "../../libc/include/printf.h"

int virtio_init() {
    // Find a PCI device with Vendor/Device 1af4/1000.
    pci_device_t virtio_pci;
    if (!pci_get_device(VIRTIO_VENDOR_ID, VIRTIO_DEVICE_ID, &virtio_pci)) return printf("device not found\n");
    pci_device_debug(virtio_pci);

    // Get BAR0 for that device.
    uint32_t bar0_address = virtio_pci.bars.bar[0] & ~0x3; // Extract address from E/S

    // Tell the device that we have noticed it
    outb(bar0_address + 0x12, VIRTIO_ACKNOWLEDGE);

    // Tell the device that we will support it.
    outb(bar0_address + 0x12, VIRTIO_ACKNOWLEDGE | VIRTIO_DRIVER);

    // read features offered by device
    printf("RETORNO = %0x\n", inl(bar0_address + 0x00));

    

    

    // Set up buffer chains in memory for both Queues.
    // Write 0x00010020 to BAR0+0x04 to set the Guest Feature flags.
    // Write 0x07 to BAR0+0x12 to Notify the host that the driver is ready.
    // Write 0x00 to BAR0+0x10 to Notify the host that the Queue 0 has been modified by the driver.

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
