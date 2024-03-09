#include "../include/e1000.h"
#include "../../libc/include/string.h"

#define TX_BUFFER_SIZE 1518
#define RX_BUFFER_SIZE 1518

uint8_t tx_buffer[TX_BUFFER_SIZE];
uint8_t rx_buffer[RX_BUFFER_SIZE];
