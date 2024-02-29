#ifndef _KB_H_
#define _KB_H_

#include "../../libc/include/types.h"

#define KEY_NULL        0
#define KEY_ESC         27
#define KEY_BACKSPACE   '\b'
#define KEY_TAB         '\t'
#define KEY_ENTER       '\n'

#define KEY_INSERT      0x90
#define KEY_DELETE      0x91
#define KEY_HOME        0x92
#define KEY_END         0x93
#define KEY_PAGE_UP     0x94
#define KEY_PAGE_DOWN   0x95
#define KEY_LEFT        0x4B
#define KEY_UP          0x48
#define KEY_RIGHT       0x4D
#define KEY_DOWN        0x50

#define KEY_F1          0x80
#define KEY_F2          (KEY_F1 + 1)
#define KEY_F3          (KEY_F1 + 2)
#define KEY_F4          (KEY_F1 + 3)
#define KEY_F5          (KEY_F1 + 4)
#define KEY_F6          (KEY_F1 + 5)
#define KEY_F7          (KEY_F1 + 6)
#define KEY_F8          (KEY_F1 + 7)
#define KEY_F9          (KEY_F1 + 8)
#define KEY_F10         (KEY_F1 + 9)
#define KEY_F11         (KEY_F1 + 10)
#define KEY_F12         (KEY_F1 + 11)

#define KEY_LCTRL       0x1D
#define KEY_RCTRL       0x1D

#define KEY_LALT        0x38
#define KEY_RALT        0x38

#define KEY_LSHIFT      0x2A
#define KEY_RSHIFT      0x36

#define KEY_CAPS_LOCK   0x3A
#define KEY_SCROLL_LOCK 0x46
#define KEY_NUM_LOCK    0x45

enum kb_state {
    KB_OFF,
    KB_READ,
    KB_HIT
};

void kb_set_state(enum kb_state st);
char kb_last_key();
void kb_init();

#endif
