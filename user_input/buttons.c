#include "buttons.h"
#include "peripherals/exti.h"
#include "peripherals/gpio.h"
#include <stdio.h>

#define BIT(x) (1U << (x))

static void df_hdlr(void *ctx) {
    (void) ctx;
}

/* Change size and contents to add more buttons. */
#define ARR_SIZE        (6)
#define DEF_THRESH      (5)
#define DEF_TIMER_MS    (10)
BUTTON button_arr[ARR_SIZE] = {
    {PIN('A', 0), HIGH, IDLE, 0, DEF_THRESH, {0}, df_hdlr, df_hdlr},
    {0, HIGH, IDLE, 0, DEF_THRESH, {0}, df_hdlr, df_hdlr},
    {0, HIGH, IDLE, 0, DEF_THRESH, {0}, df_hdlr, df_hdlr},
    {0, HIGH, IDLE, 0, DEF_THRESH, {0}, df_hdlr, df_hdlr},
    {0, HIGH, IDLE, 0, DEF_THRESH, {0}, df_hdlr, df_hdlr},
    {0, HIGH, IDLE, 0, DEF_THRESH, {0}, df_hdlr, df_hdlr}
};

void EXTI0_Handler() {
    EXTI->PR = BIT(0);
    if (button_arr[UP].state == IDLE) {
        button_arr[UP].state = BOUNCING;
    }
}

void init_buttons() {
    BUTTON *button;
    for (int i = 0; i < ARR_SIZE; i++) { 
        button = &button_arr[i];
        init_timer_t(&button->timer, DEF_TIMER_MS);
    }
}

/* TODO: debouncing algorithm */
void debounce_buttons() {
    BUTTON *button;
    uint8_t raw_signal;
    /* Loop through all buttons. */
    for (int i = 0; i < ARR_SIZE; i++) {
        button = &button_arr[i];
        
        /* NOTE: Check periodic timer here. */
        if (button->state == BOUNCING && timer_expired(&button->timer)) {
            raw_signal = gpio_read(button->pin);
            if (raw_signal == button->value) {
                button->counter = 0;
            }
            else {
                button->counter++;
                if (button->counter >= button->threshold) {
                    button->value = raw_signal;
                    button->state = CHANGED;
                }
            }
        }

        /* TODO: may want to move this elsewhere
                    and hook up press/release functions

        */
        if (button->state == CHANGED) {
            printf("button event\r\n");
            button->state = IDLE;
        }
    }
}
