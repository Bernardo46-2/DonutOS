#include "../include/virtio_net.h"
#include "../include/pci.h"
#include "../include/asm.h"
#include "../include/sys.h"
#include "../../libc/include/string.h"
#include "../../libc/include/printf.h"
#include "../../libc/include/malloc.h"
#include "../../libc/include/time.h"

virtio_net_device vn;

void virtio_init_queues(virtio_device *virtio_pci, uint32_t bar0_address);
int virtio_init_queue(virtio_device *virtio, uint32_t bar0_address, uint16_t i, uint16_t queue_size);
void negotiate(uint32_t *features);
int virtio_init(virtio_device *virtio);
int virtio_net_init();


int virtio_net_init() {
    uint8_t *buf;
    virtio_device virtio_device;
    int err = virtio_init(&virtio_device);
    if (err) return err;

    virtio_net_device virtio_net = {
        .vendor_id = virtio_device.vendor_id,
        .device_id = virtio_device.device_id,
        .io_address = virtio_device.bars.bar[0] & ~0x3,
        .irq = virtio_device.irq,
    };
    virtio_net.queue[0] = virtio_device.queue[0];
    virtio_net.queue[1] = virtio_device.queue[1];

    if (virtio_net.queue[0].desc == 0 || virtio_net.queue[1].desc == 0) {
        int err = ERR_DEVICE_BAD_CONFIGURATION;
        
        if (err) printf("Error %d, while trying to start the network device: ", err);
        switch (err)
        {
        case 0:
            break;
        case ERR_DEVICE_BAD_CONFIGURATION:
            printf("ERR_DEVICE_BAD_CONFIGURATION\n");
            break;
        case ERR_CONFIG_NOT_ACCEPTED:
            printf("ERR_CONFIG_NOT_ACCEPTED\n");
            break;
        case ERR_DEVICE_NOT_FOUND:
            printf("ERR_DEVICE_NOT_FOUND\n");
            break;
        }
        
        return ERR_DEVICE_BAD_CONFIGURATION;
    }


    // Get MAC address
    uint64_t tempq = 0;
    for (int i = 0; i < 6; i++) {
        tempq = (tempq << 8) | inb(virtio_net.io_address + DEVICE_CONFIG + i);
    }
    virtio_net.mac_address = tempq;

    vn = virtio_net;

    //Alloc RX buffers
    const int rx_desc_size = 20;
    vring_desc *rx_desc = (vring_desc*) calloc(rx_desc_size, sizeof(vring_desc));
    for (int i = 0; i < rx_desc_size; i++) {
        uint8_t *desc = (uint8_t*)malloc(NET_PACKET_SIZE);
        rx_desc[i].addr = (uint32_t)desc;
        rx_desc[i].len = NET_PACKET_SIZE;
        rx_desc[i].flags = VIRTQ_DESC_F_WRITE;
    }
    virtio_send_descriptor(&vn, 0, rx_desc, rx_desc_size);

    return 0;

    //return printf("device queue not found\n"), ERR_DEVICE_BAD_CONFIGURATION;
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
    
    //No device found
    if (!pci_get_device(VIRTIO_VENDOR_ID, VIRTIO_DEVICE_ID, pci_device)) 
        return ERR_DEVICE_NOT_FOUND;


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
    if ((inb(bar0_address + DEVICE_STATUS) & VIRTIO_FEATURES_OK) == 0) 
        return ERR_CONFIG_NOT_ACCEPTED;

    // 7 Perform device-specific setup
    virtio_init_queues(virtio, bar0_address);

    // 8 Set the DRIVER_OK status bit. At this point the device is “live”.
    outb(bar0_address + DEVICE_STATUS, VIRTIO_ACKNOWLEDGE | VIRTIO_DRIVER_LOADED | VIRTIO_FEATURES_OK | VIRTIO_DRIVER_READY);

    return 0;
}

void virtio_init_queues(virtio_device *virtio, uint32_t bar0_address) {
    uint16_t q_addr = 0;
    uint16_t size   = -1;

    while (q_addr < 2) // hardfix, should use (size != 0)
    {
        // Write the queue address that we want to access
        outw(bar0_address + QUEUE_SELECT, q_addr);
        // Now read the size. The size is not the byte size but rather the element count.
        size = inw(bar0_address + QUEUE_SIZE);
        if (size > 0) virtio_init_queue(virtio, bar0_address, q_addr, size);
        q_addr++;
    }

    virtio->queue_n = q_addr;
}

int virtio_init_queue(virtio_device *virtio, uint32_t bar0_address, uint16_t i, uint16_t queue_size) {
    uint32_t size = vring_size(queue_size);
    vring* vr = calloc(sizeof(vring), 1);
    void* p = (void*)(ALIGN((size_t)calloc(size + 4095, 1)));
    if (p == NULL) return ERR_MEMORY_ALLOCATION_ERROR;

    vring_init(vr, queue_size, p, 4096);
    outl(bar0_address + QUEUE_ADDRESS, ((size_t)vr->desc >> 12));
    vr->avail->flags = 0;
    virtio->queue[i] = *vr;

    return 0;
}

void virtio_enable_interrupts(vring* vq){
    vq->used->flags = 0;
}

void virtio_disable_interrupts(vring* vq){
    vq->used->flags = 1;
}

void virtio_send_descriptor(virtio_net_device* dev, uint8_t queue_index, vring_desc buffers[], int count)
{

    // Get the queue
    vring* vq = &dev->queue[queue_index];

    uint16_t desc_index = vq->desc_next_idx;
    uint16_t next_buffer_index;

    vring_desc *buf = &vq->desc[desc_index];

    vq->avail->ring[vq->avail->idx % vq->num] = desc_index;
    for (int i = 0; i < count; i++) {

        next_buffer_index = (desc_index+1) % vq->num;

        vq->desc[desc_index].flags = buffers[i].flags;

        // Set the next flag if there are more buffers
        if (i != (count-1)) vq->desc[desc_index].flags |= VIRTQ_DESC_F_NEXT;

        vq->desc[desc_index].next = next_buffer_index;
        vq->desc[desc_index].len = buffers[i].len;
        
        
        vq->desc[desc_index].addr = buffers[i].addr;
        desc_index = next_buffer_index;
    }
    vq->desc_next_idx = desc_index;

    vq->avail->idx++;

    
    outw(dev->io_address + QUEUE_NOTIFY, queue_index);
}
