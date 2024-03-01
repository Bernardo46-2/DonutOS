#include "../include/tty.h"
#include "../include/kb.h"

#include "../../libc/include/atoi.h"
#include "../../libc/include/string.h"

// TODO: remove this from here
#include "../include/donut.h"

#define TTY_INPUT_SIZE 4096

#define TTY_ROW (tty_ptr / VGA_HEIGHT)
#define TTY_COL (tty_ptr % VGA_WIDTH)

static uint16_t* tty_buffer;
static volatile size_t tty_ptr;
static uint16_t tty_color;
static char tty_input_buf[TTY_INPUT_SIZE] = { 0 };
static volatile size_t tty_input_ptr;
static volatile size_t tty_input_len;
static volatile char tty_stop_read = 0;

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

void tty_clear_scr() {
    uint16_t entry = vga_entry(' ', tty_color);
    
    for(size_t i = 0; i < VGA_SCR_SIZE; i++) {
        tty_buffer[i] = entry;
    }
    tty_ptr = 0;
    tty_update_cursor();
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

inline void tty_update_cursor() {
    vga_move_cursor_to(tty_ptr);
}

// TODO: handle special keys
// - arrows
// - backspace | delete
// - some commands like ctrl+c
static void tty_read_key(unsigned char key) {
    tty_stop_read = key == '\n';

    switch(key) {
    case KEY_LEFT:
        if(0 < tty_input_ptr && tty_input_ptr <= TTY_INPUT_SIZE) {
            tty_ptr--;
            tty_input_ptr--;
            tty_update_cursor();
        }
        break;
    case KEY_RIGHT:
        if(0 <= tty_input_ptr && tty_input_ptr < tty_input_len) {
            tty_ptr++;
            tty_input_ptr++;
            tty_update_cursor();
        }
        break;
    case KEY_HOME:
        tty_ptr -= tty_input_ptr;
        tty_input_ptr = 0;
        tty_update_cursor();
        break;
    case KEY_END:
        tty_ptr += tty_input_len - tty_input_ptr;
        tty_input_ptr = tty_input_len - 1;
        tty_update_cursor();
        break;
    case KEY_BACKSPACE:
        if(tty_input_len > 0) {
            // TODO: move remaining of string left
            memmove((void*)(tty_input_ptr-1), (void*)(tty_input_ptr), tty_input_len - tty_input_ptr);

            if(tty_input_ptr == tty_input_len) {
                tty_puts("\b \b");
            } else {
                memmove((void*)(tty_ptr-1), (void*)(tty_ptr), tty_input_len - tty_input_ptr);
                tty_ptr--;
                tty_update_cursor();
            }

            tty_input_ptr--;
            tty_input_len--;
        }
        break;
    case KEY_DELETE:
        // TODO: same as above
        break;
    default:
        // TODO: move remaining of string right
        tty_input_len++;
        tty_input_buf[tty_input_ptr++] = key;
        tty_putc(key);
        tty_update_cursor();
        break;
    }
}

void tty_read(char* dest) {
    tty_input_buf[0] = '\0';
    tty_input_ptr = 0;
    tty_input_len = 0;
    tty_stop_read = 0;
    kb_set_key_handler(tty_read_key);

    while(tty_input_len < TTY_INPUT_SIZE && !tty_stop_read);

    kb_set_key_handler(NULL);
    tty_input_buf[tty_input_ptr-1] = '\0';
    memcpy(dest, tty_input_buf, tty_input_len);
}

// TODO: move the command handler somewhere else
void tty_prompt() {
    char str[128];
    
    while(1) {
        tty_putc('>');
        tty_update_cursor();
        
        tty_read(str);

        if(strcmp(str, "donut") == 0) {
            tty_clear_scr();
            donut();
        } else if(strcmp(str, "clear") == 0) {
            tty_clear_scr();
        } else if(strcmp(str, "help") == 0) {
            tty_puts("\nAvailable Commands:\n");
            tty_puts("about - print system info\n");
            tty_puts("help - you're here\n");
            tty_puts("clear - clears the screen\n");
            tty_puts("donut - spin the donut\n\n");
        } else if(strcmp(str, "about") == 0) {
            tty_puts("DonutOS\n");
        } else {
            tty_puts("command `");
            tty_puts(str);
            tty_puts("` not found\n");
        }
    }
}
