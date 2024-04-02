#include "../include/virtio_net.h"
#include "../include/pci.h"
#include "../include/asm.h"
#include "../include/sys.h"
#include "../../libc/include/string.h"
#include "../../libc/include/printf.h"
#include "../../libc/include/malloc.h"
#include "../../libc/include/time.h"

void virtio_init_queues(virtio_device *virtio_pci, uint32_t bar0_address);
void virtio_init_queue(virtio_device *virtio, uint32_t bar0_address, uint16_t i, uint16_t queue_size);
void negotiate(uint32_t *features);
int virtio_init(virtio_device *virtio);
int virtio_net_init();

int virtio_net_init() {
    uint8_t *buf;
    virtio_device virtio_device;
    int err = virtio_init(&virtio_device);
    if (err) goto err1;

    virtio_net_device virtio_net = {
        .vendor_id = virtio_device.vendor_id,
        .device_id = virtio_device.device_id,
        .io_address = virtio_device.bars.bar[0] & ~0x3,
        .irq = virtio_device.irq,
        .queue_n = virtio_device.queue_n,
        .mac_address = 0,
    };

    // Get MAC address
    uint64_t tempq = 0;
    for (int i = 0; i < 6; i++) {
        tempq = (tempq << 8) | inb(virtio_net.io_address + 0x14 + i);
    }
    virtio_net.mac_address = tempq;

err1: return err;
err2: return printf("device queue not found\n"), ERR_DEVICE_BAD_CONFIGURATION;
}

// before = 0111_1001_1011_1111_1000_0000_0110_0100
// after  = 0111_1001_0001_0001_0000_0000_0110_0100
void negotiate(uint32_t *features) {
    DISABLE_FEATURE(*features, VIRTIO_MRG_RXBUF);
    DISABLE_FEATURE(*features, VIRTIO_CTRL_VQ);
    DISABLE_FEATURE(*features, VIRTIO_CTRL_RX);
    DISABLE_FEATURE(*features, VIRTIO_CTRL_VLAN);
    DISABLE_FEATURE(*features, VIRTIO_GUEST_ANNOUNCE);
    DISABLE_FEATURE(*features, VIRTIO_CTRL_MAC_ADDR);
}

int virtio_init(virtio_device *virtio) {
    // 1 Get the pci addresses and reset the device if it's previously configured
    pci_device_t *pci_device;
    if (!pci_get_device(VIRTIO_VENDOR_ID, VIRTIO_DEVICE_ID, pci_device)) goto err1;
    virtio->vendor_id = pci_device->vendor_id;
    virtio->device_id = pci_device->device_id;
    virtio->bars = pci_device->bars;
    virtio->irq = pci_device->irq;
    virtio->queue_n = 0;

    
    uint32_t bar0_address = virtio->bars.bar[0] & ~0x3;

    // 2 Set the ACKNOWLEDGE status bit: the guest OS has noticed the device.
    outb(bar0_address + DEVICE_STATUS, VIRTIO_ACKNOWLEDGE);

    // 3 Set the DRIVER status bit: the guest OS knows how to drive the device.
    outb(bar0_address + DEVICE_STATUS, VIRTIO_ACKNOWLEDGE | VIRTIO_DRIVER_LOADED);

    // 4 Read device feature bits, and write the subset of feature bits understood by the OS and driver to the device. 
    uint32_t features = inl(bar0_address + DEVICE_FEATURES); 
    negotiate(&features);
    outl(bar0_address + GUEST_FEATURES, features);

    // 5 Set the FEATURES_OK status bit. The driver MUST NOT accept new feature bits after this step.
    outb(bar0_address + DEVICE_STATUS, VIRTIO_ACKNOWLEDGE | VIRTIO_DRIVER_LOADED | VIRTIO_FEATURES_OK);

    // 6 Re-read device status to ensure the FEATURES_OK bit is still set
    if ((inb(bar0_address + DEVICE_STATUS) & VIRTIO_FEATURES_OK) == 0) goto err2;

    // 7 Perform device-specific setup
    virtio_init_queues(virtio, bar0_address);

    // 8 Set the DRIVER_OK status bit. At this point the device is “live”.
    outb(bar0_address + DEVICE_STATUS, VIRTIO_ACKNOWLEDGE | VIRTIO_DRIVER_LOADED | VIRTIO_FEATURES_OK | VIRTIO_DRIVER_READY);

    return 0;
err1:
    return printf("device not found\n"), ERR_DEVICE_NOT_FOUND;
err2:
    return printf("Features not accepted\n"), ERR_CONFIG_NOT_ACCEPTED;
}

void virtio_init_queues(virtio_device *virtio, uint32_t bar0_address) {
    uint16_t q_addr = -1;
    uint16_t size   = -1;

    while (size != 0)
    {
        q_addr++;
        // Write the queue address that we want to access
        outw(bar0_address + QUEUE_SELECT, q_addr);
        // Now read the size. The size is not the byte size but rather the element count.
        size = inw(bar0_address + QUEUE_SIZE);
        if (size > 0) virtio_init_queue(virtio, bar0_address, q_addr, size);
    }

    virtio->queue_n = q_addr;
}



void virtio_init_queue(virtio_device *virtio, uint32_t bar0_address, uint16_t i, uint16_t queue_n) {
    // virt_queue* vq = &virtio->queue[i];
    // memset(vq, 0, sizeof(virt_queue));

    // // Create virtqueue memory
    // uint32_t sizeof_buffers = (sizeof(queue_buffer) * queue_n);
    // uint32_t sizeof_queue_available = (2 * sizeof(uint16_t)) + (queue_n * sizeof(uint16_t));
    // uint32_t sizeof_queue_used = (2 * sizeof(uint16_t)) + (queue_n * sizeof(virtio_used_item));
    
    // // Make totalSize a multiple of 4096
    // uint32_t totalSize = sizeof_buffers + sizeof_queue_available + sizeof_queue_used;
    // totalSize += 4095;
    // totalSize &= ~4095;

    // // Alloc the queue
    // uint8_t* buf = (uint8_t*)malloc(totalSize);
    // memset(buf, 0, totalSize);

    // // Configure the queue
    // vq->base_address = (uint64_t)buf;
    // vq->available = (virtio_available*)&buf[sizeof_buffers];
    // vq->used = (virtio_used*)&buf[(sizeof_buffers + sizeof_queue_available + 4095) & ~4095];
    // vq->next_buffer = 0;
    // vq->lock = 0;

    // // Get the number of pages
    // // Note: This step may not be necessary if you are passing the address directly and the device supports this
    // uint32_t buf_page = ((uint64_t)vq->base_address) >> 12;
    // // Inform the device the page address
    // outl(bar0_address + 0x08, buf_page);
    
    // vq->available->flags = 0;
}
