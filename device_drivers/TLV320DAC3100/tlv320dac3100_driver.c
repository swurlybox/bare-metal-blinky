#include "tlv320dac3100_driver.h"
#include "peripherals/i2c.h"
#include "peripherals/systick.h"

#include <stdio.h>  // debugging

#define SZ  (2)

/*
void i2c_transmit(I2C_Handle *ptr, uint8_t address, uint8_t *data,
    uint32_t data_size); */

static uint8_t rd[SZ];

static void send_cmd(uint8_t reg, uint8_t dat) {
    rd[0] = reg;
    rd[1] = dat;
    i2c_transmit(I2C3, DAC_I2C_ADDR, rd, SZ);
}

void dac_init() {
    /* some delays need to be introduced to allow time for power voltages to
        ramp up. 10ns for power up, and 10ms after pll start up. */

    delay(1000);

    printf("enter dac init\r\n");


    /* 1) Define starting point:
        a) Power up applicable external hardware power supplies
        b) Set register page to 0. 
        c) Initiate SW reset (PLL is powered off as part of reset) */
        

    /* Set register page to 0. */
    send_cmd(0x00, 0x00);

    printf("hello?\r\n");

    /* Initiate SW reset */
    send_cmd(0x01, 0x01);

    /* 2) Program clock settings:
        a) Program PLL clock dividers P, J, D, R 
        b) Power up PLL
        c) Program and power up NDAC
        d) Program and power up MDAC
        e) Program OSR value
        f) Program I2S word length if required and master mode
        g) Program the processing block to be used
        h) Miscellaneous page 0 controls */

    /*  Let J=8, R=8, D=0, P=1 : gives us 98.304 MHz CODEC_CLK_IN
        Let NDAC=4, MDAC=4, DOSR=128 : gives us 48KHz DAC_fs */
    
    /* PLL_clkin: BCLK, codec_clkin: PLL_CLK */
    send_cmd(0x04, 0x07);

    /* J = 8 */
    send_cmd(0x06, 0x08);

    /* D = 0 (default) */
    /* P=1, R=8, PLL Power on */
    send_cmd(0x05, 0b10011000); 

    delay(10);

    /* NDAC = 4 */
    send_cmd(0x0B, 0b10000100);

    /* MDAC = 4 */
    send_cmd(0x0C, 0b10000100);
  
    /* DOSR = 128 (default) */
    /* I2S mode, 16-bits, Slave mode */ 
    send_cmd(0x1B, 0x00);
   
    /* Select Processing Block PRB_P1 */  
    send_cmd(0x3C, 0x01);

    /* Adaptive Filtering Mode */
    send_cmd(0x00, 0x08);
    send_cmd(0x01, 0x04);
    send_cmd(0x00, 0x00);

    /* 3) Program analog blocks: 
        a) Set register page to 1
        b) Program common-mode voltage (default = 1.35V) 
        c) Program headphone-specific depop settings
        d) Program routing of DAC output to the output amplifier
            (headphone/lineout or speaker)
        e) Unmute and set gain of output driver
        f) Power up output drivers */

    /* Set register page to 1 */
    send_cmd(0x00, 0x01);

    /* NOTE: read up on De-pop and routing settings */
    /* De-pop, Power on = 800ms, Step time = 4ms */
    send_cmd(0x21, 0x4E);
  
    /* Do want to route it to HPL/HPR of L/R mixer amps? */ 
    /* LDAC routed to LMA out, RDAC routed to RMA out */
    send_cmd(0x23, 0x88); 

    /* Unmute HPL and HPR and set gain to 0dB, Class-D gain 18db */
    send_cmd(0x28, 0x06);
    send_cmd(0x29, 0x06);
    send_cmd(0x2A, 0x14);

    /* Power up output drivers */
    send_cmd(0x1F, 0xC6);
    send_cmd(0x20, 0x86);
    /* Enable HPL/HPR, Class-D output analog volume, set = -9 dB */
    send_cmd(0x24, 0x92);
    send_cmd(0x25, 0x92);
    send_cmd(0x26, 0x92);

    /* 4) Apply waiting time determined by the de-pop settings and the soft
        -stepping settings of the driver gain or poll page 1 / register 63 */
    /* TODO: delay here?*/
    delay(1000);

    /* 5) Power up DAC
        a) Set register page to 0
        b) Power up DAC channels and set digital gain
        c) Unmute digital volume control
    */

    /* TODO: check these values */
    send_cmd(0x00, 0x00);
    send_cmd(0x3F, 0xD4);   /* power up dac left and right channels */
    send_cmd(0x41, 0xD4);   /* DAC left gain = -22 dB */
    send_cmd(0x42, 0xD4);   /* DAC right gain = -22 dB */
    send_cmd(0x40, 0x00);   /* Unmute DAC left and right channels */
}
