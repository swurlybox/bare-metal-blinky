#include "graphics_lib.h"
#include <stdint.h>
#include <stdio.h>

#include "ssd1306_driver.h"

#define BIT(x) (1U << (x))

extern uint8_t display_buf[DISPLAY_BYTE_SIZE];
static uint32_t dbi = 1; /* first byte is reserved */

/* we'll need to literally write the bytes into the display_buffer,
    so some code will need to be executed to do that. Can't really use
    DMA to update the buffer, as the bytes drawn depends on input. 

    one approach is that we can make functions that draw specific characters
    on to the screen. That would make atleast 26 functions for each character
    in the alphabet. This makes our text segment a little bit large as we
    support more characters.

    Another approach is we predefine the byte pattern for each character,
    and store these characters in some array. Depending on user input,
    we index into the array of characters, then copy the byte content
    into the display buffer. This saves the hassle of defining 26 or more
    functions for each possible character, and having to do a massive
    switch/case of if/else chain.
*/

#define CHAR_BYTE_LEN   (5)
#define SUPPORTED_CHARS (34)

#define PIXELS_PER_ROW  (128)

#define PERIOD_LOC (26)
#define SPACE_LOC (27)
#define QUESTION_LOC (28)
#define EXCLAM_LOC (29)
#define COMMA_LOC (30)
#define APOST_LOC (31)
#define DQUOT_LOC (32)
#define UNSUPPORTED (33)

typedef struct {
    uint8_t byte[CHAR_BYTE_LEN];
} display_char;

/* 
0-25: A-Z
26: .
27: space   (unsupported)
28: unsupported

*/
static display_char arr[SUPPORTED_CHARS] = {
    #include "char_byte_map.txt"
};

/* need a software side pointer to the global display_buffer */
void graphics_draw_chars(char *str) {
    char c;
    int char_index;
    /* NOTE: Possible errors/considerations

        DONE: don't overflow dbi/display_buf: just stop writing the rest.
        DONE: handle character wrap-around behaviour: characters near the edge
            just go to the next row.
        TODO: support more common characters:
            apostrophe
            quotes
            exclamation
            comma

    */

    /* scan the str for ascii characters. */
    while ((c = *str) != '\0') {
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
        else {
            char_index = UNSUPPORTED;
        }
 
        /* weird display ram shit happens here. I literally commented
        and then uncommented this, then ran the program again and it works??
        */ 
        if (((dbi + CHAR_BYTE_LEN) % PIXELS_PER_ROW <= CHAR_BYTE_LEN)) {
            printf("c: %c, ogix: %ld\r\n", c, dbi);
            dbi = dbi + (((PIXELS_PER_ROW) - (dbi % PIXELS_PER_ROW)) + 1);
            printf("newix: %ld\r\n", dbi); 
        }
        
        for (int i = 0; i < 5; i++) {    
            if (dbi >= DISPLAY_BYTE_SIZE) {
                break;                
            }

            /* caching problem here? wraparound doesn't seem to apply?  */
            display_buf[dbi++] = arr[char_index].byte[i];
        }
        /* margin */
        if (dbi >= DISPLAY_BYTE_SIZE) {
            break;                
        }
        dbi++;
        str++;        
    }

    dbi = 1;
}

/* if the next 5 bytes would wrap to the next line, just start at the next
    line. */
//void graphics_next_char();
