#include "graphics_lib.h"
#include <stdint.h>
#include <stdio.h>

#include "ssd1306_driver.h"

#define BIT(x) (1U << (x))

extern uint8_t display_buf[DISPLAY_BYTE_SIZE];
static uint32_t dbi = 1; /* first byte is reserved */

#define CHAR_BYTE_LEN   (5)
#define SUPPORTED_CHARS (36)

/* display characteristics */
#define MAX_ROWS        (7U)
#define PIXELS_PER_ROW  (128U)

#define PERIOD_LOC      (26)
#define SPACE_LOC       (27)
#define QUESTION_LOC    (28)
#define EXCLAM_LOC      (29)
#define COMMA_LOC       (30)
#define APOST_LOC       (31)
#define DQUOT_LOC       (32)
#define FSLASH_LOC      (33)
#define COLON_LOC       (34)
#define UNSUPPORTED     (35)

typedef struct {
    uint8_t byte[CHAR_BYTE_LEN];
} display_char;

/* contains information on how to draw the supported chars to the display. */
static display_char arr[SUPPORTED_CHARS] = {
    #include "char_byte_map.txt"
};

/* TODO: Also want to support / character, the directory separator */

/* helper functions */
static int  find_index(char c);
static uint8_t near_edge_check();      /* drawing the next char would wrap */
static void draw_char(int index);   /* draws at current dbi index*/

void graphics_clear() {
    for (int i = 1; i < DISPLAY_BYTE_SIZE; i++) {
        display_buf[i] = 0;
    }
}

/* start is 0-based, so the first pixel of a row is at index 0. */
void graphics_draw_horizontal_line(uint8_t row, uint8_t start, uint8_t len) {
    /* input check, row and start are valid. */
    if (row > MAX_ROWS || start > PIXELS_PER_ROW) {
        return;
    }

    /* set dbi to point at correct row and start. */
    dbi = (row * PIXELS_PER_ROW) + start + 1;

    /* loop until len is exhausted and reached edge-boundary:
        draw the 1 pixel that creates the bottom-aligned horizontal line.
        NOTE: conditional is pretty fragile. This only works if our display
        is 1-based not 0-based. */
    while ((len-- > 0) && (dbi % PIXELS_PER_ROW) != 0) {
        display_buf[dbi++] |= BIT(7);
    }
    // Handle very last pixel. NOTE: probably better way to do this
    if ((len > 0) && (dbi % PIXELS_PER_ROW == 0)) {
        display_buf[dbi] |= BIT(7);
    }

    dbi = 1;

}

void graphics_draw_line_chars(char *str, uint8_t buflen, uint8_t row) {
    char c;
    int char_index;

    /* only 8 rows on our display */
    if (row > MAX_ROWS) {
        return;
    }
    dbi = (row * PIXELS_PER_ROW) + 1;
    
    while (((c = *str) != '\0') && buflen-- > 0) {
        char_index = find_index(c);

        /* don't want to wrap to next line, so just get out. */
        if ((PIXELS_PER_ROW - (dbi % PIXELS_PER_ROW)) < CHAR_BYTE_LEN) {
            break;
        }

        draw_char(char_index);
        
        dbi++;
        str++;
    }
    dbi = 1;
}

void graphics_draw_chars(char *str) {
    char c;
    int char_index;

    /* scan the str for ascii characters. */
    while ((c = *str) != '\0') {
        /* find where the character is in the display_char arr. */
        char_index = find_index(c);  

        /* wrap dbi to next line if needed. */
        if(near_edge_check()) {
            dbi = (dbi + (((PIXELS_PER_ROW) - (dbi % PIXELS_PER_ROW)) + 1));
        }

        draw_char(char_index); 
        
        /* get out if we'll overflow the buffer. */
        if (dbi >= DISPLAY_BYTE_SIZE) { break; } 
        dbi++;  /* margin between chars */
        str++;        
    }

    dbi = 1;
}

/* helper implementations */
static int find_index(char c) {
    int char_index = UNSUPPORTED;
    if ((c >= 'A' && c <= 'Z')) {
        char_index = c - 'A';
    }
    else if ((c >= 'a' && c <= 'z')) {
        char_index = c - 'a';
    }
    else if ((c == '.')) {
        char_index = PERIOD_LOC;
    }
    else if ((c == ' ')) {
        char_index = SPACE_LOC;
    }
    else if ((c == '?')) {
        char_index = QUESTION_LOC;
    }
    else if ((c == '!')) {
        char_index = EXCLAM_LOC;
    }
    else if ((c == ',')) {
        char_index = COMMA_LOC;
    }
    else if ((c == '\'')) {
        char_index = APOST_LOC;
    }
    else if ((c == '"')) {
        char_index = DQUOT_LOC;
    }
    else if ((c == '/')) {
        char_index = FSLASH_LOC;
    }
    else if ((c == ':')) {
        char_index = COLON_LOC;
    }
    return char_index;
}

static uint8_t near_edge_check() {
    if (((dbi + CHAR_BYTE_LEN) % PIXELS_PER_ROW <= CHAR_BYTE_LEN)) {
        return 1;
    }
    return 0;
}

static void draw_char(int index) {
    for (int i = 0; i < CHAR_BYTE_LEN && dbi < DISPLAY_BYTE_SIZE; i++) {    
        display_buf[dbi++] = arr[index].byte[i];
    }   
}
