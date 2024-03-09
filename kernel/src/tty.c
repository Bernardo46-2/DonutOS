#include "../include/tty.h"
#include "../include/kb.h"
#include "../include/blue_scr.h"
#include "../include/donut.h"
#include "../include/pci.h"

#include "../../libc/include/atoi.h"
#include "../../libc/include/string.h"
#include "../../libc/include/printf.h"
#include "../../libc/include/rand.h"

#define TTY_INPUT_SIZE  512
#define TTY_HISTORY_SIZE 4

#define TTY_ROW (tty_ptr / VGA_HEIGHT)
#define TTY_COL (tty_ptr % VGA_WIDTH)

// output
static uint16_t* tty_buffer;
static size_t tty_prompt_ptr;
static size_t tty_ptr;
static uint16_t tty_color;

// input
static volatile uint8_t tty_has_key_ready;
static volatile uint8_t tty_last_key;
static uint8_t tty_stop_read;

static char tty_history[TTY_HISTORY_SIZE][TTY_INPUT_SIZE] = { 0 };

static char* tty_input_buf;
static size_t tty_input_ptr;
static size_t tty_input_len;

static size_t history_ptr;
static int32_t history_fst;
static int32_t history_lst;

// ---------------------------------------------------------------------------------------------------------------------------------------- //

void tty_init() {
    // output
    tty_buffer = (uint16_t*)VGA_MEMORY;
    tty_ptr = 0;
    tty_color = vga_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);

    //input
    tty_stop_read = 1;
    tty_has_key_ready = 0;
    history_ptr = 0;
    history_fst = -1;
    history_lst = -1;
}

// --------------------------------------------------------------- Output ----------------------------------------------------------------- //

void tty_set_color(const enum vga_color fg, const enum vga_color bg) {
    tty_color = vga_color(fg, bg);
}

inline void tty_update_cursor() {
    vga_move_cursor_to(tty_ptr);
}

