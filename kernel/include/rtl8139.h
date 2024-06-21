#ifndef RTL8139_H
#define RTL8139_H

#include "../../libc/include/types.h"
#include "../include/pci.h"
#include "../include/isr.h"

#define MAC05   0x00 // size = 6
#define MAR07   0x08 // size = 8
#define RBSTART 0x30 // size = 4
#define CMD     0x37 // size = 1
#define IMR     0x3C // size = 2
#define ISR     0x3E // size = 2

#define CONFIG_1 0x52
#define CAPR     0x38

#define ROK 0x01  // Receive OK Interrupt: Bit 0
#define TOK 0x04  // Transmit OK Interrupt: Bit 2

#define RX_TOTAL_BUFFER_SIZE 9708  // 8192 + 16 + 1500 bytes
#define RX_BUFFER_SIZE 8192
#define RX_READ_POINTER_MASK (~3)
typedef struct {
    uint16_t vendor_id;
    uint16_t device_id;
    pci_bars bars;
    uint8_t irq;
    uint32_t io_address;
    uint8_t mac_addr[6];
} rtl_device;

extern long bytes_received;
void rtl8139_irq(regs_t* rs);
uint32_t get_io_address(pci_device_t device);
void rtl8139_init();
void printIP(uint8_t* ip, uint8_t isIPv6);
void rtl_printFrame(int i);
void rtl_print_buffer();
void rtl_print_buffer_size();
void read_mac_addr();
rtl_device rtl8139_get_status();
struct EthernetHeader* rtl8139_get_ethHeader(int i);

extern struct EthernetHeader* ethHeaders;
extern int firstHeader;
extern int nHeaders;




#endif
