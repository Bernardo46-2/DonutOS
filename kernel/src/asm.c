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

inline double asm_sin(double x) {
    double result;
    __asm__ __volatile__ ("fsin" : "=t"(result) : "0"(x));
    return result;
}

inline double asm_cos(double x) {
    double result;
    __asm__ __volatile__ ("fcos" : "=t"(result) : "0"(x));
    return result;
}
