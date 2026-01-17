/*
    API for accessing GPIO and setting bits in GPIO related configuration
    registers.
*/
#ifndef GPIO_H
#define GPIO_H

#include <inttypes.h>
#include <stdbool.h>

#include "rcc.h"

/* These function macros are okay, because they are just substitutes for 
   common expressions. For functions that are a bit more complex and require
   more computations to be done, keep it as a regular inline function. */
#define BIT(x) (1UL << (x))
#define PIN(bank, num) ((((bank) - 'A') << 8) | (num))
#define PINNO(pin) (pin & 255)
#define PINBANK(pin) (pin >> 8)

/* volatile tells the compiler to not cache the variable values in memory. 
    This is important if you want to always read register values from the
    actual mcu, instead of a cached copy that might be outdated.
*/
struct gpio {
    volatile uint32_t 
    MODER, OTYPER, OSPEEDR, PUPDR,
    IDR, ODR, BSRR, LCKR, AFR[2];
};

/* STM32 has GPIO banks A-H: 0-7, with pin no.s 0-15. Banks are 0x400 apart */
#define GPIO(bank) ((struct gpio *) (0x40020000 + (0x400 * (bank))))

/* Output mode enums */
enum {GPIO_MODE_INPUT, GPIO_MODE_OUTPUT, GPIO_MODE_AF, GPIO_MODE_ANALOG};

/* Pass in a 2-byte bank-pin value, where upper-byte is the bank value
    and lower-byte is the pin no. value.

    e.g. for GPIOA3, use the PIN('A', 3) function macro.
         mode is one of the four values specified in the mode enum.
*/
static inline void gpio_set_mode(uint16_t pin, uint8_t mode)
{
    struct gpio *gpio = GPIO(PINBANK(pin));
    uint8_t n = (uint8_t) PINNO(pin);
    gpio->MODER &= ~(3U << (n * 2));
    gpio->MODER |= (mode & 3) << (n * 2);
}

/* dependency on RCC, ideally you want to pass in PINBANK(led), which maps
   'A'-'H' to 0-7, but we'll allow passing 'A'-'H' directly too. */
static inline void gpio_bank_enable(uint8_t bank) {
    if (bank >= 'A' && bank <= 'H') { 
        bank = bank - 'A';
    }
    (bank <= 7) ? RCC->AHB1ENR |= BIT(bank) : (void) 0;
}
/* clear bit, (set to 0) */
static inline void gpio_bank_disable(uint8_t bank) {
    if (bank >= 'A' && bank <= 'H') { 
        bank = bank - 'A';
    }
    (bank <= 7) ? RCC->AHB1ENR &= ~(BIT(bank)) : (void) 0;
}

/* and maybe an gpio_init_all and gpio_terminate_all */

/* TODO: function to write to a pin, basically turning it on and off.
    We can use the GPIO_BSRR or the GPIO_ODR to turn it on or off.
*/
static inline void gpio_write(uint16_t pin, bool val) {
    struct gpio *gpio = GPIO(PINBANK(pin));
    gpio->BSRR = (1U << PINNO(pin)) << (val ? 0 : 16);
}

static inline void spin(volatile uint32_t count) {
    while (count--) (void) 0;
}


#endif
