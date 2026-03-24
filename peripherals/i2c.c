#include "i2c.h"
#include "rcc.h"
#include "gpio.h"

#include <stdio.h>  /* debugging */

#define BIT(x) (1U << (x))

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
    I2C1->CR2 &= ~(0b11111U);
    I2C1->CR2 |= 0b10000U;      /* frequency*/
    
    /* 
    sm mode wants 100khz, which is 10000ns period
    t_low and t_high should be half of that period, so 
    5000ns each.

    according to the reference manual:
    t_high = CCR * tpclk
    t_low = CCR * tpclk

    our tpclk is the period of the peripheral clock. Its fed 16MHz, which
    is tpclk = 62.5ns period.

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
void i2c1_transmit(uint8_t address, uint8_t *data, uint32_t buf_size) {
    uint32_t dummy_byte;

    /* generate start condition */
    i2c1_start();
    while (!(I2C1->SR1 & BIT(0))) { (void) 0; }

    /* send address */
    i2c1_write(address << 1);
    while (!(I2C1->SR1 & BIT(1))) { (void) 0; }
    dummy_byte = I2C1->SR2;
    (void) dummy_byte;      /* read of SR1 and SR2 clears ADDR */

    /* NOTE: data transfer here, behavior changes w/ DMA enabled. */
    /* If we do decide to use DMA here, the code structure will change a 
        little bit. 
    
        Issue a DMA request to have DMA send the display buffer contents
        via I2C. If the DMA is currently busy handling a previous request,
        we can probably just drop the request entirely. With the display
        update, we're essentially writing 1024 bytes everytime.

        Then we can just exit this function. And do other tasks.

        When DMA request is done, an interrupt will be generated, then in that
        interrupt we can wait for BTF and then send a stop condition.

        I don't know, I think we'll hold off on DMA unless our performance
        loss is very noticable.
        
        Nah fuck that mentality, we'll try DMA anyway.
    */
    /* send data bytes until buf_size exhausted */
    while(buf_size-- > 0) {
       i2c1_write(*data++); 
       while(!(I2C1->SR1 & BIT(7))) { (void) 0; }   /* wait for TxE = 1 */
    }
    while(!(I2C1->SR1 & BIT(2))) { (void) 0; }  /* wait for BTF = 1 */

    /* stop condition */
    i2c1_stop();
}

void i2c1_start(void) {
    I2C1->CR1 |= BIT(8);
}

void i2c1_stop(void) {
    I2C1->CR1 |= BIT(9);
}

void i2c1_write(uint8_t byte) {
    I2C1->DR = (uint32_t) byte;
}
