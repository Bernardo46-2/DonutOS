#include "../include/tty.h"
#include "../include/kb.h"
#include "../include/blue_scr.h"
#include "../include/donut.h"
#include "../include/pci.h"
#include "../include/rtl8139.h"
#include "../include/sys.h"
#include "../include/paging.h"

#include "../../libc/include/atoi.h"
#include "../../libc/include/string.h"
#include "../../libc/include/printf.h"
#include "../../libc/include/rand.h"
#include "../../libc/include/malloc.h"

#define TTY_INPUT_SIZE  512
#define TTY_HISTORY_SIZE 16

#define TTY_ROW (tty_ptr / VGA_WIDTH)
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

const struct Command commands[] = {
            {"help", __help_command, "show this command"},
            {"about", __about_command, "print system info"},
            {"clear", __clear_command, "clears the screen"},
            {"donut", __donut_command, "spin the donut"},
            {"die", __die_command, "throw an error"},
            {"color", __color_command, "set screen color"},
            {"rand", __rand_command, "print random number"},
            {"pci", __pci_command, "scan pci bus"},
            {"ram", __ram_command, "ram usage"},
            {"dev", __dev_command, "device status"},
            {"print", __pctprint_command, "packet print"},
            {"net", __net_status_command, "start net device"},
            {"paging", __paging_test_command, "test paging"},
};

// ---------------------------------------  ------------------------------------------------------------------------------------------------- //

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

    __tty_clear_scr();
}

// --------------------------------------------------------------- Output ----------------------------------------------------------------- //

void tty_scroll(int n) {
    size_t gap = n * VGA_WIDTH;
    uint16_t entry = vga_entry(0, tty_color);

    if (gap > VGA_SCR_SIZE) {
        gap = VGA_SCR_SIZE;
    } else if (gap < 0) {
        gap = 0;
    }

    if ( n >= 0) {
        for (size_t i = 0; i < VGA_SCR_SIZE - gap; i++) {
            tty_buffer[i] = vga_entry(tty_buffer[i + gap] & 0xFF, tty_color);
        }
        for (size_t i = VGA_SCR_SIZE - gap; i < VGA_SCR_SIZE; i++) {
            tty_buffer[i] = entry;
        }
    }

    tty_ptr -= gap;
}

inline void tty_set_color(const enum vga_color fg, const enum vga_color bg) {
    tty_color = vga_color(fg, bg);
    tty_scroll(0);
}

void tty_update_cursor() {
    if (tty_ptr >= VGA_SCR_SIZE) {
       tty_scroll(TTY_ROW - VGA_HEIGHT + 1);
    }

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

void __tty_clear_scr() {
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

uint16_t tty_get_color() {
    return tty_color;
}

// ---------------------------------------------------------------- Input ----------------------------------------------------------------- //

static void __tty_read_key(unsigned char key) {
    tty_has_key_ready = 1;
    tty_last_key = key;
}

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
                if (tty_input_len > 0 && tty_input_ptr < tty_input_len) {
                    for (size_t i = tty_ptr; i < tty_ptr + tty_input_len - 1; i++) {
                        tty_buffer[i] = tty_buffer[i + 1];
                    }
                    tty_buffer[tty_ptr + tty_input_len - 1] = vga_entry(0, tty_color);
                    tty_update_cursor();

                    memmove((void*)(tty_input_buf + tty_input_ptr), (void*)(tty_input_buf + tty_input_ptr + 1), tty_input_len - tty_input_ptr);
                    tty_input_len--;
                    
                }
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

// -------------------------------------------------------------- Command Handler ----------------------------------------------------------- //

static int __pci_command(const char* _) {
    pci_scan_bus();
    return 0;
}

static int __color_command(const char* _)  {
    tty_puts("Enter color: ");
    char input[TTY_INPUT_SIZE];
    tty_read(input);
    int color = atoi(input);
    tty_puts("Background color: ");
    tty_read(input);
    int bg = atoi(input);
    tty_puts("\n");
    tty_set_color(color, bg);
    return 0;
};

static int __ram_command(const char* _) {
    printf("Total = %d, used = %d (%f %%)\n", (int)TOTAL_MEMORY, (int)memory_used, (float)memory_used*100/TOTAL_MEMORY);
    return 0;
}

static int __net_status_command(const char* _) {
    rtl_print_buffer();
    return 0;
}

static int __dev_command(const char* _) {
    //print net device status
    rtl_device dev = rtl8139_get_status();
    printf("Vendor ID: %x\n", dev.vendor_id);
    printf("Device ID: %x\n", dev.device_id);
    printf("IRQ: %d\n", dev.irq);
    printf("MAC Address: %02x:%02x:%02x:%02x:%02x:%02x\n", dev.mac_addr[0], dev.mac_addr[1], dev.mac_addr[2], dev.mac_addr[3], dev.mac_addr[4], dev.mac_addr[5]);
    printf("IO Address: %x\n", dev.io_address);
    printf("BAR0: %x\n", dev.bars.bar[0]);

    return 0;
}

static int __pctprint_command(const char* _) {
    char input[TTY_INPUT_SIZE];
    int index = 0;
    while (1) {
        if (nHeaders == 0) {
            printf("No headers to print\n");
            return 0;
        }

        if (index >= nHeaders) {
            printf("Invalid index\n");
        }

        printf("Number of headers: %d\n", nHeaders);

        tty_puts("Enter header index: ");

        tty_read(input);
        index = atoi(input);

        if (index >= nHeaders) {
            __tty_clear_scr();
            continue;
        } else if (index < 0) {
            return 0;
        }
        __tty_clear_scr();
        rtl_printFrame(index);
    }
}


static int __help_command(const char* _) {
    for (int i = 0; i < sizeof(commands)/sizeof(struct Command); i++) {
        printf("%s - %s\n", commands[i].name, commands[i].description);
    }
    return 0;
}

static int __about_command(const char* _) {
    printf("DonutOS");
    return 0;
}
static int __donut_command(const char* _) {
    __tty_clear_scr();
            vga_disable_cursor();
            donut();
            vga_enable_cursor(0, 15);
            __tty_clear_scr();
            return 0;
}
static int __die_command(const char* _) {
    blue_scr(666, "the pumpkins are ready to march on mankind");
    return 0;
}
static int __rand_command(const char* _) {
    printf("rand = %d\n", rand());
    return 0;
}

static int __clear_command(const char* _) {
    __tty_clear_scr();
    return 0;
}

static int __paging_test_command(const char* _) {
    __paging_test();
    return 0;
}

// ---------------------------------------------------------------------------------------------------------------------------------------- //

// TODO: move the command handler somewhere else
void tty_prompt() {
    char str[TTY_INPUT_SIZE];
    
    // fat cursor
    vga_enable_cursor(0, 15);
    
    while(1) {
        tty_putc('>');
        tty_prompt_ptr = tty_ptr;
        
        tty_read(str);

        bool_t found = false;

        for (int i = 0; i < sizeof(commands)/sizeof(struct Command); i++) {
            if (strcmp(str, (void*)commands[i].name) == 0) {
                found = true;
                int err = commands[i].handler(str) != 0;
                if (err) printf("The command run into an error (%d)\n", err);
                break;
            }
        }

        if (!found) printf("Command not found\n");
    }
}
