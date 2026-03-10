#ifndef I2C_H
#define I2C_H

#include <stdint.h>

struct i2c {
    volatile uint32_t CR1, CR2, OAR1, OAR2, DR, SR1, SR2, CCR, TRISE, FLTR;
};

/* pointer to the base of the i2c peripheral: APB1 */
#define I2C1 ((struct i2c *) 0x40005400)

/* i2c init function which enables i2c peripheral clock and some i2c
    interface initial configurations. */
void i2c1_init(void);

/* encapsulates a typical transmit i2c communication. Data needs to be
    formatted beforehand. */
void i2c1_transmit(uint8_t address, uint8_t *data, uint32_t data_size);


/* i think this is a good starting point, and is probably enough
    to get an SSD1306 driver going. */
/* some sort of basic write function to write a byte of data to dr */
void i2c1_write(uint8_t byte);
void i2c1_start(void);  /* start condition */
void i2c1_stop(void);   /* stop condition */

#endif
