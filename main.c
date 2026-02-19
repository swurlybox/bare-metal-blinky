/* 
    Main application logic and peripheral initializations.
*/
#include "peripherals/gpio.h"
#include "peripherals/systick.h"
#include "peripherals/usart.h"
#include "peripherals/timer.h"
#include "peripherals/spi.h"
#include "device_drivers/usd_card.h"
#include "device_drivers/fatfs_module/ff.h"

#include <stdio.h>
#include <math.h>

#define CLOCK_SPEED (16000000)
#define TICKS_PER_MILLISECOND (CLOCK_SPEED / 1000)
#define SECTOR_SIZE (512)
#define CWD_MAX_LEN (512)

/* List contents of a directory : Copied from elm chaN */
FRESULT list_dir (const char *path)
{
    FRESULT res;
    DIR dir;
    FILINFO fno;
    int nfile, ndir;

    res = f_opendir(&dir, path);                   /* Open the directory */
    if (res == FR_OK) {
        nfile = ndir = 0;
        for (;;) {
            res = f_readdir(&dir, &fno);           /* Read a directory item */
            if (fno.fname[0] == 0) break;          /* Error or end of dir */
            if (fno.fattrib & AM_DIR) {            /* It is a directory */
                printf("   <DIR>   %s\r\n", fno.fname);
                ndir++;
            } else {                               /* It is a file */
                printf("%10lu %s\r\n", fno.fsize, fno.fname);
                nfile++;
            }
        }
        f_closedir(&dir);
        printf("%d dirs, %d files.\r\n", ndir, nfile);
    } else {
        printf("Failed to open \"%s\". (%u)\r\n", path, res);
    }
    return res;
}

void print_buf(uint8_t *buf) {
    for (int i = 0; i < SECTOR_SIZE; i++) {
        printf("%3x ", *buf++);
        if((i + 1) % 16 == 0) {
            printf("\r\n");
        }
    }
}

int main(void) {
    /* Config SysTick, generate interrupt every ms, needed for delay */
    systick_init(TICKS_PER_MILLISECOND); 

    /* Config USART2 for serial debug output */
    uart_init(USART2, 115200);

    /* Init SPI and check initialization output */
    spi_init();

    /* Init SD card */
    sdcard_init();

    /* Hook up TIM2 to PA15 (Pin 17 CN7)*/
    timer_init();
    uint16_t led = PIN('A', 15);            /* vdd pin for led */
    gpio_set_mode(led, GPIO_MODE_AF);       /* set pin for output */
    gpio_set_af(led, 1);                    /* enable TIM2 for A15 */ 
    
    /* Fade in and out */
    struct timer_t timer;
    uint32_t duty_cycle = 0;
    int8_t direction = 1;   // 1 for up, 0 for down
    init_timer_t(&timer, 10);
    for (;;) {
        /* Timer polling */
        if(timer_expired(&timer)) {
            /* We want fade in and out behaviour. */
            if (duty_cycle >= 100) {
                direction = 0;
            }
            if (duty_cycle == 0) {
                direction = 1;
            }
            if (direction == 1) {
                duty_cycle++;
            }
            if (direction == 0) {
                duty_cycle--;
            }
            timer_pwm_set_duty_cycle((float) duty_cycle);
            //printf("LED Duty Cycle: %ld, dir: %d\r\n", duty_cycle, direction);
            /*printf("TIM2 COUNT: %ld, TIM2 CCR: %ld\r\n", TIM2->CNT, 
                TIM2->CCR1);*/
        }
        /* Do other work */ 
    }
    return 0;
}


