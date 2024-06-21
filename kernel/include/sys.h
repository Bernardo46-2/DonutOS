#pragma once

#include "../../libc/include/types.h"

typedef enum {
    ERR_NO_ERR,
    ERR_DEVICE_NOT_FOUND,
    ERR_CONFIG_NOT_ACCEPTED,
    ERR_DEVICE_BAD_CONFIGURATION,
    ERR_MEMORY_ALLOCATION_ERROR,
} err_t;

extern volatile int last_error;
