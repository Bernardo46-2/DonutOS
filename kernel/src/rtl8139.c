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
long bytes_received = 0;
struct EthernetHeader* ethHeaders;
int firstHeader = 0;
int nHeaders = 0;

extern struct EthernetHeader process_received_packet(void* packet, uint16_t packet_length);

void parseEthernetHeader(const uint8_t *frame, struct EthernetHeader *ethernetHeader);
void parseARPHeader(const uint8_t *frame, struct ARPHeader *arpHeader);
void parseIPv4Header(const uint8_t *frame, struct IPv4Header *ipv4Header);
void parseIPv6Header(const uint8_t *frame, struct IPv6Header *ipv6Header);
void parseTCPHeader(const uint8_t *frame, struct TCPHeader *tcpHeader);
void parseUDPHeader(const uint8_t *frame, struct UDPHeader *udpHeader);

int getHeaderIndex(const int i) {
    return (firstHeader + i) % 256;
}

void removeFirstHeader() {
    free(&ethHeaders[getHeaderIndex(firstHeader)]);
    firstHeader = (firstHeader + 1) % 256;
    nHeaders--;
}

struct EthernetHeader* rtl8139_get_ethHeader(int i) {
    return &ethHeaders[getHeaderIndex(i)];
}

void receive_packet() {
    // Convertendo o ponteiro para o local atual do pacote
    uint16_t * t = (uint16_t*)(rx_buffer + current_packet_ptr);

    // O comprimento do pacote está no segundo uint16_t, pulando o cabeçalho
    uint16_t frame_length = *(t + 1) + 4;

    bytes_received += frame_length + 4;

    // Avançar para os dados do pacote (além do cabeçalho de 4 bytes)


    // Alocação e cópia dos dados do pacote para processamento separado
    void * frame = malloc(frame_length);
    memcpy(frame, t+2, frame_length);

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////// AQUI COMECA A PILHA DE PROTOCOLOS ////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////
    // ethernet_handle_packet(packet, packet_length);
    if (nHeaders == 256) {
        removeFirstHeader();
    }
    ethHeaders[getHeaderIndex(nHeaders)] = process_received_packet(frame, frame_length);
    nHeaders++;

    // Atualizando o ponteiro de pacote considerando alinhamento
    current_packet_ptr = ((current_packet_ptr + frame_length + 3) & RX_READ_POINTER_MASK) % RX_BUFFER_SIZE;

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
    ethHeaders = (struct EthernetHeader*)malloc(256 * sizeof(struct EthernetHeader));


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
    struct EthernetHeader ethHeader = ethHeaders[getHeaderIndex(nHeaders-1)];

    printf("%d\n %d\n %d\n%d\n",ethHeader.protocol.ipv4Header.transpProtocol.udpHeader.checksum, ethHeader.protocol.ipv4Header.transpProtocol.udpHeader.destPort, ethHeader.protocol.ipv4Header.transpProtocol.udpHeader.length,ethHeader.protocol.ipv4Header.transpProtocol.udpHeader.sourcePort);
    for (int i = 0; i < 256; i++) {
        printf("%02x  ", rx_buffer[i]);
    }
    printf("\n");
}

void printIP(uint8_t* ip, uint8_t isIPv6) {
    int i;
    if (isIPv6) {
        for (i = 0; i < 16; i++) {
            printf("%02x", ip[i]);
            if (i % 2 == 1 && i < 15) {
                printf(":");
            }
        }
    } else {
        for (i = 0; i < 4; i++) {
            printf("%d", ip[i]);
            if (i < 3) {
                printf(".");
            }
        }
    }
    printf("\n");
}

void printTCP(struct TCPHeader tcpHeader) {
    printf("SOURCE PORT: %d\n", tcpHeader.sourcePort);
    printf("DEST PORT: %d\n", tcpHeader.destPort);
    printf("SEQ: %d\n", tcpHeader.sequenceNumber);
    printf("ACK: %d\n", tcpHeader.acknowledgmentNumber);
    printf("DATA OFFSET: %d\n", tcpHeader.dataOffset);
    printf("FLAGS: %d\n", tcpHeader.flags);
    printf("WINDOW SIZE: %d\n", tcpHeader.windowSize);
    printf("CHECKSUM: %d\n", tcpHeader.checksum);
    printf("URGENT POINTER: %d\n", tcpHeader.urgentPointer);
}

