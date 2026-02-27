#include "nvic.h"
#include <stdio.h>

#define BIT(x) (1U << (x))

void nvic_status(void) {
    uint8_t intlinesnum = (NVIC->ICTR & 15U);
    printf("Supported number of interrupts: %u\r\n", (intlinesnum + 1) * 32U);
}

void enable_irq(uint32_t intno) {
    /* set the corresponding nvic ISR bit */
    uint32_t *base = (uint32_t *) &NVIC->ISER[0];
    base = base + (intno / 32U);    /* offset into correct register. */
    *base |= BIT(intno % 32U);       /* enable the interrupt */
}
