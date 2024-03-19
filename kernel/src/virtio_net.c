#include "../include/virtio_net.h"
#include "../include/pci.h"
#include "../include/asm.h"
#include "../include/sys.h"
#include "../../libc/include/string.h"
#include "../../libc/include/printf.h"
#include "../../libc/include/malloc.h"
#include "../../libc/include/time.h"

static virtio_net_device virtio_net;




// int virtio_receive_frame(uint8_t* buffer, uint32_t length) {
//     virt_queue* rx = &virtio_net.rx;

//     uint16_t i = rx->next_buffer;

//     if (rx->desc[i].flags == 0) {
//         return -1;
//     }

//     memcpy(buffer, (void*) rx->desc[i].addr, rx->desc[i].len);
//     rx->desc[i].flags = 0;
//     rx->desc[i].addr = 0;
//     rx->desc[i].len = 0;
//     rx->desc[i].next = 0;

//     rx->used->ring[rx->used->idx % rx->queue_size].id = i;
//     rx->used->ring[rx->used->idx % rx->queue_size].len = rx->desc[i].len;
//     rx->used->idx++;

//     rx->next_buffer = (i + 1) % rx->queue_size;

//     outw(virtio_net.io_address + 0x10, rx->used->idx);

//     return 0;
// }


int virtio_net_init() {
    uint8_t *buf;
    virtio_device virtio_device;
    int err = virtio_init(&virtio_device);
    if (err) goto err1;


    virtio_net.vendor_id = virtio_device.vendor_id;
    virtio_net.device_id = virtio_device.device_id;
    virtio_net.io_address = virtio_device.bars.bar[0] & ~0x3;
    virtio_net.irq = virtio_device.irq;
    virtio_net.queue_n = virtio_device.queue_n;
    virtio_net.mac_address = 0;
    virtio_net.rx = virtio_device.queue[0];
    virtio_net.tx = virtio_device.queue[1];

    
    // check if both queues were found.
    if (virtio_net.rx.buffer == 0 || virtio_net.tx.buffer == 0) goto err2;

    // Get MAC address
    uint64_t tempq = 0;
    for (int i = 0; i < 6; i++) {
        tempq = (tempq << 8) | inb(virtio_net.io_address + VIRTQ_BAR0_DEVICE_CONFIG + i);
    }
    virtio_net.mac_address = tempq;

err1: return err;
err2: return printf("device queue not found\n"), ERR_DEVICE_BAD_CONFIGURATION;
}

int virtio_init(virtio_device *virtio) {
    // 1 Get the pci addresses and reset the device if it's previously configured
    pci_device_t *pci_device;
    if (!pci_get_device(VIRTIO_VENDOR_ID, VIRTIO_DEVICE_ID, pci_device)) {
        pci_scan_bus();
    }

    // Try again
    if (!pci_get_device(VIRTIO_VENDOR_ID, VIRTIO_DEVICE_ID, pci_device)) goto err1;

    virtio->vendor_id = pci_device->vendor_id;
    virtio->device_id = pci_device->device_id;
    virtio->bars = pci_device->bars;
    virtio->irq = pci_device->irq;
    virtio->queue_n = 0;
    
    uint32_t bar0_address = virtio->bars.bar[0] & ~0x3;

    // 2 Set the ACKNOWLEDGE status bit: the guest OS has noticed the device.
    outb(bar0_address + VIRTQ_BAR0_STATUS, VIRTIO_ACKNOWLEDGE);

    // 3 Set the DRIVER status bit: the guest OS knows how to drive the device.
    outb(bar0_address + VIRTQ_BAR0_STATUS, VIRTIO_ACKNOWLEDGE | VIRTIO_DRIVER);

    // 4 Read device feature bits, and write the subset of feature bits understood by the OS and driver to the device. 
    uint32_t features = inl(bar0_address + VIRTQ_BAR0_DEVICE_FEATURES); 
    negotiate(&features);
    outl(bar0_address + VIRTQ_BAR0_DRIVER_FEATURES, features);

    // 5 Set the FEATURES_OK status bit. The driver MUST NOT accept new feature bits after this step.
    outb(bar0_address + VIRTQ_BAR0_STATUS, VIRTIO_ACKNOWLEDGE | VIRTIO_DRIVER | VIRTIO_FEATURES_OK);

    // 6 Re-read device status to ensure the FEATURES_OK bit is still set
    if ((inb(bar0_address + VIRTQ_BAR0_STATUS) & VIRTIO_FEATURES_OK) == 0) goto err2;

    


    // 7 Perform device-specific setup
    virtio_init_queues(virtio, bar0_address);

    // 8 Set the DRIVER_OK status bit. At this point the device is “live”.
    outb(bar0_address + VIRTQ_BAR0_STATUS, VIRTIO_ACKNOWLEDGE | VIRTIO_DRIVER | VIRTIO_FEATURES_OK | VIRTIO_DRIVER_OK);

    return 0;
err1:
    return printf("device not found\n"), ERR_DEVICE_NOT_FOUND;
err2:
    return printf("Features not accepted\n"), ERR_CONFIG_NOT_ACCEPTED;
}

void negotiate(uint32_t *features) {
    // before = 0111_1001_1011_1111_1000_0000_0110_0100
    // after  = 0111_1001_0001_0001_0000_0000_0110_0100
    DISABLE_FEATURE(*features, VIRTIO_MRG_RXBUF);
    DISABLE_FEATURE(*features, VIRTIO_CTRL_VQ);
    DISABLE_FEATURE(*features, VIRTIO_CTRL_RX);
    DISABLE_FEATURE(*features, VIRTIO_CTRL_VLAN);
    DISABLE_FEATURE(*features, VIRTIO_GUEST_ANNOUNCE);
    DISABLE_FEATURE(*features, VIRTIO_CTRL_MAC_ADDR);

    ENABLE_FEATURE(*features, VIRTIO_MQ);
    //Verify which queue layaout is supported
    if (HAS_FEATURE(*features, VIRTIO_MQ)) {
        printf("Split queues supported\n");
    }

}

