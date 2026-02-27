#include "exti.h"
#include "syscfg.h"
#include "gpio.h"

#define BIT(x) (1U << (x))

void exti_enable(uint16_t pin, uint8_t trigger) {
    uint8_t pinno =     (uint8_t) PINNO(pin);         /* 0-15 */
    uint8_t pinbank =   (uint8_t) PINBANK(pin);     /* A-H mapped to 0-7*/
    uint32_t *crx_ptr;

    /* SYSCFG: Map pin to external interrupt. */
    crx_ptr =   (uint32_t *) &SYSCFG->EXTICR1;
    crx_ptr =   crx_ptr + (pinno / 4U);             /* Select correct reg. */
    *crx_ptr &=  (15U << ((pinno % 4U) * 4U));       /* Clear 4 bits at offst */
    *crx_ptr |=  (pinbank << ((pinno % 4U) * 4U));   /* Set 4 bits at offset */

    /* Configure Edge Detection Register */
    if (trigger & RISING_EDGE) {
        EXTI->RTSR |= BIT(pinno);
    }
    if (trigger & FALLING_EDGE) {
        EXTI->FTSR |= BIT(pinno);
    }

    /* Set Interrupt Mask */
    EXTI->IMR |= BIT(pinno);

    /* NOTE: EXTI_PR is the pending bit. Must be cleared by the interrupt
    handler by programming it to 1. It should be the first line of code
    in the interrupt, as the pending bit is what determines if an interrupt
    should be generated or not, and clearing the pending bit takes time
    to propagate throughout the bus to the NVIC. */
}
