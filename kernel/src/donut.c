#include "../include/donut.h"
#include "../include/tty.h"
#include "../include/kb.h"

#include "../../libc/include/stdlib.h"
#include "../../libc/include/stdio.h"
#include "../../libc/include/string.h"
#include "../../libc/include/time.h"

// donut without math.h header, gotten from here:
// https://www.a1k0n.net/2021/01/13/optimizing-donut.html

#define R(mul,shift,x,y) \
  _=x; \
  x -= mul*y>>shift; \
  y += mul*_>>shift; \
  _ = 3145728-x*x-y*y>>11; \
  x = x*_>>10; \
  y = y*_>>10;

static char __quit = 0;
static uint16_t __tty_color = 0;

static void __donut_key_handler(uint8_t scancode) {
    __quit = 1;
    kb_set_key_handler(NULL);
    tty_set_color(__tty_color & 0xff, (__tty_color >> 16) & 0xff);
}

void donut() {
    int8_t b[1760], z[1760];
    int sA=1024,cA=0,sB=1024,cB=0,_,frame=0;
    char str[10];
    vga_disable_cursor();
    __tty_color = tty_get_color();
    kb_set_key_handler(__donut_key_handler);
    for (;;frame++) {
        if (__quit) {
            __quit = 0;
            return;
        }
        tty_set_color((frame/50 + 1) % 16, (__tty_color >> 16) % 16);
        memset(b, 32, 1760);  // text buffer
        memset(z, 127, 1760);   // z buffer
        int sj=0, cj=1024;
        for (int j = 0; j < 90; j++) {
            int si = 0, ci = 1024;  // sine and cosine of angle i
            for (int i = 0; i < 324; i++) {
                int R1 = 1, R2 = 2048, K2 = 5120*1024;

                int x0 = R1*cj + R2,
                    x1 = ci*x0 >> 10,
                    x2 = cA*sj >> 10,
                    x3 = si*x0 >> 10,
                    x4 = R1*x2 - (sA*x3 >> 10),
                    x5 = sA*sj >> 10,
                    x6 = K2 + R1*1024*x5 + cA*x3,
                    x7 = cj*si >> 10,
                    x = 40 + 30*(cB*x1 - sB*x4)/x6,
                    y = 12 + 15*(cB*x4 + sB*x1)/x6,
                    N = (-cA*x7 - cB*((-sA*x7>>10) + x2) - ci*(cj*sB >> 10) >> 10) - x5 >> 7;

                int o = x + 80 * y;
                int8_t zz = (x6-K2)>>15;
                if (22 > y && y > 0 && x > 0 && 80 > x && zz < z[o]) {
                    z[o] = zz;
                    b[o] = ".,:;~=*oO#$@"[N > 0 ? N : 0];
                }
                R(5, 8, ci, si)  // rotate i
            }
            R(9, 7, cj, sj)  // rotate j
        }
        for (int k = 0; 1761 > k; k++)
            putchar(k % 80 ? b[k] : 10);
        R(5, 7, cA, sA);
        R(5, 8, cB, sB);
        milisleep(15);
        printf("\x1b[23A");
    }
}
