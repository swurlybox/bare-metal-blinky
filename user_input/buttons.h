/* Generic API for keeping track of button state. */
#ifndef BUTTONS_H
#define BUTTONS_H

#include <stdint.h>
#include "peripherals/systick.h"

typedef enum {LOW = 0, HIGH } BUTTON_VAL;
typedef enum {IDLE = 0, BOUNCING, CHANGED} BUTTON_STA;

/* NOTE: Used to index into button_arr. */
enum {UP = 0, DOWN, LEFT, RIGHT, SELECT, CANCEL};

typedef struct {
    uint16_t pin;
    BUTTON_VAL value;           /* changed by main execution flow */
    volatile BUTTON_STA state;  /* changes from an interrupt */
    volatile uint8_t counter;   /* debounce counter */
    volatile uint8_t sum;       /* filters out bounce noise */ 
    uint8_t threshold;          /* threshold for consistent signal */
    struct timer_t timer;
    void (*press)(void *);
    void (*release)(void *);
} BUTTON;

extern BUTTON button_arr[]; /* look in buttons.c to see exact contents */

void user_input_init();     /* configure gpio pins for external interrupts */
void debounce_buttons();    /* debouncing algorithm. */
void change_press(BUTTON *button, void (*newfcn)(void *));
void change_release(BUTTON *button, void (*newfcn)(void *));

#endif
