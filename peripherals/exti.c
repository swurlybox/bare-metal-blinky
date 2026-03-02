#include "exti.h"
#include "rcc.h"
#include "syscfg.h"
#include "gpio.h"

#define BIT(x) (1U << (x))

void exti_enable(uint16_t pin, uint8_t trigger) {
    uint8_t pinno =     (uint8_t) PINNO(pin);       /* 0-15 */
    uint8_t pinbank =   (uint8_t) PINBANK(pin);     /* A-H mapped to 0-7*/
    uint32_t *crx_ptr;

    RCC->APB2ENR |= BIT(14);                        /* SYSCFG enable bit */
    
    /* SYSCFG: Map pin to external interrupt. */
    crx_ptr =   (uint32_t *) &SYSCFG->EXTICR1;
    crx_ptr =   crx_ptr + (pinno / 4U);             /* Select correct reg. */
    *crx_ptr &=  ~(15U << ((pinno % 4U) * 4U));       /* Clear 4 bits at offst */
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

    RCC->APB2ENR &= ~BIT(14);   /* Turn off SYSCFG clock to save power(?) */
}
