#include "i2c.h"
#include "rcc.h"
#include "gpio.h"

#include <stdio.h>  /* debugging */

#define BIT(x) (1U << (x))

static void i2c_start(I2C_Handle *p);
static void i2c_stop(I2C_Handle *p);
static void i2c_write(I2C_Handle *p, uint8_t byte);

/* TODO: refactor for i2c3: PA8 scl, PB4 sda */
void i2c3_init() {
    RCC->APB1ENR |= BIT(23);

    uint16_t scl = PIN('A', 8);
    uint16_t sda = PIN('B', 4);

    gpio_set_mode(scl, GPIO_MODE_AF);
    gpio_set_mode(sda, GPIO_MODE_AF);
    gpio_set_otype(scl, GPIO_OTYPE_OD);
    gpio_set_otype(sda, GPIO_OTYPE_OD);
    gpio_set_pupd(scl, GPIO_PU);
    gpio_set_pupd(sda, GPIO_PU);
    gpio_set_af(scl, 4);
    gpio_set_af(sda, 4);
  
    /* Set the peripheral clock frequency to 16MHz*/ 
    I2C3->CR2 &= ~(0b111111U);
    I2C3->CR2 |= 0b10000U; 

    /* Clock control settings related for SCL. These registers' values are set
    according to which I2C mode you want (Fm/Sm mode) and the peripheral
    clock frequency. NOTE: DON'T TOUCH. See RM for details and formulas. */
    I2C3->CCR |= 0x50;
    I2C3->TRISE &= ~(0b11111U);
    I2C3->TRISE |= 0b10001U;

    I2C3->CR1 |= BIT(0);
    printf("i2c3 SR1, SR2: %lu, %lu\r\n", I2C3->SR1, I2C3->SR2);
    printf("i2c3 CR1, CR2: %lu, %lu\r\n", I2C3->CR1 & 0xFFFF, I2C3->CR2);
    printf("i2c3 CCR, TRISE: %lu, %lu\r\n", I2C3->CCR, I2C3->TRISE);
}


void i2c1_init() {
    RCC->APB1ENR |= BIT(21);

    uint16_t scl = PIN('B', 8);
    uint16_t sda = PIN('B', 9);

    gpio_set_mode(scl, GPIO_MODE_AF);
    gpio_set_mode(sda, GPIO_MODE_AF);
    gpio_set_otype(scl, GPIO_OTYPE_OD);
    gpio_set_otype(sda, GPIO_OTYPE_OD);
    gpio_set_pupd(scl, GPIO_PU);
    gpio_set_pupd(sda, GPIO_PU);
    gpio_set_af(scl, 4);
    gpio_set_af(sda, 4);

    /* NOTE: Enable DMA eventually. */
    I2C1->CR2 &= ~(0b111111U);
    I2C1->CR2 |= 0b10000U;      /* frequencyi */
    
    /* 
    sm mode wants 100khz, which is 10000ns period
    t_low and t_high should be half of that period, so 
    5000ns each.

    according to the reference manual:
    t_high = CCR * tpclk
    t_low = CCR * tpclk

    our tpclk is the period of the peripheral clock. Its fed 16MHz, which
    is tpclk = 62.5ns period.
    
    but if we leave freq a lower value than apb1 clock, i think its fine?


    NOTE: new fclk = 32MHz. tpclk = 31.25ns

    5000/31.25 = 160d

    so that means CCR should be t_high / tpclk (5000 / 62.5) which
    nets us 80d -> 50h
    */
    I2C1->CCR |= 0x50;  /* t_high + t_low = t_scl */

    I2C1->TRISE &= ~(0b11111U);
    I2C1->TRISE |= 0b10001U;

    I2C1->CR1 |= BIT(0);    /* enable peripheral */
    printf("i2c1 SR1, SR2: %lu, %lu\r\n", I2C1->SR1, I2C1->SR2);
    printf("i2c1 CR1, CR2: %lu, %lu\r\n", I2C1->CR1 & 0xFFFF, I2C1->CR2);
    printf("i2c1 CCR, TRISE: %lu, %lu\r\n", I2C1->CCR, I2C1->TRISE);
}

/* with DMA enabled, we make a DMA request rather than have processor
    do the data transfer.  */
void i2c_transmit(I2C_Handle *p, uint8_t address, uint8_t *data,
    uint32_t buf_size) {

    uint32_t dummy_byte;

    /* Generate start condition */
    i2c_start(p);
    while (!(p->SR1 & BIT(0))) { (void) 0; }

    /* Send address */
    i2c_write(p, address << 1);
    while (!(p->SR1 & BIT(1))) { (void) 0; }
    dummy_byte = p->SR2;
    (void) dummy_byte;      /* read of SR1 and SR2 clears ADDR */

    /* Send data bytes until buf_size exhausted */
    while(buf_size-- > 0) {
       i2c_write(p, *data++); 
       while(!(p->SR1 & BIT(7))) { (void) 0; }   /* wait for TxE = 1 */
    }
    while(!(p->SR1 & BIT(2))) { (void) 0; }  /* wait for BTF = 1 */

    /* Generate stop condition */
    i2c_stop(p);
}

static void i2c_start(I2C_Handle *p) {
    p->CR1 |= BIT(8);
}

static void i2c_stop(I2C_Handle *p) {
    p->CR1 |= BIT(9);
}

static void i2c_write(I2C_Handle *p, uint8_t byte) {
    p->DR = (uint32_t) byte;
}
