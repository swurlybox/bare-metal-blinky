#include "spi.h"
#include "rcc.h"
#include "gpio.h"

/* NOTE: Debugging */
#include <stdio.h>

#define BIT(x) (1U << (x))

/* NOTE: Initializes SPI2 interface with very specific configurations,
    according to my application needs.

    SPI2 Pin Configurations:
        NSS:    PB12
        SCK:    PB13
        MISO:   PB14
        MOSI:   PB15

    Baud Rate:                  250KHz initially, ramped up to 8MHz in SD 
                                initialization.
    Data Frame Format:          8 bits
    MSBFIRST:                   Yes
    Master Mode:                Yes
    Software Slave Management:  Enabled initially to drive CS pin manually,
                                then disabled to let hardware manage CS pin 
                                automatically on data transfers.
*/
void spi_init(void) {
    RCC->APB1ENR |= BIT(14);    /* Enable SPI peripheral clock */

    /* Configure corresponding GPIO pins */
    uint16_t nss, sck, miso, mosi;
    nss = PIN('B', 12);
    sck = PIN('B', 13);
    miso = PIN('B', 14);
    mosi = PIN('B', 15);

    gpio_set_mode(nss, GPIO_MODE_AF);
    gpio_set_mode(sck, GPIO_MODE_AF);
    gpio_set_mode(miso, GPIO_MODE_AF);
    gpio_set_mode(mosi, GPIO_MODE_AF);

    /* 5 is the AF_NUM for SPI */
    gpio_set_af(nss, 5);
    gpio_set_af(sck, 5);
    gpio_set_af(miso, 5);
    gpio_set_af(mosi, 5);

    /* Set SPI-related configurations. NOTE: May want to enable DMA bits l8r */ 
    SPI2->CR1 |= 0b101 << 3U;   /* fpclk/64 ~ 250KHz, which is safe for SD */
    SPI2->CR1 |= BIT(2);        /* Master mode */
    SPI2->CR2 |= BIT(2);        /* SSOE */

    #ifdef DEV_ENV
        /* NOTE: Diagnostics, verify SPI configurations. */
        printf("------ Checking Initial SPI configurations ------\r\n");
        printf("Baud Rate Field Value: %ld\r\n", 
            (SPI2->CR1 & (BIT(3) | BIT(4) | BIT(5))) >> 3U);
        printf("Master mode: %ld\r\n", (SPI2->CR1 & BIT(2)) >> 2U);
        printf("SSOE enable (CS controlled by HW): %ld\r\n", 
            (SPI2->CR2 & BIT(2)) >> 2U);
    #endif 
}

/* Single byte transfer. Nice to have for its granularity. */
uint8_t spi_transfer(uint8_t tx_data) {
    uint8_t rx_data = 0;
    SPI2->CR1 |= BIT(6);                /* Enable SPI communication */
    SPI2->DR = (uint32_t) (tx_data);    /* TX_buffer transmit, toggles SCLK */
    while ((SPI2->SR & BIT(7)) || !(SPI2->SR & BIT(0))) {
        (void) 0;   /* Wait till RX_buffer non-empty and SPI isn't busy. */
    } 
    rx_data = (uint8_t) SPI2->DR;       /* Read RX_buffer, clears RXNE bit */
    SPI2->CR1 &= ~BIT(6);               /* Disable SPI communication */
    return rx_data;
}