void printUDP(struct UDPHeader udpHeader) {
    printf("SOURCE PORT: %d\n", udpHeader.sourcePort);
    printf("DEST PORT: %d\n", udpHeader.destPort);
    printf("LENGTH: %d\n", udpHeader.length);
    printf("CHECKSUM: %d\n", udpHeader.checksum);
}

void printICMP(struct ICMPHeader icmpHeader) {
    printf("TYPE: %d\n", icmpHeader.type);
    printf("CODE: %d\n", icmpHeader.code);
    printf("CHECKSUM: %d\n", icmpHeader.checksum);
}
void printHopOptions(struct IPV6HopByHopHeader hopByHopHeader) {
    printf("NEXT HEADER: %d\n", hopByHopHeader.nextHeader);
    printf("HEADER EXT LENGTH: %d\n", hopByHopHeader.headerExtLength);
    printf("OPTIONS: ");
    for (int i = 0; i < 6; i++) {
        printf("%02x", hopByHopHeader.options[i]);
    }
    printf("\n");
    printf("PADDING: ");
    for (int i = 0; i < 2; i++) {
        printf("%02x", hopByHopHeader.padding[i]);
    }
    printf("\n");
}

void printIPv6(struct IPv6Header ipv6Header) {
    printf("VERSION: %d\n", ipv6Header.versionClassFlow >> 28);
    printf("CLASS: %d\n", (ipv6Header.versionClassFlow >> 20) & 0xFF);
    printf("FLOW LABEL: %d\n", ipv6Header.versionClassFlow & 0xFFFFF);
    printf("PAYLOAD LENGTH: %d\n", ipv6Header.payloadLength);
    printf("NEXT HEADER: %d\n", ipv6Header.nextHeader);
    printf("HOP LIMIT: %d\n", ipv6Header.hopLimit);
    printf("SOURCE IP: ");
    printIP(ipv6Header.sourceIP, 1);
    printf("DEST IP: ");
    printIP(ipv6Header.destIP, 1);
}

void printIPv4(struct IPv4Header ipv4Header) {
    printf("VERSION: %d\n", ipv4Header.version);
    printf("HEADER LENGTH: %d\n", ipv4Header.headerLength);
    printf("TYPE OF SERVICE: %d\n", ipv4Header.typeOfService);
    printf("TOTAL LENGTH: %d\n", ipv4Header.totalLength);
    printf("IDENTIFICATION: %d\n", ipv4Header.identification);
    printf("FLAGS FRAGMENT OFFSET: %d\n", ipv4Header.flagsFragmentOffset);
    printf("TTL: %d\n", ipv4Header.ttl);
    printf("PROTOCOL: %d\n", ipv4Header.protocol);
    printf("HEADER CHECKSUM: %d\n", ipv4Header.headerChecksum);
    printf("SOURCE IP: ");
    printIP((uint8_t*)&ipv4Header.sourceIP, 0);
    printf("DEST IP: ");
    printIP((uint8_t*)&ipv4Header.destIP, 0);
}


