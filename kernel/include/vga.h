#ifndef _VGA_H_
#define _VGA_H_

#include "../../libc/include/types.h"

#define VGA_MEMORY 0xb8000
#define VGA_HEIGHT 25
#define VGA_WIDTH 80
#define VGA_SCR_SIZE (VGA_HEIGHT * VGA_WIDTH)

enum vga_color {
	VGA_COLOR_BLACK = 0x0,
	VGA_COLOR_BLUE = 0x1,
	VGA_COLOR_GREEN = 0x2,
	VGA_COLOR_CYAN = 0x3,
	VGA_COLOR_RED = 0x4,
	VGA_COLOR_MAGENTA = 0x5,
	VGA_COLOR_BROWN = 0x6,
	VGA_COLOR_LIGHT_GREY = 0x7,
	VGA_COLOR_DARK_GREY = 0x8,
	VGA_COLOR_LIGHT_BLUE = 0x9,
	VGA_COLOR_LIGHT_GREEN = 0xa,
	VGA_COLOR_LIGHT_CYAN = 0xb,
	VGA_COLOR_LIGHT_RED = 0xc,
	VGA_COLOR_LIGHT_MAGENTA = 0xd,
	VGA_COLOR_LIGHT_BROWN = 0xe,
	VGA_COLOR_WHITE = 0xf,
};

uint16_t vga_color(const enum vga_color fg, const enum vga_color bg);
uint16_t vga_entry(const uint16_t c, const uint16_t color);
void vga_move_cursor_to(size_t index);
void vga_enable_cursor(uint8_t start, uint8_t end);
void vga_disable_cursor();

#endif
