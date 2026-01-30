#include "timer.h"
#include "rcc.h"

#define BIT(x) (1U << (x))
#define CLOCK_SPEED (16000000)
#define ARR_VAL     (1000)
#define TIMER_FREQ  (1000)

/* Enables TIM2 Channel 1 for PWM output
    w/ counter increment frequency 1KHz and 1000 ARR
    duty cycle granularity */
void timer_init(void) {
    /* Enable the clock to this peripheral (RCC) */
    RCC->APB1ENR |= BIT(0); 

    /* Set up our timer's mode */
    TIM2->CCMR1 &= ~(15U);  /* Reset channel 1, default mode output */
    TIM2->CCMR1 |= (6U << 4) | BIT(3); /* Set PWM mode, enable preload */

    /* Set the prescaler and ARR initial values */
    TIM2->PSC = 15; /* +1 internally to equal 16 (avoids divide by 0 error) */
    TIM2->ARR = ARR_VAL - 1; /* including 0 so -1 */
    TIM2->CCR1 = TIM2->ARR / 2;

    TIM2->EGR |= BIT(0);    /* update generation, re-init counter/registers*/

    /* Enable capture and compare register */
    TIM2->CCER |= BIT(0);   /* OC1 Signal output on corresponding output pin */

    /* Enable the counter preload and counter itself */
    TIM2->CR1 |= BIT(7) | BIT(0);
}

void timer_pwm_set_duty_cycle(float duty_cycle) {
    uint32_t raw_value = (uint32_t) (duty_cycle * (float) ARR_VAL / 100.0F);
    TIM2->CCR1 = raw_value;
};
