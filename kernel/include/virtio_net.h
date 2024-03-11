#ifndef _VIRTIO_H_
#define _VIRTIO_H_

#include "../../libc/include/types.h"
#include "../include/pci.h"

// Documentation:
// See: https://ozlabs.org/~rusty/virtio-spec/virtio-0.9.5.pdf
// See: http://docs.oasis-open.org/virtio/virtio/v1.0/cs04/virtio-v1.0-cs04.html
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
#define HAS_FEATURE(v,feature)      (v & (1<<feature))
#define PAGE_COUNT(x) ((x+0xFFF)>>12)
#define MIRROR_BIT 46
#define MIRROR(x) (((uint64_t)x)|(1LL<<MIRROR_BIT))
#define UNMIRROR(x) (((uint64_t)x)&~(1LL<<MIRROR_BIT))

#define VIRTIO_ACKNOWLEDGE        1
#define VIRTIO_DRIVER             2
#define VIRTIO_DRIVER_OK          4
#define VIRTIO_FEATURES_OK        8
#define VIRTIO_DEVICE_NEEDS_RESET 64
#define VIRTIO_FAILED             128

typedef long long int64_t;
typedef unsigned long long uint64_t;

typedef struct {
    uint64_t address;
    uint32_t length;
    uint16_t flags;
    uint16_t next;
} queue_buffer;

typedef struct {
    uint16_t flags;
    uint16_t index;
    uint16_t rings[];
} virtio_available;

typedef struct {
    uint32_t index;
    uint32_t length;
} virtio_used_item;

typedef struct {
    uint16_t         flags;
    uint16_t         index;
    virtio_used_item rings[];
} virtio_used;

typedef struct
{
    uint16_t queue_size;
    union
    {
        queue_buffer* buffers;
        uint64_t base_address;
    };
    virtio_available* available;
    virtio_used* used;
    uint16_t last_used_index;
    uint16_t last_available_index;
    uint8_t* buffer;
    uint32_t chunk_size;
    uint16_t next_buffer;
    uint64_t lock;
} virt_queue;

typedef struct {
    uint16_t   vendor_id;
    uint16_t   device_id;
    pci_bars   bars;
    uint8_t    irq;
    virt_queue queue[16];
    uint64_t   queue_n;
} virtio_device;

typedef struct {
    uint16_t   vendor_id;
    uint16_t   device_id;
    uint32_t   io_address;
    uint8_t    irq;
    virt_queue queue[3];
    uint64_t   queue_n;
    uint64_t   mac_address;
} virtio_net_device;

typedef struct
{
    uint8_t  flags;
    uint8_t  gso_type;
    uint16_t header_length;
    uint16_t gso_size;
    uint16_t checksum_start;
    uint16_t checksum_offset;
} net_header;

int virtio_net_init();

#endif