void virtio_init_queues(virtio_device *virtio, uint32_t bar0_address) {
    uint16_t q_addr = -1;
    uint16_t size   = -1;

    while (size != 0)
    {
        q_addr++;
        // Write the queue address that we want to access
        outw(bar0_address + VIRTQ_BAR0_QUEUE_SELECT, q_addr);
        // Now read the size. The size is not the byte size but rather the element count.
        size = inw(bar0_address + VIRTQ_BAR0_QUEUE_SIZE);
        if (size > 0) virtio_init_queue(virtio, bar0_address, q_addr, size);
    }

    virtio->queue_n = q_addr;
}

void virtio_init_queue(virtio_device *virtio, uint32_t bar0_address, uint16_t i, uint16_t queue_n) {
    virt_queue* vq = &virtio->queue[i];
    memset(vq, 0, sizeof(virt_queue));

    // Create virtqueue memory
    uint32_t sizeof_descriptors =  queue_n * sizeof(virtq_desc);
    uint32_t sizeof_queue_available =  queue_n * sizeof(uint16_t) + (2*sizeof(uint16_t));
    uint32_t sizeof_queue_used = queue_n * sizeof(virtq_used_item) + (2*sizeof(uint16_t));
    

    uint32_t totalSize = sizeof_descriptors + sizeof_queue_available;


    //Align the size to be multiple of 4096
    totalSize = (totalSize + 0xFFF) & ~0xFFF;

    totalSize = (totalSize + sizeof_queue_used + 0xFFF) & ~0xFFF;

    // Alloc the queue
    void* buf = (void*)malloc(totalSize * 2);
    memset(buf, 0, totalSize);

    void * start = (void*)(((uint32_t)buf + 0xFFF) & ~0xFFF);


    // Configure the queue
    vq->buffer = (uint8_t*) start;
    vq->desc = (virtq_desc*) start;
    vq->available = (virtq_avail*) (start + sizeof_descriptors);
    vq->used = (virtq_used*) (start + ((sizeof_descriptors + sizeof_queue_available + 0xFFF) & ~0xFFF));
    vq->queue_size = queue_n;
    vq->buffer_size = totalSize;

    // The device stores only pages, not physical addresses
    // Inform the device the address
    outl(bar0_address + 0x08,  ((uint32_t)start) >> 12);
    
    printf("Address: %x\n", (uint32_t)start);
    printf("Stored address: %x\n", inl(bar0_address + 0x08) << 12);
    
    //TODO: Implement interrupt handling
    vq->available->flags = 0; // Do not trigger an interrupt when the queue is empty
}

uint64_t virtio_net_mac() {
    return virtio_net.mac_address;
}


int virtio_send_frame(uint8_t* buffer, uint32_t length) {
    virt_queue* tx = &virtio_net.tx;

    uint16_t i = tx->desc_idx;


    //TODO: TEST
    //Select queue
    outw(virtio_net.io_address + VIRTQ_BAR0_QUEUE_SELECT, 0);
    printf("Queue selected: %d\n", inw(virtio_net.io_address + VIRTQ_BAR0_QUEUE_SELECT));
    //Get the queue size
    uint16_t size = inw(virtio_net.io_address + VIRTQ_BAR0_QUEUE_SIZE);
    printf("Queue size: %d\n", size);
    //Get the queue address
    uint32_t addr = inl(virtio_net.io_address + VIRTQ_BAR0_QUEUE_ADDRESS);
    //Set the queue address
    outl(virtio_net.io_address + VIRTQ_BAR0_QUEUE_ADDRESS, (uint32_t) tx->buffer >> 12);
    //Status
    printf("Status: %b\n", inb(virtio_net.io_address + VIRTQ_BAR0_STATUS));

    printf("Address real / stored: %x / %x\n", (uint32_t) tx->buffer, inl(virtio_net.io_address + VIRTQ_BAR0_QUEUE_ADDRESS) << 12);

    if (tx->desc[i].flags == 0) {
        tx->desc[i].addr = (uint32_t) buffer;
        tx->desc[i].len = length;
        tx->desc[i].flags = 0x2; // Set the flag to 2 to indicate that the buffer is ready to be sent
        tx->desc_idx = (i + 1) % tx->queue_size;


        tx->available->ring[tx->available->idx % tx->queue_size] = i;
        tx->available->idx++;


        //dump virtio registers
        for (int i = 0; i<= 0x13; i++) {
            printf("%x %2x %4x %8x \n", i, inb(virtio_net.io_address + i), inw(virtio_net.io_address + i), inl(virtio_net.io_address + i));
            milisleep(300);
        }
    

        // Notify the device
        outl(virtio_net.io_address + 0x10, 0); // Notify the device that there is a new buffer to be sent
        


        printf("Used idx: %d\n", tx->used->idx);


        printf("\nDescriptor: \n");
        //dump memory
        // for (int i = 0; i < tx->buffer_size; i++) {
        //     if (i + (void *)tx->buffer == (void*) tx->available)
        //         printf("\nAvailable: \n");
        //     else if (i + (void *)tx->buffer == (void*) tx->used)
        //         printf("\nUsed: \n");


        //     if (tx->buffer[i] != 0)
        //     printf("%x", tx->buffer[i]);
        //      else if (tx->buffer[i] == 0 && tx->buffer[i+1] != 0) {
        //          printf(" ");
        //     }

        // }
        printf("\n");
    
    } else {
       return -1;
    }

    return 0;
}
