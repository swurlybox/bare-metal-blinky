/*
    API for accessing GPIO and setting bits in GPIO related configuration
    registers.
*/
#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>
#include <stdbool.h>

#define PIN(bank, num) ((((bank) - 'A') << 8) | (num))
#define PINNO(pin) (pin & 255)
#define PINBANK(pin) (pin >> 8)

struct gpio {
    volatile uint32_t 
    MODER, OTYPER, OSPEEDR, PUPDR,
    IDR, ODR, BSRR, LCKR, AFR[2];
};

/* STM32 has GPIO banks A-H: 0-7, with pin no.s 0-15. Banks are 0x400 apart */
#define GPIO(bank) ((struct gpio *) (0x40020000 + (0x400 * (bank))))

enum {GPIO_MODE_INPUT, GPIO_MODE_OUTPUT, GPIO_MODE_AF, GPIO_MODE_ANALOG};

void gpio_set_mode(uint16_t pin, uint8_t mode);
void gpio_bank_enable(uint8_t bank);
void gpio_bank_disable(uint8_t bank);
void gpio_write(uint16_t pin, bool val);

#endif
