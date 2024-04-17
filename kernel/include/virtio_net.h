#pragma once

#include "../../libc/include/types.h"
#include "../include/pci.h"

typedef long long int64_t;
typedef unsigned long long uint64_t;

// Documentation:
// See: https://ozlabs.org/~rusty/virtio-spec/virtio-0.9.5.pdf
// See: https://docs.oasis-open.org/virtio/virtio/v1.3/virtio-v1.3.pdf
// See: http://www.dumais.io/index.php?article=aca38a9a2b065b24dfa1dee728062a12

#define VIRTIO_VENDOR_ID 0x1AF4
#define VIRTIO_DEVICE_ID 0x1000

#define CCW_CMD_VDEV_RESET 0x33

// Feature bits
#define VIRTIO_CSUM           0  // Device handles packets with partial checksum. This “checksum offload” is a common feature on modern network cards.
#define VIRTIO_GUEST_CSUM     1  // Driver handles packets with partial checksum.
#define VIRTIO_CTRL_G_O       2  // VIRTIO_NET_F_CTRL_GUEST_OFFLOADS - Control channel offloads reconfiguration support.
#define VIRTIO_MTU            3  // Device maximum MTU reporting is supported. 
#define VIRTIO_MAC            5  // Device has given MAC address.
#define VIRTIO_GUEST_TSO4     7  // Driver can receive TSOv4.                  == Requires VIRTIO_GUEST_CSUM.
#define VIRTIO_GUEST_TSO6     8  // Driver can receive TSOv6.                  == Requires VIRTIO_GUEST_CSUM.
#define VIRTIO_GUEST_ECN      9  // Driver can receive TSO with ECN.           == Requires VIRTIO_GUEST_TSO4 or VIRTIO_GUEST_TSO6.
#define VIRTIO_GUEST_UFO      10 // Driver can receive UFO.                    == Requires VIRTIO_GUEST_CSUM.
#define VIRTIO_HOST_TSO4      11 // Device can receive TSOv4.                  == Requires VIRTIO_CSUM.
#define VIRTIO_HOST_TSO6      12 // Device can receive TSOv6.                  == Requires VIRTIO_CSUM.
#define VIRTIO_HOST_ECN       13 // Device can receive TSO with ECN.           == Requires VIRTIO_HOST_TSO4 or VIRTIO_HOST_TSO6.
#define VIRTIO_HOST_UFO       14 // Device can receive UFO.                    == Requires VIRTIO_CSUM.
#define VIRTIO_MRG_RXBUF      15 // Driver can merge receive buffers.
#define VIRTIO_STATUS         16 // Configuration status field is available.
#define VIRTIO_CTRL_VQ        17 // Control channel is available.
#define VIRTIO_CTRL_RX        18 // Control channel RX mode support.           == Requires VIRTIO_CTRL_VQ.
#define VIRTIO_CTRL_VLAN      19 // Control channel VLAN filtering.            == Requires VIRTIO_CTRL_VQ.
#define VIRTIO_GUEST_ANNOUNCE 21 // Driver can send gratuitous packets.        == Requires VIRTIO_CTRL_VQ.
#define VIRTIO_MQ             22 // Device supports multiqueue w. auto receive steering. == Requires VIRTIO_CTRL_VQ.
#define VIRTIO_CTRL_MAC_ADDR  23 // Set MAC address through control channel.   == Requires VIRTIO_CTRL_VQ.
#define VIRTIO_RSC_EXT        61 // Device can process duplicated ACKs and report number of coalesced segments and duplicated ACKs == Requires VIRTIO_HOST_TSO4 or VIRTIO_HOST_TSO6.
#define VIRTIO_STANDBY        62 // Device may act as a standby for a primary device with the same MAC address.

#define DISABLE_FEATURE(v,feature)  v &= ~(1<<feature)
#define ENABLE_FEATURE(v,feature)   v |=  (1<<feature)

#define VIRTIO_ACKNOWLEDGE        1
#define VIRTIO_DRIVER_LOADED      2
#define VIRTIO_DRIVER_READY       4
#define VIRTIO_FEATURES_OK        8
#define VIRTIO_DEVICE_NEEDS_RESET 64
#define VIRTIO_DEVICE_ERROR       40
#define VIRTIO_DRIVER_FAILED      80
#define VIRTIO_FAILED             128

#define DEVICE_FEATURES 0x00
#define GUEST_FEATURES  0x04
#define QUEUE_ADDRESS   0x08
#define QUEUE_SIZE      0x0C
#define QUEUE_SELECT    0x0E
#define QUEUE_NOTIFY    0x10
#define DEVICE_STATUS   0x12
#define ISR_STATUS      0x13
#define DEVICE_CONFIG   0x14

