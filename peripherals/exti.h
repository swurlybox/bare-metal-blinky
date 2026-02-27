#ifndef EXTI_H
#define EXTI_H

#include <stdint.h>

struct exti {
    volatile uint32_t IMR, EMR, RTSR, FTSR, SWIER, PR;
};

/* NVIC is the nested vector interrupt controller, which is handles
all interrupts, including the external interrupts. Its automatically enabled
on bootup. We just need to hookup external interrupts on certain pins. */
#define EXTI ((struct exti *) 0x40013C00)

/* NOTE: This is a declaration of an enum. So including this in multiple
    source files won't throw any errors. However if in a given source file
    there are conflicting names of the enum elements, an error will be thrown.
    */
enum {RISING_EDGE = 1, FALLING_EDGE = 2, BOTH = 3};

/*  Set up interrupt on certain pin in SYSCFG.
    Rising edge and/or falling edge trigger.
    Only applies to GPIO pins.

    Can bitwise OR FALLING_EDGE and RISING_EDGE to trigger interrupt on
    both edges.

*/
void exti_enable(uint16_t pin, uint8_t trigger);


#endif
