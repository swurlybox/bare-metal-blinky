#include "buttons.h"
#include "peripherals/exti.h"
#include "peripherals/gpio.h"

#include "setup/nvic.h"

#include <stdio.h>
 
#define EXTI0_INTERRUPT_NO  (6)
#define EXTI1_INTERRUPT_NO  (7)
#define EXTI2_INTERRUPT_NO  (8)
#define EXTI3_INTERRUPT_NO  (9)
#define EXTI4_INTERRUPT_NO  (10)
#define EXTI5_INTERRUPT_NO  (23)

#define BIT(x) (1U << (x))

/* ISRs */
void EXTI0_Handler() {
    EXTI->PR = BIT(0);
    if (button_arr[UP].state == IDLE) {
        button_arr[UP].state = BOUNCING;
    }
}

void EXTI1_Handler() {
    EXTI->PR = BIT(1);
    if (button_arr[DOWN].state == IDLE) {
        button_arr[DOWN].state = BOUNCING;
    }
}

void EXTI2_Handler() {
    EXTI->PR = BIT(2);
    if (button_arr[LEFT].state == IDLE) {
        button_arr[LEFT].state = BOUNCING;
    }
}

void EXTI3_Handler() {
    EXTI->PR = BIT(3);
    if (button_arr[RIGHT].state == IDLE) {
        button_arr[RIGHT].state = BOUNCING;
    }
}

void EXTI4_Handler() {
    EXTI->PR = BIT(4);
    if (button_arr[SELECT].state == IDLE) {
        button_arr[SELECT].state = BOUNCING;
    }
}

void EXTI9_5_Handler() {
    /* Multiple interrupt sources share this interrupt service routine. */
    /* But for our purposes, we only have one source.*/
    EXTI->PR = BIT(5);
    if (button_arr[CANCEL].state == IDLE) {
        button_arr[CANCEL].state = BOUNCING;
    }
}

static void df_hdlr(void *ctx) {
    (void) ctx;
}

static void buttons_timer_init();

/* Change size and contents to add more buttons. */
#define ARR_SIZE        (6)
#define DEF_THRESH      (10)
#define DEF_TIMER_MS    (5)
BUTTON button_arr[ARR_SIZE] = {
    {PIN('A', 0), HIGH, IDLE, 0, 0, DEF_THRESH, {0}, df_hdlr, df_hdlr},
    {PIN('A', 1), HIGH, IDLE, 0, 0, DEF_THRESH, {0}, df_hdlr, df_hdlr},
    {PIN('B', 2), HIGH, IDLE, 0, 0, DEF_THRESH, {0}, df_hdlr, df_hdlr},
    {PIN('B', 3), HIGH, IDLE, 0, 0, DEF_THRESH, {0}, df_hdlr, df_hdlr},
    {PIN('A', 4), HIGH, IDLE, 0, 0, DEF_THRESH, {0}, df_hdlr, df_hdlr},
    {PIN('A', 5), HIGH, IDLE, 0, 0, DEF_THRESH, {0}, df_hdlr, df_hdlr}
};

void button_config_exti(uint16_t pin) {
    gpio_set_mode(pin, GPIO_MODE_INPUT);
    gpio_set_pupd(pin, GPIO_PU);
    exti_enable(pin, FALLING_EDGE | RISING_EDGE);
}

/* Sets up user input according to hardwired jumper configurations on the
    board. This mainly consists of buttons attached to GPIO pins. */
void user_input_init() {
    /* NOTE: See which EXTI lines map to which GPIO pins. This information
        can be found in your MCU's reference manual, under something like
        "External interrupts/event line mappings." */
    
    /* GPIO / EXTI setup. Order should match button_arr. */
    /* NOTE: Could be better and update the button_arr pins dynamically. */
    button_config_exti(PIN('A', 0));    /* up button        */
    button_config_exti(PIN('A', 1));    /* down button      */
    button_config_exti(PIN('B', 2));    /* left button      */
    button_config_exti(PIN('B', 3));    /* right button     */
    button_config_exti(PIN('A', 4));    /* select button    */
    button_config_exti(PIN('A', 5));    /* cancel button    */
    
    /* NVIC enable interrupts. */
    enable_irq(EXTI0_INTERRUPT_NO);
    enable_irq(EXTI1_INTERRUPT_NO);
    enable_irq(EXTI2_INTERRUPT_NO);
    enable_irq(EXTI3_INTERRUPT_NO);
    enable_irq(EXTI4_INTERRUPT_NO);
    enable_irq(EXTI5_INTERRUPT_NO);

    /* Print number of supported interrupts. */
    nvic_status(); 

    /* Initialize periodic timers on buttons. */
    buttons_timer_init();
}

void buttons_timer_init() {
    BUTTON *button;
    for (int i = 0; i < ARR_SIZE; i++) { 
        button = &button_arr[i];
        init_timer_t(&button->timer, DEF_TIMER_MS);
    }
}

void debounce_buttons() {
    BUTTON *button;
    volatile uint8_t raw_signal;
    
    /* Loop through all buttons. */ 
    for (int i = 0; i < ARR_SIZE; i++) {
        button = &button_arr[i];        

        /* Debounce any buttons. */
        if (button->state == BOUNCING && timer_expired(&button->timer)) {
            raw_signal = gpio_read(button->pin);
            //printf("raw signal: %d\r\n", raw_signal);
            button->counter++;
            if (raw_signal != button->value) {
                button->sum++;
            }
            if (button->counter >= button->threshold) {
                if (button->sum == 0) {
                    /* deal with possible inconsistency between raw signal
                        and button value. In which case, go back to idle. */
                    button->state = IDLE;
                } 
                else if (button->sum == button->counter){
                    /* consistent signal, change the buttons's value. */
                    button->value = raw_signal;
                    button->state = CHANGED;
                }
                button->counter = 0;
                button->sum = 0; 
            }
        }

        /* Handle any events. */
        else if (button->state == CHANGED) {
            if (button->value == LOW) {
                printf("Button press: %d\r\n", button->value);
            }
            else {
                printf("Button release: %d\r\n", button->value);
            }
            button->state = IDLE;
        }
    }
}
