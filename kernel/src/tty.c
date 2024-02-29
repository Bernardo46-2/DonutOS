#include "../include/tty.h"
#include "../include/kb.h"

#include "../../libc/include/atoi.h"
#include "../../libc/include/string.h"

#define TTY_ROW (tty_ptr / VGA_HEIGHT)
#define TTY_COL (tty_ptr % VGA_WIDTH)

static uint16_t* tty_buffer;
static size_t tty_ptr;
static uint16_t tty_color;
static char* tty_input_buf[4096];
static size_t tty_input_ptr;

void tty_init() {
    tty_buffer = (uint16_t*)VGA_MEMORY;
    tty_ptr = 0;
    tty_color = vga_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
}

void tty_set_color(const enum vga_color fg, const enum vga_color bg) {
    tty_color = vga_color(fg, bg);
}

void tty_putc(const uint16_t c) {
    uint16_t entry;
    
    switch(c) {
    case '\n':
        tty_ptr += VGA_WIDTH - TTY_COL;
        break;
    case '\r':
        tty_ptr -= TTY_COL;
        break;
    case '\b':
        tty_ptr--;
        break;
    default:
        entry = vga_entry(c, tty_color);
        tty_buffer[tty_ptr++] = entry;
        break;
    }
}

int tty_puts(const char* const str) {
    const char* s = str;
    
    while(*s != '\0') {
        if(*s != '\033') {
            tty_putc((*s));
        } else {
            s++;
            size_t tmp_len = 16;
            char tmp[tmp_len];
            size_t i = 0;
            int x = 0;

            if(*s == '[') {
                s++;
                while('0' <= *s && *s <= '9') {
                    tmp[i++] = *s;
                    s++;
                }

                if(i < tmp_len) tmp[i] = '\0';
                if(tmp[0] != '\0') x = atoi(tmp);

                switch(*s) {
                    case 'A':
                        tty_ptr -= VGA_WIDTH * x;
                        break;
                    case 'B':
                        tty_ptr += VGA_WIDTH * x;
                        break;
                    case 'C':
                        tty_ptr += x;
                        break;
                    case 'D':
                        tty_ptr -= x;
                        break;
                    case 'H':
                        tty_ptr = 0;
                        break;
                    default:
                        break;
                }
            }
        }

        s++;
    }

    vga_move_cursor_to(tty_ptr);

    return s - str;
}

// do the buffer saving here
// receive char
// handle what to do
// kb handles only control keys and send other keys here

inline void tty_update_cursor() {
    vga_move_cursor_to(tty_ptr);
}

void tty_read_key(char key) {
    
}

// TODO: a proper read prompt
// - when kb.last_key() == '\n', copy buffer to a str and
//   handle special chars and whatnot. that will optimze
//   the kb_handler
char* tty_read() {
    // kb_set_state(KB_READ);
    
    char last_key;
    do {
        last_key = kb_last_key();
    } while(last_key != '\n');
    
    // kb_set_state(KB_OFF);
    
    return NULL;
}
