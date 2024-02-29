#include "../include/kb.h"
#include "../include/asm.h"
#include "../include/isr.h"
#include "../include/irq.h"
#include "../include/vga.h"
#include "../include/tty.h"

#define IS_PRESS(x)      (x < 0x80)
#define IS_RELEASE(x)    (x > 0x80)
#define KEY_SCANCODE(x)  (x & 0x7f)

static uint8_t kb_layout[2][128] = {
    {
        KEY_NULL, KEY_ESC, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
        '-', '=', KEY_BACKSPACE, KEY_TAB, 'q', 'w', 'e', 'r', 't', 'y', 'u',
        'i', 'o', 'p', '[', ']', KEY_ENTER, 0, 'a', 's', 'd', 'f', 'g', 'h', 'j',
        'k', 'l', ';', '\'', '`', KEY_LSHIFT, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm',
        ',', '.', '/', KEY_LSHIFT, 0, 0, ' ', 0, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5,
        KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, 0, 0, KEY_HOME, KEY_UP,
        KEY_PAGE_UP, '-', KEY_LEFT, '5', KEY_RIGHT, '+', KEY_END, KEY_DOWN,
        KEY_PAGE_DOWN, KEY_INSERT, KEY_DELETE, 0, 0, 0, KEY_F11, KEY_F12
    }, {
        KEY_NULL, KEY_ESC, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
        '_', '+', KEY_BACKSPACE, KEY_TAB, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U',
        'I', 'O', 'P', '{', '}', KEY_ENTER, 0, 'A', 'S', 'D', 'F', 'G', 'H', 'J',
        'K', 'L', ':', '\"', '~', KEY_LSHIFT, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M',
        '<', '>', '?', KEY_RSHIFT, 0, 0, ' ', 0, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5,
        KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, 0, 0, KEY_HOME, KEY_UP,
        KEY_PAGE_UP, '-', KEY_LEFT, '5', KEY_RIGHT, '+', KEY_END, KEY_DOWN,
        KEY_PAGE_DOWN, KEY_INSERT, KEY_DELETE, 0, 0, 0, KEY_F11, KEY_F12
    }
};

static char* kb_buf = NULL;
static char kb_buf_index = 0;
static size_t kb_buf_size = 0;

static struct {
    uint8_t shift : 1;
    uint8_t ctrl : 1;
    uint8_t alt : 1;
    uint8_t num_lock : 1;
    uint8_t caps_lock : 1;
    uint8_t scroll_lock : 1;
} mods;

static void (*kb_key_handler)(char key) = NULL;

// toggles keyboard read state on/off
// also resets buffer if state = KB_READ
// inline void kb_set_state(enum kb_state st) {
//     state = st;
//     if(state == KB_READ) kb_buf_index = 0;
// }

inline char kb_last_key() {
    return kb_buf[kb_buf_index-1];
}

// TODO
// this is gonna have to handle some special keys
// - home
// - end
// - arrows
// - backspace
static void __kb_handler(regs_t* rs) {
    uint16_t scancode = inb(0x60);
    uint8_t key = KEY_SCANCODE(scancode);
    uint8_t is_release = IS_RELEASE(scancode);
    uint8_t is_control_key = 1;

    switch(key) {
        case KEY_LSHIFT:
        case KEY_RSHIFT:
            mods.shift = !is_release;
            break;
        case KEY_NUM_LOCK:
            mods.num_lock = is_release ? !mods.num_lock : mods.num_lock;
            break;
        case KEY_CAPS_LOCK:
            mods.caps_lock = is_release ? !mods.caps_lock : mods.caps_lock;
            break;
        case KEY_SCROLL_LOCK:
            mods.scroll_lock = is_release ? !mods.scroll_lock : mods.scroll_lock;
            break;
        case KEY_LCTRL:
            mods.ctrl = !is_release;
            break;
        case KEY_LALT:
            mods.alt = !is_release;
            break;
        default:
            is_control_key = 0;
            break;
    }
    
    if(state == KB_READ && !is_control_key) {
        if(!is_release) {
            uint8_t i = (mods.caps_lock ^ mods.shift) & 0x1;
            kb_buf[kb_buf_index++] = kb_layout[i][key];
            tty_putc(kb_layout[i][key]);
            tty_update_cursor();
        }
    } else if(state == KB_HIT) {
        // TODO
    }
}

void kb_init() {
    irq_install(1, __kb_handler);
}
