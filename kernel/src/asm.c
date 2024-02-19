#include "../include/asm.h"

// TODO: double check this entire thing

size_t GDT_CODE_ADDR = 0;

void read_consts() {
    GDT_CODE_ADDR = *(size_t*)(0x7ff0);
    
    if(GDT_CODE_ADDR == 0) {
        // TODO: blue screen
    }
}

inline size_t gdt_code_addr() {
    return GDT_CODE_ADDR;
}

void cli() {
    __asm__ __volatile__ ("cli");
}

void outb(uint16_t port, uint8_t data) {
    __asm__ __volatile__ ("outb %0, %1" : : "a"(data), "Nd"(port));
}

uint8_t inb(uint16_t port) {
    uint8_t data;
    __asm__ __volatile__ ("inb %1, %0" : "=a"(data) : "Nd"(port));
    return data;
}
