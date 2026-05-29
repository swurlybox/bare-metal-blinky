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
#include "peripherals/rcc.h"

#include "states/ff_nav.h"
#include "states/audio_playback.h"
#include "states/gbl_ctx.h"

#include "device_drivers/usd_card.h"
#include "device_drivers/fatfs_module/ff.h"
#include "device_drivers/SSD1306/ssd1306_driver.h"
#include "device_drivers/TLV320DAC3100/tlv320dac3100_driver.h"

#include "user_input/buttons.h"

#include "setup/nvic.h"

#include <stdio.h>
#include <math.h>

#define CLOCK_SPEED (44000000)
#define TICKS_PER_MILLISECOND (CLOCK_SPEED / 1000)
#define SECTOR_SIZE (512)
#define CWD_MAX_LEN (512)
#define LAZYMOUNT   (0)

#define DOWN    (0)
#define UP      (1)

#define BIT(x) (1U << x)

GBL_CTX_T ctx;

void system_init(void) {
    /* try amping up APB1 clock speed to 40MHz */
    /* Use PLL_R, set N to be 80, gets divided by R=2 to get 40MHz sysclk */
    RCC->PLLCFGR &= ~(0b111111111U << 6);
    RCC->PLLCFGR |= (88U << 6); /* set pll_n to 80 */
    RCC->CR |=  BIT(24);         /* main pll on */
    RCC->CFGR   |= 0b11;        /* set pll_r as sysclk */

    /* Internal peripheral initializations. */
    systick_init(TICKS_PER_MILLISECOND);    /* systick */
    uart_init(USART2, 115200);              /* usart */
    spi_init();                             /* spi */
    i2c1_init();
    i2c3_init();    /* TEST: newly added */
    i2s3_init();    /* initialize i2s for audio playback*/

    /* External peripheral/device intiializations. */
    display_init();
    display_update();
    user_input_init();  /* Enable button interrupts, no events hooked yet. */ 
    dac_init();

    /* Software-side structure initializations. */
    f_mount(&ctx.fs, "", LAZYMOUNT);
    ctx.status = 1;     /* first entry to state flag. */

    ctx.execute = ff_nav_main;  // TODO: hook to ff_nav_main() later
}

typedef struct {
    struct timer_t timer;   /* periodic timer used to adjust duty cycle. */
    uint8_t direction;
    uint32_t duty_cycle;
} STATUS_LED_T;

void status_led_blink(uint16_t led, struct timer_t *timer) {
    static int on = 1;
    if (timer_expired(timer)) {
            gpio_write(led, on);
            on = !on;
    }
}

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
    struct timer_t timer;
    init_timer_t(&timer, 1000);
    uint16_t led = PIN('A', 6);
    gpio_set_mode(led, GPIO_MODE_OUTPUT);
    gpio_set_otype(led, GPIO_OTYPE_PP);
    gpio_set_pupd(led, GPIO_PU);    

    printf("Entering main loop\r\n");
    for (;;) {
        status_led_blink(led, &timer);
        ctx.execute(NULL);              /* state-specific main */
        /* do other work */
    }
    return 0;
}


