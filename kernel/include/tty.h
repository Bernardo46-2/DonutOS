#pragma once

#include "../../libc/include/types.h"
#include "vga.h"

void tty_init();
uint16_t tty_get_color();
void tty_set_color(const enum vga_color fg, const enum vga_color bg);
void tty_putc(const uint16_t c);
void __tty_clear_scr();
int tty_puts(const char* const str);
void tty_update_cursor();
void tty_read(char* dest);
void tty_scroll(int n);
void tty_prompt();

static int __pci_command(const char * _);
static int __color_command(const char* _);
static int __ram_command(const char* _);
static int __net_status_command(const char* _);
static int __dev_command(const char* _);
static int __pctprint_command(const char* _);
static int __help_command(const char* _);
static int __about_command(const char* _);
static int __donut_command(const char* _);
static int __die_command(const char* _);
static int __rand_command(const char* _);
static int __clear_command(const char* _);
static int __paging_test_command(const char* _);
static int __process_test_command(const char* _);
static int __process_last_error_command(const char* _);

struct Command {
    char name[8];
    int (*handler)(const char* input);
    char description[50];
};

extern const struct Command commands[];
