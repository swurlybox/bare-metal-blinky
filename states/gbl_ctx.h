/* Module to include such that files are aware of the global context object. */
#ifndef GBL_CTX_H
#define GBL_CTX_H

#include "device_drivers/fatfs_module/ff.h"
#include <stdint.h>

/* generally, all relevant global variables should go in this structure. */

typedef struct {
    void (*execute)(void *args);    /* state-specific main function */
    uint8_t status;                 /* 8 bits for flags */
    /* BIT(0) indicates first entry to state. */
    FATFS fs;                       /* filesystem-object */
} GBL_CTX_T;

extern GBL_CTX_T ctx;

#endif
