/* API for talking to an external microSD flash memory. 
    Usually the underlying hardware interface is SPI, but we're hiding
    that hardware interface detail by writing this device driver. */
#ifndef USD_CARD_H
#define USD_CARD_H

#include <stdint.h>

void sd_card_init(void);

/* TODO: Generic read write operations that I think might be useful? 
    Should only ever do multi-writes/reads as single-write transactions
    wear our flash memory more compared to multi-write, whereas multi-read
    is more efficient than multiple single-reads.
*/
// NOTE: See CMD13, returns SSR register
uint8_t sd_card_get_status(void);
// NOTE: SEE CMD25
uint8_t sd_card_multi_write(uint32_t LBA, uint8_t *databuf, uint32_t sec_cnt);
// NOTE: SEE CMD18
uint8_t sd_card_multi_read(uint32_t LBA, uint8_t *srcbuf, uint32_t sec_cnt);

#endif
