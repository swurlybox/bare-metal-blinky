/*
    API for interfacing with the SysTick Interrupt peripheral. systick_init
    must be called first before using this API, as all functions in this API
    depends on SysTick peripheral to be initialized first.
*/
#ifndef SYSTICK_H
#define SYSTICK_H

#include <stdint.h>
#include <stdbool.h>

struct systick {
    volatile uint32_t CTRL, LOAD, VAL, CALIB;
};

void systick_init(uint32_t ticks);
void delay(unsigned ms);

/* ---------------------------------------- */

struct timer_t {
    uint32_t period, expiry;
    uint8_t overflow_fl;
};

uint32_t get_s_ticks(void);
void init_timer_t(struct timer_t *timer, uint32_t period);
bool timer_expired(struct timer_t *timer);

#endif
