#include "../include/time.h"
#include "../../kernel/include/timer.h"

void milisleep(size_t time) {
    size_t stop_time = timer_get() + time;
    while(timer_get() < stop_time);
}
