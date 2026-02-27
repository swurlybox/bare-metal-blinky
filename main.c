/* 
    Main application logic and peripheral initializations.
*/
#include "peripherals/gpio.h"
#include "peripherals/systick.h"
#include "peripherals/usart.h"
#include "peripherals/timer.h"
#include "peripherals/spi.h"
#include "peripherals/syscfg.h"
#include "peripherals/exti.h"

#include "device_drivers/usd_card.h"
#include "device_drivers/fatfs_module/ff.h"

#include "setup/nvic.h"

#include <stdio.h>
#include <math.h>

#define EXTI0_INTERRUPT_NO  (6)
#define CLOCK_SPEED (16000000)
#define TICKS_PER_MILLISECOND (CLOCK_SPEED / 1000)
#define SECTOR_SIZE (512)
#define CWD_MAX_LEN (512)
#define LAZYMOUNT   (0)

#define DOWN    (0)
#define UP      (1)

FATFS fs;

void system_init(void) {
    systick_init(TICKS_PER_MILLISECOND); 
    uart_init(USART2, 115200);      /* sent to PC terminal */
    spi_init();                     
    f_mount(&fs, "", LAZYMOUNT);    /* mount sd_card filesystem */
  
    /* setup external interrupts for user_input */ 
    syscfg_init();
    
    /* setup external interrupt on test button. */
    uint16_t test_button = PIN('A', 0);
    gpio_set_mode(test_button, GPIO_MODE_INPUT);
    exti_enable(test_button, FALLING_EDGE | RISING_EDGE);

    /* TODO: Enable IRQ */
    nvic_status();
    enable_irq(EXTI0_INTERRUPT_NO);       /* EXTI0 is interrupt no. 6*/

    /* Status LED: TIM2 on PA15 (Pin 17 CN7) */
    timer_init();
    uint16_t led = PIN('A', 15);
    gpio_set_mode(led, GPIO_MODE_AF);
    gpio_set_af(led, 1);
}

int main(void) {
    system_init(); 

    struct timer_t timer;
    uint32_t duty_cycle = 0;
    int8_t direction = UP;
    init_timer_t(&timer, 10);   /* create periodic timer */
    for (;;) {
        /* STATUS LED: Fade in and out */
        if(timer_expired(&timer)) {
            if (duty_cycle >= 100) {
                direction = DOWN;
            }
            if (duty_cycle == 0) {
                direction = UP;
            }

            if (direction == UP) {
                duty_cycle++;
            }
            if (direction == DOWN) {
                duty_cycle--;
            }
            timer_pwm_set_duty_cycle((float) duty_cycle); 
        }

        /* NOTE: PA0 for first button interrupt test. CN8 Pin 1 

            9-5 and 15-10 are grouped interrupt lines that map to one
            handler. It's a bit annoying to untangle it and determine the
            source of the interrupt. So I'll stick with the single interrupt
            -per-pin lines.
        */
        




        /* Poll for button press event, or use a pure interrupt to manage
            filesystem navigation state. 

           Button press will trigger an interrupt, setting a global shared
           button_pressed variable. If the button_pressed variable hasn't
           been cleared, that means the button press hasn't been handled yet.
           Subsequent button press interrupts will have no effect on the
           button_pressed variable until it has been handled and cleared.
           
           To deal with the possibility of debouncing, we check for a
           consistent signal for a period of time after the button_press
           event, then set the button_press variable. */

        /* However, start with getting a simple button interrupt working. */


        /* Do other work */ 
    }
    return 0;
}


