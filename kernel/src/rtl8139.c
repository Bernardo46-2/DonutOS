#include "../include/rtl8139.h"
#include "../include/pci.h"
#include "../include/asm.h"
#include "../include/irq.h"
#include "../include/isr.h"
#include "../../libc/include/printf.h"
#include "../../libc/include/string.h"
#include "../../libc/include/malloc.h"
#include "../../libc/include/net.h"

// TODO
// fazer toda a sessao de `Transmitting Packets`

#define VENDOR_ID 0x10EC
#define DEVICE_ID 0x8139

uint8_t* rx_buffer;
uint32_t ioaddr;
uint8_t mac_addr[6];
uint32_t current_packet_ptr;
struct EthernetHeader ethHeader;

extern struct EthernetHeader process_received_packet(void* packet, uint16_t packet_length);

void receive_packet() {
    // Convertendo o ponteiro para o local atual do pacote
    uint16_t * t = (uint16_t*)(rx_buffer + current_packet_ptr);

    // O comprimento do pacote está no segundo uint16_t, pulando o cabeçalho
    uint16_t packet_length = *(t + 1);

    // Avançar para os dados do pacote (além do cabeçalho de 4 bytes)
    t += 2;

    // Alocação e cópia dos dados do pacote para processamento separado
    void * packet = malloc(packet_length);
    memcpy(packet, t, packet_length);

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////// AQUI COMECA A PILHA DE PROTOCOLOS ////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////
    // ethernet_handle_packet(packet, packet_length);
    ethHeader = process_received_packet(packet, packet_length);
    free(packet);

    // Atualizando o ponteiro de pacote considerando alinhamento
    current_packet_ptr = (current_packet_ptr + packet_length + 4 + 3) & RX_READ_POINTER_MASK;

    if(current_packet_ptr > RX_BUFFER_SIZE)
        current_packet_ptr -= RX_BUFFER_SIZE;

    // Atualizar o CAPR para informar ao hardware que o pacote foi processado
    outw(ioaddr + CAPR, current_packet_ptr - 0x10);
}



void rtl8139_init() {
    pci_device_t dev;
    pci_get_device(VENDOR_ID, DEVICE_ID, &dev);

    // PCI Bus Mastering
    enable_bus_mastering(dev);

    // Turning on the RTL8139
    ioaddr = get_io_address(dev);
    outb(ioaddr + CONFIG_1, 0x00);

    // Software Reset!
    outb(ioaddr + CMD, 0x10);
    while((inb(ioaddr + CMD) & 0x10) != 0){}

    // Init Receive buffer
    rx_buffer = (uint8_t*)malloc(RX_TOTAL_BUFFER_SIZE);
    outl(ioaddr + RBSTART, (size_t)rx_buffer); // send uint32_t memory location to RBSTART (0x30)

    // Set IMR + ISR
    outw(ioaddr + IMR, TOK | ROK); // Habilita IRQs para TOK e ROK apenas

    // Configura AB, AM, APM, AAP e WRAP=1
    outl(ioaddr + 0x44, 0xf | (1 << 7));

    // Sets the RE and TE bits high
    outb(ioaddr + CMD, 0x0C);

    // instalar IRQ
    irq_install(dev.irq, rtl8139_irq);

    read_mac_addr();
}

void rtl8139_irq(regs_t* rs) {
    uint16_t status = inw(ioaddr + ISR);
    outw(ioaddr + ISR, TOK | ROK);

    if (status & ROK) {
        receive_packet();
    } else if (status & TOK) {
        printf("pacote enviado!\n");
    }

    // Limpa as flags de interrupção escrevendo de volta ao ISR
}

void rtl_print_buffer() {
    for (int i = 0; i < 256; i++) {
        printf("%02x  ", rx_buffer[i]);
    }
    printf("\n");
}

void rtl_printFrame() {
    //ethernetHeader to be read here.
    // fazer if no EtherType do struct dnv pra descobri qual protocolo é
    // depois, printar o struct do pacote
}

