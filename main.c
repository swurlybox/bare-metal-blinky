/* 
    Main application logic and startup code.
*/
#include "peripherals/gpio.h"
#include "peripherals/systick.h"
#include "peripherals/usart.h"

#include <stdio.h>
#include <math.h>

#define CLOCK_SPEED (16000000)
#define TICKS_PER_MILLISECOND (CLOCK_SPEED / 1000)

int main(void) {
    /* Config SysTick, generate interrupt every ms, needed for delay */
    systick_init(TICKS_PER_MILLISECOND); 

    /* Config USART2 for serial debug output */
    uart_init(USART2, 115200);

    /* Setup GPIO: GPIOC13, Pin 1 of CN7 */
    uint16_t led = PIN('C', 10);                /* vdd pin for led */
    gpio_bank_enable((uint8_t) PINBANK(led));   /* enable bank c*/
    gpio_set_mode(led, GPIO_MODE_OUTPUT);       /* set pin for output */

    /* Blink every second */
    struct timer_t timer;
    init_timer_t(&timer, 100);
    for (;;) {
        /* Timer polling */
        if(timer_expired(&timer)) {
            static bool on;
            gpio_write(led, on);
            on = !on; 
            printf("LED: %d, tick: %lu\r\n", on, get_s_ticks());  
        }
        /* Do other work */ 
    }
    return 0;
}


