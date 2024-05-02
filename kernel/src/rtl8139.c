#include "../include/rtl8139.h"
#include "../include/pci.h"
#include "../include/asm.h"
#include "../include/irq.h"
#include "../include/isr.h"
#include "../../libc/include/printf.h"
#include "../../libc/include/string.h"
#include "../../libc/include/malloc.h"

// TODO
// fazer toda a sessao de `Transmitting Packets`

#define VENDOR_ID 0x10EC
#define DEVICE_ID 0x8139

uint8_t* rx_buffer;
uint32_t ioaddr;
uint8_t mac_addr[6];
uint32_t current_packet_ptr;
long bytes_received = 0;

void receive_packet() {
    // Convertendo o ponteiro para o local atual do pacote
    uint16_t * t = (uint16_t*)(rx_buffer + current_packet_ptr);

    // O comprimento do pacote está no segundo uint16_t, pulando o cabeçalho
    uint16_t packet_length = *(t + 1);

    bytes_received += packet_length + 4;

    // Avançar para os dados do pacote (além do cabeçalho de 4 bytes)
    t += 2;

    // Alocação e cópia dos dados do pacote para processamento separado
    void * packet = malloc(packet_length);
    memcpy(packet, t, packet_length);

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////// AQUI COMECA A PILHA DE PROTOCOLOS ////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////
    // ethernet_handle_packet(packet, packet_length);
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