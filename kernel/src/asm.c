#include "../include/asm.h"

inline void cli() {
    __asm__ __volatile__ ("cli");
}

inline void sti() {
    __asm__ __volatile__ ("sti");
}

inline void outb(uint16_t port, uint8_t data) {
    __asm__ __volatile__ ("outb %0, %1" : : "a"(data), "Nd"(port));
}

inline uint8_t inb(uint16_t port) {
    uint8_t data;
    __asm__ __volatile__ ("inb %1, %0" : "=a"(data) : "Nd"(port));
    return data;
}

inline void io_wait() {
    outb(0x80, 0);
}

inline uint16_t get_if() {
    uint16_t flags;
    __asm__ __volatile__ ( "pushf\n\t"
                           "pop %0"
                           : "=g"(flags) );
    return flags & (1 << 9);
}

inline void outl(uint16_t port, uint32_t val) {
    __asm__ volatile ("outl %0, %1" : : "a"(val), "Nd"(port));
}

inline uint32_t inl(uint16_t port) {
    uint32_t ret;
    __asm__ volatile("inl %1, %0"
                     : "=a"(ret)
                     : "Nd"(port));
    return ret;
}
