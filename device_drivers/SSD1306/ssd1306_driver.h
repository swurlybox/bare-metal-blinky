#ifndef SSD1306_DRIVER_H
#define SSD1306_DRIVER_H

#define DISPLAY_I2C_ADDR (0x3C)
#define DISPLAY_BYTE_SIZE   (1024 + 1)

#include <stdint.h>

/* 128 * 64 = 8192 bits on a monochrome display. That's 1024 bytes. */
extern uint8_t display_buf[DISPLAY_BYTE_SIZE];

/* necessary configurations to get display booted up. */
void display_init();

/* sends the contents of the display buffer to ssd1306's gddram. */
void display_update();

#endif
