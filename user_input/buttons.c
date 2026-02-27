#include "buttons.h"
#include "peripherals/exti.h"
#include <stdio.h>

#define BIT(x) (1U << (x))

/* NOTE: Debouncing issue is real. I don't think we can stop the interrupts
    from firing on the debounced portion of the button presses. If the NVIC
    sees a rising or falling edge, it'll set the pending bit, then the ISR
    is called. */
void EXTI0_Handler() {
    /* clear interrupt flag */
    static volatile int count = 0;
    EXTI->PR = BIT(0);

    /* NOTE: We're going to need a timer-based solution, as well as read
    the raw signal on the GPIO pin. */

    printf("button event: %d\r\n", count++);
}
