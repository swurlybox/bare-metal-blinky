#include "ff_nav.h"
#include "device_drivers/fatfs_module/ff.h"
#include "states/gbl_ctx.h"
#include "user_input/buttons.h"
#include "device_drivers/SSD1306/graphics_lib.h"
#include "device_drivers/SSD1306/ssd1306_driver.h"

#include <stdio.h>

#define BIT(x) (1U << (x))
#define CWD_SIZE    (512)

#define WIN_DEF_START   (0)
#define WIN_DEF_END     (6)

#define FILETYPE_PX_OFFSET  (12)
#define FILENAME_PX_OFFSET  (24)

#define CWD_MARGIN_OFFSET      (30)
#define MARGIN1_OFFSET      (FILETYPE_PX_OFFSET - 4)
#define MARGIN2_OFFSET      (FILENAME_PX_OFFSET - 4)

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

/* ~80 line function, pretty long already but its fine. */
uint32_t list_directory(const char *path) {
    DIR dirobj;
    FRESULT res;
    FILINFO fno;
    uint32_t nfile = 0;
    uint32_t ndir = 0;
    char c;

    /*  Open the associated directory. */
    res = f_opendir(&dirobj, path);
    if (res != FR_OK) {
        printf("Couldn't open directory: %s\r\n", path);
        return 0;
    }
    
    /* UART OUTPUT */
    while(1) {
        res = f_readdir(&dirobj, &fno);
        if (res != FR_OK || fno.fname[0] == 0) { break; }

        /* UART output. */
        if (nfile + ndir == ff_nav.index) { c = '>'; }
        else { c = ' '; } 

        if (fno.fattrib & AM_DIR) {
            printf("%5c DIR: %s\r\n", c, fno.fname);
            ndir++;
        } else {
            printf("%5c FIL: %s\r\n", c, fno.fname);
            nfile++;
        }
    }
    ff_nav.dirent_count = nfile + ndir;
    printf("%ld items\r\n", ff_nav.dirent_count);

    /* DISPLAY OUTPUT */ 
    uint8_t row = 1;
    uint8_t win_start;
    uint8_t win_end;

    graphics_clear();
    graphics_draw_line_chars("CWD: ", 0, 0, 5);
    graphics_draw_line_chars(ff_nav.cwd, 0, CWD_MARGIN_OFFSET , 20);
    graphics_draw_horizontal_line(0, 0, 128);

    /* Sliding window calculation to determine the 7 entries to display. */
    if (ff_nav.dirent_count <= 7 || ff_nav.index <= 2) {
        win_start = WIN_DEF_START;
        win_end =   WIN_DEF_END;
    }
    else if (ff_nav.dirent_count - ff_nav.index <= 3) {
        win_start = (uint8_t) (ff_nav.dirent_count - 7U);
        win_end =   (uint8_t) (ff_nav.dirent_count - 1U);
    }
    else {
        win_start = (uint8_t) (ff_nav.index - 3U);
        win_end =   (uint8_t) (ff_nav.index + 3U);
    }

    printf("win_start: %d\r\n", win_start);
    printf("win_end: %d\r\n", win_end);

    uint8_t index = 0;
    f_rewinddir(&dirobj);
    while (1) {
        res = f_readdir(&dirobj, &fno);
        if (res != FR_OK || fno.fname[0] == 0) { break; }

        if ((index) >= win_start && (index) <= win_end) {
            if (index == ff_nav.index) {
                graphics_draw_line_chars(">", row, 0, 1);
            }
            if (fno.fattrib & AM_DIR) {
                graphics_draw_line_chars("D", row, FILETYPE_PX_OFFSET, 1);    
            }   
            else {
                graphics_draw_line_chars("F", row, FILETYPE_PX_OFFSET, 1);    
            }
            graphics_draw_line_chars(fno.fname, row, FILENAME_PX_OFFSET, 20);
            graphics_draw_horizontal_line(row, MARGIN1_OFFSET, 128);
            graphics_draw_vertical_dotted_line(MARGIN1_OFFSET, (row * 8), 8);
            graphics_draw_vertical_dotted_line(MARGIN2_OFFSET, (row * 8), 8);
            row++;
        }
        index++;
    }

    /* NOTE: Notable latency between button event and display update.
        So there is room for improvement. */
    display_update();

    /* Cleanup */
    f_closedir(&dirobj);
    return ff_nav.dirent_count;
}

/* Filesystem navigation functions.*/
static void ff_nav_cancel() {
    /* Go to parent directory. */
    f_chdir("..");
    f_getcwd(ff_nav.cwd, CWD_SIZE);
    ff_nav.index = 0;
    list_directory(ff_nav.cwd);
}

static void ff_nav_select() { 
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

static void ff_nav_up() {
    if (ff_nav.index == 0) {
        ff_nav.index = ff_nav.dirent_count - 1;
    }
    else {
        ff_nav.index--;
    }
    list_directory(ff_nav.cwd);
}

static void ff_nav_down() {
    if (ff_nav.index == ff_nav.dirent_count - 1) {
        ff_nav.index = 0;
    }
    else {
        ff_nav.index++;
    }
    list_directory(ff_nav.cwd);
}