void rtl_print_buffer_size() {
    int last = 0;

    for (int i = 0; i < RX_TOTAL_BUFFER_SIZE; i++) {
        if (rx_buffer[i] != 0) {
            last = i;
        }
    }

    // Imprime o último índice não-zero
    printf("buffer size = %d\n", last);
}

uint32_t get_io_address(pci_device_t device) {
    return device.bars.bar[0] & ~0x3;
}

void read_mac_addr() {
    uint32_t mac_part1 = inl(ioaddr + 0x00);
    uint16_t mac_part2 = inw(ioaddr + 0x04);
    mac_addr[0] = mac_part1 >> 0;
    mac_addr[1] = mac_part1 >> 8;
    mac_addr[2] = mac_part1 >> 16;
    mac_addr[3] = mac_part1 >> 24;
    mac_addr[4] = mac_part2 >> 0;
    mac_addr[5] = mac_part2 >> 8;

    // printf("MAC Address: %01x:%01x:%01x:%01x:%01x:%01x\n", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
}

struct EthernetHeader process_received_packet(void* packet, uint16_t packet_length) {
    // Parse Ethernet header
    struct EthernetHeader ethernetHeader;
    parseEthernetHeader((uint8_t*)packet, &ethernetHeader);

    // Check the EtherType to determine the protocol
    if (ethernetHeader.etherType == 0x0806) {
        // ARP packet
        parseARPHeader((uint8_t*)packet + sizeof(struct EthernetHeader), &ethernetHeader.protocol.arpHeader);
        // Process ARP packet
        // ...
    } else if (ethernetHeader.etherType == 0x0800) {
        // IPv4 packet
        parseIPv4Header((uint8_t*)packet + sizeof(struct EthernetHeader), &ethernetHeader.protocol.ipv4Header);
        if (ethernetHeader.protocol.ipv4Header.protocol == 0x06) {
            // TCP packet
            parseTCPHeader((uint8_t*)packet + sizeof(struct EthernetHeader) + sizeof(ethernetHeader.protocol.ipv4Header), &ethernetHeader.protocol.ipv4Header.transpProtocol.tcpHeader);
            // Process TCP packet
            // ...
        } else if (ethernetHeader.protocol.ipv4Header.protocol == 0x11) {
            // UDP packet
            parseUDPHeader((uint8_t*)packet + sizeof(struct EthernetHeader) + sizeof(ethernetHeader.protocol.ipv4Header), &ethernetHeader.protocol.ipv4Header.transpProtocol.udpHeader);
            // Process UDP packet
            // ...
        }
        // Process IPv4 packet
        // ...
    } else if (ethernetHeader.etherType == 0x86DD) {
        // IPv6 packet
        parseIPv6Header((uint8_t*)packet + sizeof(struct EthernetHeader), &ethernetHeader.protocol.ipv6Header);
        if (ethernetHeader.protocol.ipv6Header.nextHeader == 0x06) {
            // TCP packet
            parseTCPHeader((uint8_t*)packet + sizeof(struct EthernetHeader) + sizeof(ethernetHeader.protocol.ipv6Header), &ethernetHeader.protocol.ipv6Header.transpProtocol.tcpHeader);
            // Process TCP packet
            // ...
        } else if (ethernetHeader.protocol.ipv6Header.nextHeader == 0x11) {
            // UDP packet
            parseUDPHeader((uint8_t*)packet + sizeof(struct EthernetHeader) + sizeof(ethernetHeader.protocol.ipv6Header), &ethernetHeader.protocol.ipv6Header.transpProtocol.udpHeader);
            // Process UDP packet
            // ...
        }
        // Process IPv6 packet
        // ...
    } else {
        // Unknown protocol
        // ...
    }

    return ethernetHeader;
}

