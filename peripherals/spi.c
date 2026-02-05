#include "spi.h"
#include "rcc.h"
#include "gpio.h"

#define BIT(x) (1U << (x))

/* Initialized with a few default configurations. */
void spi_init(void) {
    /* Enable SPI peripheral clock */
    RCC->APB1ENR |= BIT(14);

    /* Decide which SPI interface we want to use */
    /* SPI2:
            PB12, 13, 14, 15; NSS, SCK, MISO, MOSI*/

    /* Configure the corresponding GPIO pins */
    uint16_t nss, sck, miso, mosi;
    nss = PIN('B', 12);
    sck = PIN('B', 13);
    miso = PIN('B', 14);
    mosi = PIN('B', 15);

    gpio_set_mode(nss, GPIO_MODE_AF);
    gpio_set_mode(sck, GPIO_MODE_AF);
    gpio_set_mode(miso, GPIO_MODE_AF);
    gpio_set_mode(mosi, GPIO_MODE_AF);

    /* AF5 is the SPI alternate functions */
    gpio_set_af(nss, 5);
    gpio_set_af(sck, 5);
    gpio_set_af(miso, 5);
    gpio_set_af(mosi, 5);

    /* Set any SPI-related configurations */
    /* 
        Serial Clock Baud Rate: f_pclk / 2
        Data Frame Format: 8 bits
        MSBFIRST (see if we need to change)

        May want to enable Tx/Rx buffer DMA bits in CR2 
    */
    SPI2->CR1 |= BIT(2);    /* Master mode */
    SPI2->CR2 |= BIT(2);    /* SS output enabled */
}

/* Repeatedly call spi_transfer. Sends bytes from srcbuf over MOSI.
   Stores bytes from MISO to dstbuf. Until size is exhausted.

    May need to write dummy bytes to srcbuf if needed. Srcbuf and dstbuf
    should ideally be the same size, as a transaction requires writing
    for each byte.

   TODO: Test
*/
uint8_t spi_block_transfer(uint8_t *srcbuf, uint8_t *dstbuf, uint32_t size) {
    (void) srcbuf, (void) dstbuf, (void) size;

    /* 
        We're given two pointers:
        - srcbuf: which points to the bytes of data we want to send.
        - dstbuf: which points to the bytes of data we want to store.
        - size:   the length in bytes of the data we want to send and receive.
    */

    /* NOTE: The little caveat detailed below doesn't seem very helpful to us,
        maybe there is a better way to extract the specific data we want. 
        But this function is intended to be very generalized. */
    while (size > 0) {
        /* This means there may be some dummy data in our dst and srcbufs. 
            The caller of this function will need to know where the data
            that we want lies within these buffers. */
        *dstbuf = spi_transfer(*srcbuf);
        dstbuf++, srcbuf++, size--;
    }

    return 0;
}

/* TODO: Test */
/* Single byte transfer. Nice to have for its granularity. */
uint8_t spi_transfer(uint8_t tx_data) {
    uint8_t rx_data = 0;
    
    // Enable SPI communication
    SPI2->CR1 |= BIT(6);
    
    // Write data to the TX_buffer NOTE: Starts the CLK once transmitted.
    SPI2->DR = (uint32_t) (tx_data);
    
    // Wait until SPI is not busy and RX Buffer is not empty.
    while ((SPI->SR & BIT(7)) || !(SPI->SR & BIT(0))) {
        (void) 0;
    }
    
    // Store the received data.
    rx_data = (uint8_t) SPI2->DR;

    // Disable SPI communication.
    SPI2->CR1 &= ~BIT(6);

    return rx_data;
}
