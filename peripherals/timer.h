/* API for interacting with hardware timer peripheral. */
#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

/* TIM2 to TIM5 registers */
struct timer {
    volatile uint32_t   CR1, CR2, SMCR, DIER, SR, EGR, CCMR1, CCMR2,
                        CCER, CNT, PSC, ARR, RESERVED0, CCR1, CCR2, CCR3,
                        CCR4, RESERVED1, DCR, DMAR, TIM2_OR, TIM5_OR;
};

#define TIM2 ((struct timer * ) (0x40000000))

void timer_init(void);
void timer_pwm_set_duty_cycle(float duty_cycle);

#endif
