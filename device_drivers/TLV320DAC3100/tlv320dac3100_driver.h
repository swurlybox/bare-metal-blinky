#ifndef TLV320DAC3100_H
#define TLV320DAC3100_H

#define DAC_I2C_ADDR    (0x18)

/* send the necessary commands over i2c2 to initialize the DAC. */
void dac_init();

/* extraneous controls: volume control, etc. */

#endif
