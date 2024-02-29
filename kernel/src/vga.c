#include "../include/vga.h"
#include "../include/asm.h"

// TODO:
// - double buffering

inline uint16_t vga_color(const enum vga_color fg, const enum vga_color bg) {
    return bg << 4 | fg;
}

inline uint16_t vga_entry(const uint16_t c, const uint16_t color) {
    return color << 8 | c;
}

void vga_move_cursor_to(size_t index) {
    outb(0x3d4, 0x0f);
    outb(0x3d5, (uint8_t)(index & 0xff));
    outb(0x3d4, 0x0e);
    outb(0x3d5, (uint8_t)(index >> 8 & 0xff));
}