void rtl_printFrame(int i) {
    //ethernetHeader to be read here.
    // fazer if no EtherType do struct dnv pra descobri qual protocolo é
    // depois, printar o struct do pacote
    char* protocolName;
    uint8_t* sourceIP;
    uint8_t* destIP;
    uint16_t length;
    struct EthernetHeader ethHeader = ethHeaders[i];

    printf("ETHERNET HEADER\n");
    printf("SOURCE MAC: ");
    for (int i = 0; i < 6; i++) {
        printf("%02x", ethHeader.sourceMAC[i]);
        if (i < 5) {
            printf(":");
        }
    }
    printf("\nDESTINATION MAC: ");
    for (int i = 0; i < 6; i++) {
        printf("%02x", ethHeader.destMAC[i]);
        if (i < 5) {
            printf(":");
        }
    }
    printf("\n\n");

    printf("NETWORK PROTOCOL: ");
    // Verificar o tipo de protocolo
    if (ethHeader.etherType == 0x0800) { // IPv4
        printf("IPv4\n");
        printIPv4(ethHeader.protocol.ipv4Header);
        printf("PROTOCOL: ");
        switch (ethHeader.protocol.ipv4Header.protocol)
        {
        case 0x06:
            printf("TCP\n");
            printTCP(ethHeader.protocol.ipv4Header.transpProtocol.tcpHeader);
            break;
        case 0x11:
            printf("UDP\n");
            printUDP(ethHeader.protocol.ipv4Header.transpProtocol.udpHeader);
            break;
        case 0x01:
            printf("ICMP\n");
            printICMP(ethHeader.protocol.ipv4Header.transpProtocol.icmpHeader);
            break;
        default:
            printf("Unsuppored protocol\n");
            break;
        }
    } else if (ethHeader.etherType == 0x86DD) { // IPv6
        printf("IPv6\n");
        printIPv6(ethHeader.protocol.ipv6Header);
    
        printf("PROTOCOL: ");
        struct IPv6Header ipv6Header = ethHeader.protocol.ipv6Header;
        switch (ethHeader.protocol.ipv6Header.nextHeader)
        {
        case 0x06:
            printf("TCP\n");
            printf("SOURCE PORT: %d\n", ethHeader.protocol.ipv6Header.transpProtocol.tcpHeader.sourcePort);
            break;
        case 0x11:
            printf("UDP\n");
            printf("SOURCE PORT: %d\n", ethHeader.protocol.ipv6Header.transpProtocol.udpHeader.sourcePort);
            break;

        case 0:
            printf("HOPOPT\n");
            printHopOptions(ipv6Header.transpProtocol.hopByHopHeader);
            printf("NEXT HEADER: ");
            switch (ipv6Header.transpProtocol.hopByHopHeader.nextHeader)
            {
            case 0x06:
                printf("TCP\n");
                printTCP(ipv6Header.transpProtocol.hopByHopHeader.transpProtocol.tcpHeader);
            case 0x11:
                printf("UDP\n");
                printUDP(ipv6Header.transpProtocol.hopByHopHeader.transpProtocol.udpHeader);
                break;
            case 0x3A:
                printf("ICMPv6\n");
                printICMP(ipv6Header.transpProtocol.hopByHopHeader.transpProtocol.icmpHeader);
            }
            break;
        case 0x3A:
            printf("ICMPv6\n");
            printICMP(ipv6Header.transpProtocol.icmpHeader);
            break;
        default:
            printf("Unsuppored protocol\n");
            break;
        }
    } else if (ethHeader.etherType == 0x0806){ // ARP
        printf("ARP\n");
        printf("SENDER MAC: ");
        for (int i = 0; i < 6; i++) {
            printf("%02x", ethHeader.protocol.arpHeader.senderMAC[i]);
            if (i < 5) {
                printf(":");
            }
        }
        printf("\nSENDER IP: ");
        for (int i = 0; i < 4; i++) {
            printf("%d", ethHeader.protocol.arpHeader.senderIP >> (8 * i) & 0xFF);
            if (i < 3) {
                printf(".");
            }
        }
        printf("\nTARGET MAC: ");
        for (int i = 0; i < 6; i++) {
            printf("%02x", ethHeader.protocol.arpHeader.targetMAC[i]);
            if (i < 5) {
                printf(":");
            }
        }
        printf("\nTARGET IP: ");
        for (int i = 0; i < 4; i++) {
            printf("%d", ethHeader.protocol.arpHeader.targetIP >> (8 * i) & 0xFF);
            if (i < 3) {
                printf(".");
            }
        }
        printf("\n");
        printf("PROTOCOL: %s\n", ethHeader.protocol.arpHeader.opcode == 1 ? "REQUEST" : "REPLY");
    } else {
        printf("Unknown protocol\n");
        return; // Não é um pacote IP ou ARP, então não fazemos nada
    }

    
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

void parseICMPHeader(const uint8_t *frame, struct ICMPHeader *icmpHeader) {
    icmpHeader->type = frame[0];
    icmpHeader->code = frame[1];
    icmpHeader->checksum = (frame[2] << 8) | frame[3];
    icmpHeader->restOfHeader = (frame[4] << 24) | (frame[5] << 16) | (frame[6] << 8) | frame[7];
}

void parseIPv6HopByHop(const uint8_t *frame, struct IPV6HopByHopHeader *ipv6HopByHopHeader) {
    ipv6HopByHopHeader->nextHeader = frame[0];
    ipv6HopByHopHeader->headerExtLength = frame[1];
    memcpy(ipv6HopByHopHeader->options, frame + 2, 6);
    memcpy(ipv6HopByHopHeader->padding, frame + 8, 2);
    if (ipv6HopByHopHeader->nextHeader == 0x06) {
        parseTCPHeader(frame + 8, &ipv6HopByHopHeader->transpProtocol.tcpHeader);
    } else if (ipv6HopByHopHeader->nextHeader == 0x11) {
        parseUDPHeader(frame + 8, &ipv6HopByHopHeader->transpProtocol.udpHeader);
    } else if (ipv6HopByHopHeader->nextHeader == 0x3A) {
        parseICMPHeader(frame + 8, &ipv6HopByHopHeader->transpProtocol.icmpHeader);
    }
}

struct EthernetHeader process_received_packet(void* frame, uint16_t frame_length) {
    // Parse Ethernet header
    struct EthernetHeader ethernetHeader;
    parseEthernetHeader((uint8_t*)frame, &ethernetHeader);
  

    // Check the EtherType to determine the protocol
    if (ethernetHeader.etherType == 0x0806) {
        // ARP packet
    

        parseARPHeader((uint8_t*)frame + 14, &ethernetHeader.protocol.arpHeader);
    } else if (ethernetHeader.etherType == 0x0800) {
        // IPv4 packet
        parseIPv4Header((uint8_t*)frame + 14, &ethernetHeader.protocol.ipv4Header);
        if (ethernetHeader.protocol.ipv4Header.protocol == 0x06) {
            // TCP packet
            parseTCPHeader((uint8_t*)frame + 14 + 20, &ethernetHeader.protocol.ipv4Header.transpProtocol.tcpHeader);
            // Process TCP packet
            // ...
        } else if (ethernetHeader.protocol.ipv4Header.protocol == 0x11) {
            // UDP packet
            parseUDPHeader((uint8_t*)frame + 14 + 20, &ethernetHeader.protocol.ipv4Header.transpProtocol.udpHeader);
            // Process UDP packet
            // ...
        }
        // Process IPv4 packet
        // ...
    } else if (ethernetHeader.etherType == 0x86DD) {
        // IPv6 packet
        parseIPv6Header((uint8_t*)frame + 14, &ethernetHeader.protocol.ipv6Header);
        if (ethernetHeader.protocol.ipv6Header.nextHeader == 0x06) {
            // TCP packet
            parseTCPHeader((uint8_t*)frame + 14 + 40, &ethernetHeader.protocol.ipv6Header.transpProtocol.tcpHeader);
            // Process TCP packet
            // ...
        } else if (ethernetHeader.protocol.ipv6Header.nextHeader == 0x11) {
            // UDP packet
            parseUDPHeader((uint8_t*)frame + 14 + 40, &ethernetHeader.protocol.ipv6Header.transpProtocol.udpHeader);
            // Process UDP packet
            // ...
        } else if (ethernetHeader.protocol.ipv6Header.nextHeader == 0x3A) {
            // ICMPv6 packet
            parseICMPHeader((uint8_t*)frame + 14 + 40, &ethernetHeader.protocol.ipv6Header.transpProtocol.icmpHeader);
            // Process ICMPv6 packet
            // ...
        } else if (ethernetHeader.protocol.ipv6Header.nextHeader == 0) {
            // Hop-by-Hop Options header
            parseIPv6HopByHop((uint8_t*)frame + 14 + 40, &ethernetHeader.protocol.ipv6Header.transpProtocol.hopByHopHeader);
            // Process Hop-by-Hop Options header
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
    memcpy(&arpHeader->senderMAC, frame + 8, 6);
    memcpy(&arpHeader->senderIP, frame + 14, 4);
    memcpy(&arpHeader->targetMAC, frame + 18, 6);
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
    memcpy(&ipv6Header->payloadLength, frame + 4, 2);
    ipv6Header->nextHeader = frame[6];
    ipv6Header->hopLimit = frame[7];
    memcpy(&ipv6Header->sourceIP, frame + 8, 16);
    memcpy(&ipv6Header->destIP, frame + 24, 16);
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

rtl_device rtl8139_get_status() {
    pci_device_t device;
    pci_get_device(VENDOR_ID, DEVICE_ID, &device);

    rtl_device rtl_dev;

    rtl_dev.vendor_id = device.vendor_id;
    rtl_dev.device_id = device.device_id;
    rtl_dev.bars = device.bars;
    rtl_dev.irq = device.irq;
    rtl_dev.io_address = get_io_address(device);
    memcpy(rtl_dev.mac_addr, mac_addr, 6);

    return rtl_dev;
}
