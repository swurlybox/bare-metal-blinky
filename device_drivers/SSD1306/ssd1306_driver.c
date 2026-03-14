#include "ssd1306_driver.h"
#include "peripherals/i2c.h"
#include "graphics_lib.h"
#include "peripherals/systick.h"

#define BIT(x) (1U << (x))

uint8_t display_buf[DISPLAY_BYTE_SIZE];;  /* 8 for the control byte */

void display_update() {
    /* TEST: Write some pixels on the screen */
    display_buf[0] = 0x40;      /* data bytes only */
    graphics_draw_chars("abcdefghijklmnopqrstuvwxyz"
    " ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    " !?.,'\"@$%^");


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
