#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
/*
    We want to use a timer for PWM. 
    In order to do this, we need to set a couple of registers in our timer
    peripheral.

    Once the timer is configured properly, it'll generate an interrupt every
    time the comparison value changes. We 

*/

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
