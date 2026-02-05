#ifndef SPI_H
#define SPI_H

#include <stdint.h>

#define WRITE_OK    (1)
#define READ_OK     (1)
#define FAIL        (0)

struct spi {
    volatile uint32_t CR1, CR2, SR, DR, CRCPR, RXCRCR, TXCRCR, I2SCFGR,
        I2SPR;
}

#define SPI2 ((struct spi *) (0x40003800))

/* API for generic SPI driver to communicate with SPI devices */
void spi_init(void);
uint8_t spi_transfer(uint8_t byte);
uint8_t spi_block_transfer(uint8_t *srcbuf, uint8_t *dstbuf, uint32_t size);

#endif
