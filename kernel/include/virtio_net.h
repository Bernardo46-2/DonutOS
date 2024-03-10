#ifndef _VIRTIO_H_
#define _VIRTIO_H_

// Documentation:
// See: https://ozlabs.org/~rusty/virtio-spec/virtio-0.9.5.pdf
// See: http://docs.oasis-open.org/virtio/virtio/v1.0/cs04/virtio-v1.0-cs04.html
// See: http://www.dumais.io/index.php?article=aca38a9a2b065b24dfa1dee728062a12

#define VIRTIO_VENDOR_ID 0x1AF4
#define VIRTIO_DEVICE_ID 0x1000

#define CCW_CMD_VDEV_RESET 0x33

// KRAIO
// Feature bits
// 0111_1001_1011_1111_1000_0000_0110_0100
#define VIRTIO_CSUM           0  // Device handles packets with partial checksum. This “checksum offload” is a common feature on modern network cards.
#define VIRTIO_GUEST_CSUM     1  // Driver handles packets with partial checksum.
#define VIRTIO_CTRL_G_O       2  // Control channel offloads reconfiguration support.
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
#define VIRTIO_CTRL_RX        18 // Control channel RX mode support.         == Requires VIRTIO_CTRL_VQ.
#define VIRTIO_CTRL_VLAN      19 // Control channel VLAN filtering.          == Requires VIRTIO_CTRL_VQ.
#define VIRTIO_GUEST_ANNOUNCE 21 // Driver can send gratuitous packets.        == Requires VIRTIO_CTRL_VQ.
#define VIRTIO_MQ             22 // Device supports multiqueue w. auto receive steering. == Requires VIRTIO_CTRL_VQ.
#define VIRTIO_CTRL_MAC_ADDR  23 // Set MAC address through control channel. == Requires VIRTIO_CTRL_VQ.
#define VIRTIO_RSC_EXT        61 // Device can process duplicated ACKs and report number of coalesced segments and duplicated ACKs == Requires VIRTIO_HOST_TSO4 or VIRTIO_HOST_TSO6.
#define VIRTIO_STANDBY        62 // Device may act as a standby for a primary device with the same MAC address.

#define VIRTIO_ACKNOWLEDGE        1
#define VIRTIO_DRIVER             2
#define VIRTIO_FAILED             128
#define VIRTIO_FEATURES_OK        8
#define VIRTIO_DRIVER_OK          4
#define VIRTIO_DEVICE_NEEDS_RESET 64

int virtio_init();

#endif
