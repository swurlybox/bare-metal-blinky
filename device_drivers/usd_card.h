/* API for talking to an external microSD flash memory. 
    Usually the underlying hardware interface is SPI, but we're hiding
    that hardware interface detail by writing this device driver. */
#ifndef USD_CARD_H
#define USD_CARD_H

#include <stdint.h>

void sd_card_init(void);

/* TODO: Generic read write operations that I think might be useful? */
void sd_card_write(uint32_t address, uint8_t *databuf, uint32_t size);
void sd_card_read(uint32_t address, uint8_t *srcbuf, uint32_t size);

#endif
