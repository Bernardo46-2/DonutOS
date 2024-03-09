#include "../include/pit.h"
#include "../include/asm.h"

#define PIT_HZ        1193182

#define PIT_CHANNEL_0 0x40
#define PIT_CHANNEL_1 0x41
#define PIT_CHANNEL_2 0x42
#define PIT_COMMAND   0x43

static void __pit_set_command(uint8_t channel, uint8_t access_mode, uint8_t op_mode, uint8_t bcd) {
    uint8_t command_byte = 0;
    command_byte |= bcd & 0x1;
    command_byte |= (op_mode & 0x7) << 1;
    command_byte |= (access_mode & 0x3) << 4;
    command_byte |= (channel & 0x3) << 6;
    outb(PIT_COMMAND, command_byte);
}

void pit_set(int hz) {
    uint16_t divisor = PIT_HZ / hz;
    __pit_set_command(0, 3, 3, 0); // square wave mode
    outb(PIT_CHANNEL_0, divisor & 0xff);
    outb(PIT_CHANNEL_0, divisor >> 8 & 0xff);
}
