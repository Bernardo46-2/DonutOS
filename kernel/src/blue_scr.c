#include "../include/blue_scr.h"
#include "../include/asm.h"
#include "../include/tty.h"

#include "../../libc/include/stdio.h"
#include "../../libc/include/string.h"

void blue_scr(int code, const char* message) {
    cli();
    tty_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLUE);
    __tty_clear_scr();

    size_t msg_len = strlen(message);
    size_t pad_len = (VGA_WIDTH - msg_len - 5) / 2;

    if(message != NULL) {
        for(size_t i = 0; i < VGA_HEIGHT * 0.2; i++) {
            putchar('\n');
        }
        
        for(size_t i = 0; i < (VGA_WIDTH - 17) / 2; i++) {
            putchar(' ');
        }
        printf("Error and whatnot\n\n\n");
        
        for(size_t i = 0; i < (VGA_WIDTH - 7) / 2; i++) {
            putchar(' ');
        }
        printf("no: %2d\n\n", code);
        
        for(size_t i = 0; i < pad_len; i++) {
            putchar(' ');
        }
        printf("msg: %s\n", message);
    }
    
    vga_disable_cursor();
    while(1);
}
