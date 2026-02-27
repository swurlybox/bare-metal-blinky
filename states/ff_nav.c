#include "ff_nav.h"
#include "device_drivers/fatfs_module/ff.h"
#include <stdio.h>

/* If this structure is to be modified via interrupts, best to make the
    members volatile. If we are to do polling, then the volatile keyword
    won't be necessary. We'll play it safe. */
typedef struct ff_nav_state {
    volatile uint32_t index;
    volatile uint32_t dirent_count;
} FF_NAV_T;

static FF_NAV_T ff_nav_t = {0};

uint32_t list_directory(const char *path) {
    DIR dirobj;
    FRESULT res;
    FILINFO fno;
    uint32_t nfile = 0;
    uint32_t ndir = 0;

    /*  Open the associated directory. */
    res = f_opendir(&dirobj, path);
    if (res != FR_OK) {
        printf("Couldn't open directory: %s\r\n", path);
        return 0;
    }

    /*  Start reading items. Counting up the items along the way. 
        And printing the items to a user-interface. */
    while(1) {
        res = f_readdir(&dirobj, &fno);
        if (res != FR_OK || fno.fname[0] == 0) { break; }
        if (fno.fattrib & AM_DIR) {
            printf("DIR: %s\r\n", fno.fname);
            ndir++;
        } else {
            printf("FIL: %s\r\n", fno.fname);
            nfile++;
        }
    }

    /*  Close the directory. And return the number of counted items. */
    f_closedir(&dirobj);
    ff_nav_t.dirent_count = nfile + ndir;
    printf("%ld items\r\n", ff_nav_t.dirent_count);
    return ff_nav_t.dirent_count;
}


