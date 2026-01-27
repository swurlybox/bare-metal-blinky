/*
    Implementation of functions declared in gpio.h header file.
*/

#include "gpio.h"
#include "rcc.h"

#define BIT(x) (1U << (x))
#define GPIO(bank) ((struct gpio *) (0x40020000 + (0x400 * (bank))))

void gpio_set_mode(uint16_t pin, uint8_t mode) {
    struct gpio *gpio = GPIO(PINBANK(pin));
    uint8_t n = (uint8_t) PINNO(pin);
    RCC->AHB1ENR |= BIT(PINBANK(pin));
    gpio->MODER &= ~(3U << (n * 2));
    gpio->MODER |= (mode & 3) << (n * 2);
}

void gpio_bank_enable(uint8_t bank) {
    if (bank >= 'A' && bank <= 'H') {
        bank = bank - 'A';
    }
    (bank <= 7) ? RCC->AHB1ENR |= BIT(bank) : (void) 0;
}

void gpio_bank_disable(uint8_t bank) {
    if (bank >= 'A' && bank <= 'H') {
        bank = bank - 'A';
    }
    (bank <= 7) ? RCC->AHB1ENR &= ~(BIT(bank)) : (void) 0;
}

void gpio_write(uint16_t pin, bool val) {
    struct gpio *gpio = GPIO(PINBANK(pin));
    gpio->BSRR = (1U << PINNO(pin)) << (val ? 0 : 16);
}

void gpio_set_af(uint16_t pin, uint8_t af_num) {
    struct gpio *gpio = GPIO(PINBANK(pin));
    uint8_t n = (uint8_t) PINNO(pin); 
    gpio->AFR[n >> 3] &= ~(15U << ((n & 7) * 4));
    gpio->AFR[n >> 3] |= ((uint32_t) af_num & 15U) << ((n & 7) * 4);
}
