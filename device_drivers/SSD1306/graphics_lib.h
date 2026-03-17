#ifndef GRAPHICS_LIB_H
#define GRAPHICS_LIB_H

#include <stdint.h>

/* relies on null-terminator as end symbol. */
void graphics_draw_chars(char *str);

/* specify row, buflen, str, does not wrap to next line
    start is px granularity, while buflen refers to the number of chars
    in the str.
*/
void graphics_draw_line_chars(
    char *str, uint8_t row, uint8_t start, uint8_t buflen);

/* where start and len have 1 pixel granularity. */
void graphics_draw_horizontal_line(uint8_t row, uint8_t start, uint8_t len);

/* zeros out the display buffer. */
void graphics_clear(void);

/* where start is the y-offset in pixel from the top */
void graphics_draw_vertical_dotted_line(uint8_t col, uint8_t start, 
    uint8_t len);

#endif