// Ethernet header parsing function
void parseEthernetHeader(const uint8_t *frame, struct EthernetHeader *ethernetHeader) {
    memcpy(ethernetHeader->destMAC, frame, 6);
    memcpy(ethernetHeader->sourceMAC, frame + 6, 6);
    ethernetHeader->etherType = (frame[12] << 8) | frame[13];
}

// ARP header parsing function
void parseARPHeader(const uint8_t *frame, struct ARPHeader *arpHeader) {
    memcpy(&arpHeader->hardwareType, frame, 2);
    memcpy(&arpHeader->protocolType, frame + 2, 2);
    arpHeader->hardwareAddrLen = frame[4];
    arpHeader->protocolAddrLen = frame[5];
    memcpy(&arpHeader->opcode, frame + 6, 2);
    memcpy(arpHeader->senderMAC, frame + 8, 6);
    memcpy(&arpHeader->senderIP, frame + 14, 4);
    memcpy(arpHeader->targetMAC, frame + 18, 6);
    memcpy(&arpHeader->targetIP, frame + 24, 4);
}

// IPv4 header parsing function
void parseIPv4Header(const uint8_t *frame, struct IPv4Header *ipv4Header) {
    ipv4Header->version = (frame[0] >> 4) & 0xF;
    ipv4Header->headerLength = frame[0] & 0xF;
    ipv4Header->typeOfService = frame[1];
    ipv4Header->totalLength = (frame[2] << 8) | frame[3];
    ipv4Header->identification = (frame[4] << 8) | frame[5];
    ipv4Header->flagsFragmentOffset = (frame[6] << 8) | frame[7];
    ipv4Header->ttl = frame[8];
    ipv4Header->protocol = frame[9];
    ipv4Header->headerChecksum = (frame[10] << 8) | frame[11];
    memcpy(&ipv4Header->sourceIP, frame + 12, 4);
    memcpy(&ipv4Header->destIP, frame + 16, 4);
}

// IPv6 header parsing function
void parseIPv6Header(const uint8_t *frame, struct IPv6Header *ipv6Header) {
    memcpy(&ipv6Header->versionClassFlow, frame, 4);
    ipv6Header->payloadLength = (frame[4] << 8) | frame[5];
    ipv6Header->nextHeader = frame[6];
    ipv6Header->hopLimit = frame[7];
    memcpy(ipv6Header->sourceIP, frame + 8, 16);
    memcpy(ipv6Header->destIP, frame + 24, 16);
}

// TCP header parsing function
void parseTCPHeader(const uint8_t *frame, struct TCPHeader *tcpHeader) {
    tcpHeader->sourcePort = (frame[0] << 8) | frame[1];
    tcpHeader->destPort = (frame[2] << 8) | frame[3];
    tcpHeader->sequenceNumber = (frame[4] << 24) | (frame[5] << 16) | (frame[6] << 8) | frame[7];
    tcpHeader->acknowledgmentNumber = (frame[8] << 24) | (frame[9] << 16) | (frame[10] << 8) | frame[11];
    tcpHeader->dataOffset = (frame[12] >> 4) & 0xF;
    tcpHeader->reserved = (frame[12] & 0xF) | ((frame[13] >> 6) & 0x3);
    tcpHeader->flags = ((frame[13] & 0x3F) << 8) | frame[14];
    tcpHeader->windowSize = (frame[15] << 8) | frame[16];
    tcpHeader->checksum = (frame[17] << 8) | frame[18];
    tcpHeader->urgentPointer = (frame[19] << 8) | frame[20];
}

// UDP header parsing function
void parseUDPHeader(const uint8_t *frame, struct UDPHeader *udpHeader) {
    udpHeader->sourcePort = (frame[0] << 8) | frame[1];
    udpHeader->destPort = (frame[2] << 8) | frame[3];
    udpHeader->length = (frame[4] << 8) | frame[5];
    udpHeader->checksum = (frame[6] << 8) | frame[7];
}
