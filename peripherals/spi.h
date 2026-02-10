/* API for generic SPI driver to communicate with SPI devices */
#ifndef SPI_H
#define SPI_H

#include <stdint.h>

struct spi {
    volatile uint32_t CR1, CR2, SR, DR, CRCPR, RXCRCR, TXCRCR, I2SCFGR,
        I2SPR;
};

#define SPI2 ((struct spi *) (0x40003800))

void spi_init(void);
uint8_t spi_transfer(uint8_t byte);

#endif
