#ifndef _SYS_H_
#define _SYS_H_

#include "../../libc/include/types.h"

typedef enum {
    ERR_NO_ERR,
    ERR_DEVICE_NOT_FOUND,
    ERR_CONFIG_NOT_ACCEPTED,
    ERR_DEVICE_BAD_CONFIGURATION,
} err_t;

#endif