static void __tty_write_char(const uint16_t c) {
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

void tty_putc(const uint16_t c) {
    __tty_write_char(c);
    tty_update_cursor();
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
            __tty_write_char(*s);
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

    tty_update_cursor();

    return s - str;
}

// ------------------------------------------------------------ History ---------------------------------------------------------------------- //

static void __history_new_buf() {
    if(history_fst == -1 && history_lst == -1) {
        history_fst = 0;
        history_lst = 0;
        return;
    }
    
    char* tmp_ptr = tty_history[history_ptr];
    char* snd_lst = tty_history[((size_t)(history_lst-1)) % TTY_HISTORY_SIZE];
    
    if(tmp_ptr[0] == '\0') return;
        
    if(tty_history[history_lst][0] != '\0') {
        if(strcmp(tmp_ptr, snd_lst) != 0) {
            strcpy(tty_history[history_lst], tmp_ptr);
            history_lst = (history_lst+1) % TTY_HISTORY_SIZE;
        } else {
            tty_history[history_lst][0] = '\0';
        }
    } else {
        if(strcmp(tmp_ptr, snd_lst) != 0) {
            strcpy(tty_history[history_lst], tty_history[history_ptr]);
            history_lst = (history_lst+1) % TTY_HISTORY_SIZE;
        }
    }
    
    if(history_fst == history_lst) {
        history_fst = (history_fst+1) % TTY_HISTORY_SIZE;
    }

    history_ptr = history_lst;
}

static void __history_move(int32_t offset) {
    size_t tmp = tty_input_len;
    tty_input_buf[tty_input_len] = '\0';
    
    history_ptr = ((size_t)(history_ptr+offset)) % TTY_HISTORY_SIZE;
    tty_input_buf = tty_history[history_ptr];
    
    tty_input_len = strlen(tty_input_buf);
    tty_input_ptr = tty_input_len;
    
    tty_ptr = tty_prompt_ptr;
    tty_puts(tty_input_buf);
    
    if(tmp > tty_input_len) {
        for(size_t i = 0; i < tmp; i++) {
            __tty_write_char(' ');
        }
    }

    tty_ptr = tty_prompt_ptr + tty_input_len;
}

static void __history_next_buf() {
    if(history_ptr != history_lst) {
        __history_move(1);
    }
}

static void __history_prev_buf() {
    if(history_ptr != history_fst) {
        __history_move(-1);
    }
}

// ---------------------------------------------------------------- Input ----------------------------------------------------------------- //

static void __tty_read_key(unsigned char key) {
    tty_has_key_ready = 1;
    tty_last_key = key;
}

// TODO: delete key
static void __tty_handle_key() {
    tty_has_key_ready = 0;
    unsigned char key = tty_last_key;
    
    if(!(tty_stop_read = key == '\n')) {
       switch(key) {
           case KEY_UP:
               __history_prev_buf();
               break;
           case KEY_DOWN:
               __history_next_buf();
               break;
           case KEY_LEFT:
               if(0 < tty_input_ptr && tty_input_ptr <= tty_input_len) {
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
               tty_input_ptr = tty_input_len;
               tty_update_cursor();
               break;
           case KEY_BACKSPACE:
               if(tty_input_len > 0 && tty_input_ptr > 0) {
                   if(tty_input_ptr == tty_input_len) {
                       tty_puts("\b \b");
                       tty_input_len--;
                       tty_input_ptr--;
                   } else {
                       for(size_t i = tty_ptr-1; i < tty_ptr+tty_input_len-1; i++) {
                           tty_buffer[i] = tty_buffer[i+1];
                       }
                       tty_buffer[tty_ptr + tty_input_len] = vga_entry(' ', tty_color);
                       tty_ptr--;

                       memmove((void*)(tty_input_buf+tty_input_ptr-1), (void*)(tty_input_buf+tty_input_ptr), tty_input_len - tty_input_ptr);
                       tty_input_ptr--;
                       tty_input_len--;
                       tty_update_cursor();
                   }
               }
               break;
           case KEY_DELETE:
               // TODO: same as above
               break;
           default:
               if(tty_input_len > tty_input_ptr) {
                   for(size_t i = tty_input_len; i > tty_input_ptr; i--) {
                       tty_input_buf[i] = tty_input_buf[i-1];
                       tty_buffer[tty_ptr+(i-tty_input_ptr)] = tty_buffer[tty_ptr+(i-tty_input_ptr-1)];
                   }
               }
               tty_input_len++;
               tty_input_buf[tty_input_ptr++] = key;
               tty_putc(key);
               break;
       }
    }
}

void tty_read(char* dest) {
    __history_new_buf();
    tty_input_buf = tty_history[history_lst];
    tty_input_buf[0] = '\0';
    tty_input_ptr = 0;
    tty_input_len = 0;
    tty_stop_read = 0;
    kb_set_key_handler(__tty_read_key);

    while(tty_input_len < TTY_INPUT_SIZE && !tty_stop_read) {
        if(tty_has_key_ready) {
            __tty_handle_key();
        }
    }

    kb_set_key_handler(NULL);
    tty_putc('\n');
    tty_input_buf[tty_input_len] = '\0';
    memcpy(dest, tty_input_buf, tty_input_len+1);
}

// ---------------------------------------------------------------------------------------------------------------------------------------- //

// TODO: move the command handler somewhere else
void tty_prompt() {
    char str[TTY_INPUT_SIZE];
    mrand(1000);
    srand(0);
    
    while(1) {
        tty_putc('>');
        tty_prompt_ptr = tty_ptr;
        
        tty_read(str);

        if(strcmp(str, "donut") == 0) {
            tty_clear_scr();
            donut();
        } else if(strcmp(str, "clear") == 0) {
            tty_clear_scr();
        } else if(strcmp(str, "help") == 0) {
            tty_puts("\nAvailable Commands:\n");
            tty_puts("help  - you're here\n");
            tty_puts("about - print system info\n");
            tty_puts("clear - clears the screen\n");
            tty_puts("donut - spin the donut\n");
            tty_puts("die   - throw an error\n");
            tty_puts("\n");
        } else if(strcmp(str, "about") == 0) {
            tty_puts("DonutOS\n");
        } else if(strcmp(str, "die") == 0) {
            blue_scr(666, "the pumpkins are ready to march on mankind");
        } else if(strcmp(str, "rand") == 0) {
            printf("rand = %d\n", rand());
        } else if(strcmp(str, "pci") == 0) {
            scan_pci_bus();
        } else {
            tty_puts("command `");
            tty_puts(str);
            tty_puts("` not found\n");
        }
    }
}
