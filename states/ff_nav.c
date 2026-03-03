#include "ff_nav.h"
#include "device_drivers/fatfs_module/ff.h"
#include "states/gbl_ctx.h"
#include "user_input/buttons.h"

#include <stdio.h>

#define BIT(x) (1U << (x))
#define CWD_SIZE    (512)

/* If this structure is to be modified via interrupts, best to make the
    members volatile. If we are to do polling, then the volatile keyword
    won't be necessary. We'll play it safe. */
typedef struct ff_nav_state {
    volatile uint32_t index;
    volatile uint32_t dirent_count;
    char    cwd[CWD_SIZE];
} FF_NAV_T;

static FF_NAV_T ff_nav = {0};

static void ff_nav_up();
static void ff_nav_down();
static void ff_nav_select();
static void ff_nav_cancel();

void ff_nav_main(void *args) {
    (void) args;
    
    /* Code that executes only once. Only on a fresh entry into this state. */
    if ((ctx.status & 1U)) {
        f_getcwd(ff_nav.cwd, CWD_SIZE);
        list_directory(ff_nav.cwd);

        /* hook up button functionality to ff navigation functions.*/
        button_arr[UP].release = ff_nav_up;
        button_arr[DOWN].release = ff_nav_down;
        button_arr[SELECT].release = ff_nav_select;
        button_arr[CANCEL].release = ff_nav_cancel;

        printf("Enter ff_nav\r\n");
        ctx.status &= (uint8_t) ~BIT(0);
    }

    /* Do any work that needs to be done. */
    buttons_listen();
}

void ff_nav_cancel() {
    /* Go to parent directory. */
    f_chdir("..");
    f_getcwd(ff_nav.cwd, CWD_SIZE);
    ff_nav.index = 0;
    list_directory(ff_nav.cwd);
}

void ff_nav_select() { 
    FRESULT res;
    DIR dirobj;
    FILINFO fno;

    res = f_opendir(&dirobj, ff_nav.cwd);
    if (res != FR_OK) {
        /* NOTE: If this is the case, we have a corrupt cwd. */
        printf("Couldn't open directory: %s\r\n", ff_nav.cwd);
        return;
    }

    /* Get the selected item at the right index. */
    for (uint32_t i = 0; i <= ff_nav.index; i++) {
        res = f_readdir(&dirobj, &fno);
    }
    
    if (fno.fattrib & AM_DIR) {
        /* If you are a directory, change to it. */
        res = f_chdir(fno.fname);
        f_getcwd(ff_nav.cwd, CWD_SIZE);
        ff_nav.index = 0;
    } else {
        /* Selected a file. */
        printf("file selected: %s\r\n", fno.fname);
    }

    f_closedir(&dirobj);
    list_directory(ff_nav.cwd);
}

void ff_nav_up() {
    if (ff_nav.index == 0) {
        ff_nav.index = ff_nav.dirent_count - 1;
    }
    else {
        ff_nav.index--;
    }
    list_directory(ff_nav.cwd);
}

void ff_nav_down() {
    if (ff_nav.index == ff_nav.dirent_count - 1) {
        ff_nav.index = 0;
    }
    else {
        ff_nav.index++;
    }
    list_directory(ff_nav.cwd);
}


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

    char c = ' ';
    /*  Start reading items. Counting up the items along the way. 
        And printing the items to a user-interface. */
    while(1) {
        res = f_readdir(&dirobj, &fno);
        if (res != FR_OK || fno.fname[0] == 0) { break; }
        
        /* currently printing to terminal, later to an i2c display */
        if (nfile + ndir == ff_nav.index) {
            c = '>';
        }
        else {
            c = ' ';
        }

        if (fno.fattrib & AM_DIR) {
            printf("%5c DIR: %s\r\n", c, fno.fname);
            ndir++;
        } else {
            printf("%5c FIL: %s\r\n", c, fno.fname);
            nfile++;
        }
    }

    /*  Close the directory. And return the number of counted items. */
    f_closedir(&dirobj);
    ff_nav.dirent_count = nfile + ndir;
    printf("%ld items\r\n", ff_nav.dirent_count);
    return ff_nav.dirent_count;
}


