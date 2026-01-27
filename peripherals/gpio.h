/*
    API for accessing GPIO and setting bits in GPIO related configuration
    registers.
*/
#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>
#include <stdbool.h>

/* Upper byte is the bank value, lower byte is the pin no. value */
#define PIN(bank, num) ((((bank) - 'A') << 8) | (num))
#define PINNO(pin) (pin & 255)
#define PINBANK(pin) (pin >> 8)

struct gpio {
    volatile uint32_t 
    MODER, OTYPER, OSPEEDR, PUPDR,
    IDR, ODR, BSRR, LCKR, AFR[2];
};

enum {GPIO_MODE_INPUT, GPIO_MODE_OUTPUT, GPIO_MODE_AF, GPIO_MODE_ANALOG};

void gpio_set_mode(uint16_t pin, uint8_t mode);
void gpio_bank_enable(uint8_t bank);
void gpio_bank_disable(uint8_t bank);
void gpio_write(uint16_t pin, bool val);
void gpio_set_af(uint16_t pin, uint8_t af_num);

/* Helper spin function */
static inline void spin(volatile uint32_t count) {
    while (count--) (void) 0;
}

#endif
