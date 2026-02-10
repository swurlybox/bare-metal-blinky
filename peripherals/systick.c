#include "systick.h"

#include <stddef.h>

#define BIT(x) (1U << (x))
#define CLOCK_SPEED     (16000000)
#define DEFAULT_TICKS   (16000000 / 1000)
#define NO  (0)
#define YES (1)
#define OFF (0)
#define ON  (1)

#define SYSTICK ((struct systick *) (0xE000E010))

static volatile uint8_t is_systick_init = NO;

void systick_init(uint32_t ticks) {
    if(ticks - 1 > 0xFFFFFF) return;    // SysTick counter is a 24-bit value.    
    SYSTICK->LOAD = ticks - 1;                  // Set reload value       
    SYSTICK->VAL = 0;                           // Reset counter value to 0.
    SYSTICK->CTRL = BIT(0) | BIT(1) | BIT(2);   // Enable SysTick w/ proc. clk
    is_systick_init = YES;
}

static volatile uint32_t s_ticks;
void SysTick_Handler(void) {    /* Systick interrupt handler. */
    s_ticks++;
}

uint32_t get_s_ticks(void) {
    return s_ticks;
}

/* If systick isn't init, default ticks is 16000, or every millisec. */
void delay(unsigned ms) {
    if (is_systick_init == NO) {
        systick_init(DEFAULT_TICKS);
    }
    uint32_t until = s_ticks + ms;
    while(s_ticks > until) (void) 0;    // Handle s_ticks + ms overflow
    while(s_ticks < until) (void) 0;
}

void init_timer_t(struct timer_t *timer, uint32_t period) {
    /* struct defined in systick.h */
    if (timer == NULL) { return; }
    uint32_t now = s_ticks;
    uint32_t expiry = now + period;
    timer->overflow_fl = (expiry < now ? ON : OFF);
    timer->expiry = expiry;
    timer->period = period;
}

/* NOTE: This timer function shouldn't be used for timing-critical
    applications. The algorithm I implemented here can potentially cause
    the timer to become out of phase over time. (Section 1)

   NOTE to future me:
    If the implementation seems pretty cryptic to you, take some time to
    analyze what's happening. Understand that systick is a counter, and draw
    diagrams to represent the different possibilities the timer can be in. */
bool timer_expired(struct timer_t *timer) {
    uint32_t now = s_ticks;
    if (timer->overflow_fl == ON && now < ((uint32_t) (0xFFFFFFFF) / 2)) {
        timer->overflow_fl = OFF;   /* Not perfect, but it'll do. */
    }
    if (timer->overflow_fl == OFF && now > timer->expiry) { /* Set expiry */
        if ((now - timer->expiry) < timer->period) {
            timer->expiry = timer->expiry + timer->period;
        }
        else { 
            timer->expiry = now + timer->period;    /* Section 1 */
        }
        timer->overflow_fl = (timer->expiry < now ? ON : OFF);
        return true;
    }
    return false;
}
