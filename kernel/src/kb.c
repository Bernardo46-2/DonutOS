#include "../include/kb.h"
#include "../include/asm.h"
#include "../include/isr.h"
#include "../include/irq.h"
#include "../include/ctx.h"

#define IS_PRESS(x)      (x < 0x80)
#define IS_RELEASE(x)    (x > 0x80)
#define SCANCODE_TO_KEY(x)  (x & 0x7f)

static uint8_t kb_layout_us[2][128] = {
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

static struct {
    uint8_t shift : 1;
    uint8_t ctrl : 1;
    uint8_t alt : 1;
    uint8_t num_lock : 1;
    uint8_t caps_lock : 1;
    uint8_t scroll_lock : 1;
} mods;

static uint8_t __last_key = 0;

inline uint8_t kb_last_key() {
    return __last_key;
}

static void (*kb_key_handler)(unsigned char) = NULL;

static void __kb_handler(regs_t* rs) {
    uint16_t scancode = inb(0x60);
    uint8_t key = SCANCODE_TO_KEY(scancode);
    uint8_t is_release = IS_RELEASE(scancode);

    __last_key = key;

    switch(key) {
        case KEY_LSHIFT:
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
        case KEY_RSHIFT:
            if(is_release) {
                uint8_t i = (mods.caps_lock ^ mods.shift) & 0x1;
                __proc_kb_debug(rs, kb_layout_us[i][key]);
            }
            break;
        default:
            if(kb_key_handler && !is_release) {
                uint8_t i = (mods.caps_lock ^ mods.shift) & 0x1;
                kb_key_handler(kb_layout_us[i][key]);
            }
            break;
    }
}

void kb_set_key_handler(void (*key_handler)(unsigned char)) {
    if(kb_key_handler == NULL || key_handler == NULL) {
        kb_key_handler = key_handler;
    }
}

void kb_init() {
    irq_install(1, __kb_handler);
}
