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
#include "peripherals/i2c.h"

#include "states/ff_nav.h"
#include "states/gbl_ctx.h"

#include "device_drivers/usd_card.h"
#include "device_drivers/fatfs_module/ff.h"
#include "device_drivers/SSD1306/ssd1306_driver.h"

#include "user_input/buttons.h"

#include "setup/nvic.h"

#include <stdio.h>
#include <math.h>

#define CLOCK_SPEED (16000000)
#define TICKS_PER_MILLISECOND (CLOCK_SPEED / 1000)
#define SECTOR_SIZE (512)
#define CWD_MAX_LEN (512)
#define LAZYMOUNT   (0)

#define DOWN    (0)
#define UP      (1)

GBL_CTX_T ctx;

void system_init(void) {
    systick_init(TICKS_PER_MILLISECOND);    /* systick */
    uart_init(USART2, 115200);              /* usart */
    spi_init();                             /* spi */
    i2c1_init();

    /* check display is working */
    display_init();
    display_update();

    user_input_init();  /* Enable button interrupts, no events hooked yet. */ 

    /* PWM Status LED: TIM2 on PA15 (Pin 17 CN7) */
    timer_init();                       /* TIM2 Frequency Timer */
    uint16_t led = PIN('A', 15);
    gpio_set_mode(led, GPIO_MODE_AF);
    gpio_set_af(led, 1);

    /* Setup GBL_CTX (global context) object.  */ 
    f_mount(&ctx.fs, "", LAZYMOUNT);
    ctx.status = 1;
    ctx.execute = ff_nav_main;  // TODO: hook to ff_nav_main() later
}

typedef struct {
    struct timer_t timer;   /* periodic timer used to adjust duty cycle. */
    uint8_t direction;
    uint32_t duty_cycle;
} STATUS_LED_T;

void status_led_fade(STATUS_LED_T *led) {
    if(timer_expired(&led->timer)) {
        /* Set direction. */
        if (led->duty_cycle >= 100) {
            led->direction = DOWN;
        }
        if (led->duty_cycle == 0) {
            led->direction = UP;
        }

        /* Adjust duty cycle. */
        if (led->direction == UP) {
            led->duty_cycle++;
        }
        if (led->direction == DOWN) {
            led->duty_cycle--;
        }
        timer_pwm_set_duty_cycle((float) led->duty_cycle);
    }
}

int main(void) {
    /* Peripheral and hardware initializations. */
    system_init();
    
    /* Any other extraneous initializations. */ 
    STATUS_LED_T status_led = {
        .timer = {0}, 
        .direction = UP, 
        .duty_cycle = 0
    };
    init_timer_t(&status_led.timer, 10);

    for (;;) {
        /* STATUS LED: Fade in and out */
        status_led_fade(&status_led); 

        /* Call ctx.execute() */
        ctx.execute(NULL);

        /* NOTE: PA0 for first button interrupt test. CN7 Pin 28 */ 

 
        /* Do other work */ 
    }
    return 0;
}


