/* 
    Main application logic and startup code.
*/
#include "gpio.h"
#include "systick.h"
#include "usart.h"

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
    init_timer_t(&timer, 1000);
    for (;;) {
        if(timer_expired(&timer)) {
            static bool on;
            gpio_write(led, on);
            on = !on;
            if (on) {
                uart_write_buf(USART2, "on\r\n", 4);
            } else {
                uart_write_buf(USART2, "off\r\n", 5);
            }
        }
        // We can do other stuff here.

        /*
        gpio_write(led, true);
        delay(1000);
        gpio_write(led, false);
        delay(1000);
        */
    }
    return 0;
}


