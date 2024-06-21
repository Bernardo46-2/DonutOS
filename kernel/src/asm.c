#include "../include/asm.h"

inline void cli() {
    __asm__ __volatile__ ("cli");
}

inline void sti() {
    __asm__ __volatile__ ("sti");
}

inline void io_wait() {
    outb(0x80, 0);
}

inline uint32_t get_flags() {
    uint32_t flags;
    __asm__ __volatile__ ("pushf\n\t" "pop %0" : "=g"(flags) );
    return flags;
}

inline uint32_t get_gs() {
    uint32_t gs;
    __asm__ __volatile__ ("mov %%gs, %0" : "=r" (gs));
    return gs;
}

inline uint32_t get_fs() {
    uint32_t fs;
    __asm__ __volatile__ ("mov %%fs, %0" : "=r" (fs));
    return fs;
}

inline uint32_t get_es() {
    uint32_t es;
    __asm__ __volatile__ ("mov %%es, %0" : "=r" (es));
    return es;
}

inline uint32_t get_ds() {
    uint32_t ds;
    __asm__ __volatile__ ("mov %%ds, %0" : "=r" (ds));
    return ds;
}

inline uint32_t get_cs() {
    uint32_t cs;
    __asm__ __volatile__ ("mov %%cs, %0" : "=r" (cs));
    return cs;
}

inline uint32_t get_ss() {
    uint32_t ss;
    __asm__ __volatile__ ("mov %%ss, %0" : "=r" (ss));
    return ss;
}

inline uint32_t get_cr0() {
    uint32_t cr0;
    __asm__ __volatile__ ("mov %%cr0, %0" : "=r" (cr0));
    return cr0;
}

inline void set_cr0(size_t value) {
    __asm__ __volatile__ ("mov %0, %%cr0" : : "r" (value));
}

inline uint32_t get_cr3() {
    uint32_t cr3;
    __asm__ __volatile__ ("mov %%cr3, %0" : "=r" (cr3));
    return cr3;
}

inline void set_cr3(size_t value) {
    __asm__ __volatile__ ("mov %0, %%cr3" : : "r" (value));
}

inline void set_esp(size_t value) {
    __asm__ __volatile__ ("mov %0, %%esp" : : "r" (value));
}

inline void outb(uint16_t port, uint8_t data) {
    __asm__ __volatile__ ("outb %0, %1" : : "a"(data), "Nd"(port));
}

inline uint8_t inb(uint16_t port) {
    uint8_t data;
    __asm__ __volatile__ ("inb %1, %0" : "=a"(data) : "Nd"(port));
    return data;
}

inline void outw(uint16_t port, uint16_t val) {
    __asm__ __volatile__ ("outw %0, %1" : : "a"(val), "Nd"(port));
}

inline uint16_t inw(uint16_t port) {
    uint16_t data;
    __asm__ __volatile__ ("inw %1, %0" : "=a"(data) : "Nd"(port));
    return data;
}

inline void outl(uint16_t port, uint32_t val) {
    __asm__ __volatile__ ("outl %0, %1" : : "a"(val), "Nd"(port));
}

inline uint32_t inl(uint16_t port) {
    uint32_t data;
    __asm__ __volatile__ ("inl %1, %0" : "=a"(data) : "Nd"(port));
    return data;
}