#define TOTAL_SECTOR_COUNT    14
#define MAXIMUM_SEGMENT_SIZE  1C
#define MAXIMUM_SEGMENT_COUNT 20
#define CYLINDER_COUNT        24
#define HEAD_COUNT            26
#define SECTOR_COUNT          27
#define BLOCK_LENGTH          28

/* This marks a buffer as continuing via the next field. */
#define VIRTQ_DESC_F_NEXT       1
/* This marks a buffer as write-only (otherwise read-only). */
#define VIRTQ_DESC_F_WRITE      2
/* This means the buffer contains a list of buffer descriptors. */
#define VIRTQ_DESC_F_INDIRECT   4

#define NET_PACKET_SIZE 1514

// Buffers[QueueSIze]
typedef struct {
    uint64_t addr;  // 64-bit address of the buffer on the guest machine.
    uint32_t len;   // 32-bit length of the buffer.
    uint16_t flags; // 1: Next field contains linked buffer index;  
                    // 2: Buffer is write-only (clear for read-only).
                    // 4: Buffer contains additional buffer addresses.
    uint16_t next;  // If flag is set, contains index of next buffer in chain.
} vring_desc;

// Available
typedef struct {
    uint16_t flags;       // 1: Do not trigger interrupts.
    uint16_t idx;         // Index of the next ring index to be used.  (Last available ring buffer index+1)
    uint16_t ring[];      // [QueueSize] List of available buffer indexes from the Buffers array above.
    //uint16_t used_event;  // Only used if VIRTIO_F_EVENT_IDX was negotiated
} vring_avail;

//  Ring
typedef struct {
    uint32_t id;   // Index of the used buffer in the Buffers array above.
    uint32_t len;  // Total bytes written to buffer.
} vring_used_elem;

// Used
typedef struct {
    uint16_t flags;         // 1: Do not notify device when buffers are added to available ring.
    uint16_t idx;           // Index of the next ring index to be used.  (Last used ring buffer index+1)
    vring_used_elem ring[]; // [QueueSize]
    //uint16_t avail_event;   // Only used if VIRTIO_F_EVENT_IDX was negotiated
} vring_used;

typedef struct {
    uint32_t num;
    vring_desc  *desc;
    vring_avail *avail;
    vring_used  *used;
    uint16_t desc_next_idx;
} vring;

#define ALIGN(x) (((x)+4095) & ~4095)
static inline unsigned vring_size(unsigned int qsz) {
    return ALIGN(sizeof(vring_desc) * qsz + sizeof(uint16_t) * (2+qsz))
        + ALIGN(sizeof(vring_used_elem) * qsz);
}

static inline void vring_init(vring *vr, unsigned int num, void *p, uint32_t align){
	vr->num = num;
	vr->desc = (vring_desc*)p;
	vr->avail = (vring_avail *)((char *)p + num * sizeof(vring_desc));
	vr->used = (vring_used*)(((size_t)&vr->avail->ring[num] + align - 1) & ~(align - 1));
}

static inline int vring_need_event(uint16_t event_idx, uint16_t new_idx, uint16_t old_idx){
    return (uint16_t)(new_idx - event_idx - 1) < (uint16_t)(new_idx - old_idx);
}

typedef struct {
    uint16_t   vendor_id;
    uint16_t   device_id;
    pci_bars   bars;
    uint8_t    irq;
    vring      queue[16];
    uint64_t   queue_n;
} virtio_device;

typedef struct {
    uint16_t   vendor_id;
    uint16_t   device_id;
    uint32_t   io_address;
    uint8_t    irq;
    vring      queue[2];
    uint64_t   mac_address;
} virtio_net_device;

#define VIRTIO_NET_HDR_F_NEEDS_CSUM 1
#define VIRTIO_NET_HDR_GSO_NONE 0
#define VIRTIO_NET_HDR_GSO_TCPV4 1
#define VIRTIO_NET_HDR_GSO_UDP 3
#define VIRTIO_NET_HDR_GSO_TCPV6 4
#define VIRTIO_NET_HDR_GSO_ECN 0x80

struct virtio_net_hdr
{
    uint8_t flags;
    uint8_t gso_type;
    uint16_t hdr_len;
    uint16_t gso_size;
    uint16_t csum_start;
    uint16_t csum_offset;
};

int virtio_net_init();
void virtio_send_descriptor(virtio_net_device* dev, uint8_t queue_index, vring_desc buffers[], int count);

extern virtio_net_device vn;