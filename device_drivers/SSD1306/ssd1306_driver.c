#include "ssd1306_driver.h"
#include "peripherals/i2c.h"
#include "graphics_lib.h"
#include "peripherals/systick.h"

#define BIT(x) (1U << (x))

uint8_t display_buf[DISPLAY_BYTE_SIZE];;  /* 8 for the control byte */

#define FILETYPE_PX_OFFSET (12)
#define FILENAME_PX_OFFSET (24)

void display_update() {
    display_buf[0] = 0x40;      /* data bytes only */

    /*
    graphics_draw_chars("abcdefghijklmnopqrstuvwxyz"
    " ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    " !?.,'\"/:0123456789_@");
    graphics_clear();
    graphics_draw_line_chars("CWD: myMemeFolder", 0, 0, 30);
    graphics_draw_horizontal_line(0, 0, 200);
    for (uint8_t i = 1; i <= 7; i++) {
        graphics_draw_line_chars("F", i, FILETYPE_PX_OFFSET, 30);
        graphics_draw_horizontal_line(i, 8, 200);
    }
    graphics_draw_line_chars(">", 3, 0, 1);
    graphics_draw_vertical_dotted_line(
        FILETYPE_PX_OFFSET - 4, 8, 64
    );
    graphics_draw_vertical_dotted_line(
        FILENAME_PX_OFFSET - 4, 8, 64
    );
    graphics_draw_line_chars("a really long file", 1, FILENAME_PX_OFFSET, 30); 
    graphics_draw_line_chars("short", 2, FILENAME_PX_OFFSET, 30);
    graphics_draw_line_chars("music.mp3", 3, FILENAME_PX_OFFSET, 30);
    graphics_draw_line_chars("hidden_figures.mv", 4, FILENAME_PX_OFFSET, 30);
    graphics_draw_line_chars("hi :D", 5, FILENAME_PX_OFFSET, 30);
    */

    i2c1_transmit(DISPLAY_I2C_ADDR, display_buf, DISPLAY_BYTE_SIZE);
}

void display_init() {
    uint8_t buflen = 4;
    uint8_t data[buflen];

    /* 100ms delay to give display ample time to ramp up voltage. */
    delay(100);

    /* SET MUX RATIO */
    data[0] = 0x80;      /* command and no continuation. */
    data[1] = 0xA8;
    data[2] = 0x80;
    data[3] = 0x3F;
    i2c1_transmit(DISPLAY_I2C_ADDR, data, 4);

    /* SET MEM ADDRESSING MODE */
    data[1] = 0x20;
    data[3] = 0x00;
    i2c1_transmit(DISPLAY_I2C_ADDR, data, 4);

    /* SET DISPLAY OFFSET */
    /* 0x00 */
    data[1] = 0xD3;
    /* 0x00 */ 
    data[3] = 0x00;
    i2c1_transmit(DISPLAY_I2C_ADDR, data, 4);

    /* SET DISPLAY START LINE */
    data[1] = 0x40;
    i2c1_transmit(DISPLAY_I2C_ADDR, data, 2);

    /* SET SEGMENT RE-MAP */
    data[1] = 0xA0;
    i2c1_transmit(DISPLAY_I2C_ADDR, data, 2);

    /* SET COM OUTPUT SCAN DIRECTION */
    data[1] = 0xC0;     /* C0/C8*/
    i2c1_transmit(DISPLAY_I2C_ADDR, data, 2);

    /* SET COM PINS HW CONFIGS */
    data[1] = 0xDA;
    data[3] = 0x12;     /* apparently alternating fixes it? but the datasheet's
                            diagram and the actual result has a discrepancy. */
    i2c1_transmit(DISPLAY_I2C_ADDR, data, 4);

    /* SET CONTRAST CONTROL */
    data[1] = 0x81;
    data[3] = 0x7F;
    i2c1_transmit(DISPLAY_I2C_ADDR, data, 4);

    /* DISABLE ENTIRE DISPLAY ON */
    data[1] = 0xA4;
    i2c1_transmit(DISPLAY_I2C_ADDR, data, 2);

    /* SET NORMAL DISPLAY */
    data[1] = 0xA6;
    i2c1_transmit(DISPLAY_I2C_ADDR, data, 2);

    /* SET OSC FREQUENCY */
    data[1] = 0xD5;
    data[3] = 0x80;
    i2c1_transmit(DISPLAY_I2C_ADDR, data, 4); /* around 117 fps */

    /* ENABLE CHARGE PUMP REGULATOR */
    data[1] = 0x8D;
    data[3] = 0x14;
    i2c1_transmit(DISPLAY_I2C_ADDR, data, 4);

    /* DISPLAY ON */
    data[1] = 0xAF;
    i2c1_transmit(DISPLAY_I2C_ADDR, data, 2);
}
