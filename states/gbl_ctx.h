#ifndef GBL_CTX_H
#define GBL_CTX_H

#include "device_drivers/fatfs_module/ff.h"
#include <stdint.h>

typedef struct {
    void (*execute)(void *args);    /* state-specific main function */
    uint8_t status;                 /* 8 bits for flags */
    FATFS fs;                       /* filesystem-object */
} GBL_CTX_T;

extern GBL_CTX_T ctx;

#endif
