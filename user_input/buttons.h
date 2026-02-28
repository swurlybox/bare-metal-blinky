/* Generic API for keeping track of button state. */
#ifndef BUTTONS_H
#define BUTTONS_H

#include <stdint.h>
#include "peripherals/systick.h"

/* 0 for active low, means pressed. */
typedef enum {LOW, HIGH} BUTTON_VAL;
typedef enum {IDLE, BOUNCING, CHANGED} BUTTON_STA;

/* NOTE: Used to index into button_arr. */
enum {UP = 0, DOWN, LEFT, RIGHT, SELECT, CANCEL};

typedef struct {
    uint16_t pin;
    BUTTON_VAL value;           /* changed by main execution flow */
    volatile BUTTON_STA state;  /* changes from an interrupt */
    uint8_t counter;            /* debounce counter */ 
    uint8_t threshold;          /* threshold for consistent signal */
    struct timer_t timer;
    void (*press)(void *);
    void (*release)(void *);
} BUTTON;

extern BUTTON button_arr[]; /* look in buttons.c to see exact contents */

void init_buttons();        /* initialize timer */
void debounce_buttons();    /* debouncing algorithm. */
void change_press(BUTTON *button, void (*newfcn)(void *));
void change_release(BUTTON *button, void (*newfcn)(void *));

#endif
