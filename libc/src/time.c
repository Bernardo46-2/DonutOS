#include "../include/time.h"

// TODO: write this the proper way..
void usleep(size_t time) {
    for(size_t i = 0; i < time * 100; i++);
}
