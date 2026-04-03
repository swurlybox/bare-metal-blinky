#ifndef I2C_H
#define I2C_H

#include <stdint.h>

typedef struct i2c {
    volatile uint32_t CR1, CR2, OAR1, OAR2, DR, SR1, SR2, CCR, TRISE, FLTR;
} I2C_Handle;

/* APB1 */
#define I2C1 ((I2C_Handle *) 0x40005400)
#define I2C2 ((I2C_Handle *) 0x40005800)
#define I2C3 ((I2C_Handle *) 0x40005C00)

/* Initialize internal I2C peripheral. */
void i2c1_init(void);
void i2c3_init(void);

/* Send a single I2C packet. */
void i2c_transmit(I2C_Handle *ptr, uint8_t address, uint8_t *data,
    uint32_t data_size);

#endif
