#include "../include/vga.h"

// TODO:
// - double buffering (requires malloc)

uint16_t vga_color(const enum vga_color fg, const enum vga_color bg) {
    return bg << 4 | fg;
}

uint16_t vga_entry(const uint16_t c, const uint16_t color) {
    return color << 8 | c;
}
