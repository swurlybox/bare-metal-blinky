#ifndef RTC_H
#define RTC_H

#include <stdint.h>

struct rtc {
    volatile uint32_t TR, DR, CR, ISR, PRER, WUTR, CALIBR, ALRMAR,
        ALRMBR, WPR, SSR, SHIFTR, TSTR, TSDR, TSSSR, CALR, TAFCR,
        ALRMASSR, ALRMBSSR, BKPR[20];
};

#define RTC ((struct rtc *) 0x40002800)

/* RCC->BDCR Bit 15 RTCEN RTC Clock Enable
   RCC->BDCR Bit 9:8 RTCSEL[1:0]: RTC clock source selection
        may want to choose LSI for the 32kHz oscillator.
        '10'

*/
void rtc_init(void);
/* TODO: Gettime function */

#endif
