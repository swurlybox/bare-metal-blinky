#include "usart.h"
#include "gpio.h"
#include "rcc.h"
#include <math.h>

#define FREQ (16000000U)
#define BIT(x) (1U << (x))

/* 8 bit, 1 parity, oversampling 16. User specifies UART interface and baud */
void uart_init(struct uart *uart, unsigned long baud) {
    /* tx and rx pins correspond to certain GPIO pins, af can differ depending
        on which usart interface we're using. 
        TODO: (optional) Handle other USART interfaces. This shit is better
        done via auto code generation from pin configurations in the vendor
        IDE, since we ultimately decide (hardcode) which pins we want to use
        for each USART interface.
    */
    uint8_t af;
    uint16_t tx, rx;

    if (uart == USART2) {
        RCC->APB1ENR |= BIT(17);
        tx = PIN('A', 2);
        rx = PIN('A', 3);
        af = 7;
    } 

    gpio_set_mode(tx, GPIO_MODE_AF);
    gpio_set_af(tx, af);
    gpio_set_mode(rx, GPIO_MODE_AF);
    gpio_set_af(rx, af);
    
    uart->CR1 = 0; 

    /* BRR calculation: DONE: Seems to work now after enabling FPU */ 
    uint8_t OVER8 = 0;
    uint32_t mantissa, fractional;
    float usartdiv = ((float) (FREQ)) / ((float) (8U * (2U - OVER8) * baud));
    mantissa = (uint32_t) floorf(usartdiv);
    fractional = (uint32_t) roundf((usartdiv - (float) mantissa) * 16.0F);
    uart->BRR = (mantissa << 4) | (fractional << 0);
    
    uart->CR1 |= BIT(13) | BIT(2) | BIT(3);
    uart_write_buf(uart, "USART initialized\r\n", 19);
}

uint8_t uart_read_ready(struct uart *uart) {
    /* Check the status register at a specific bit */ 
    return (uint8_t) (uart->SR * BIT(5));
}

uint8_t uart_read_byte(struct uart *uart) {
    /* Grab the data from the data register */
    return (uint8_t) (uart->DR & 255);
}

void uart_write_byte(struct uart *uart, uint8_t byte) {
    uart->DR = byte;
    /* We want to wait until the data has finished transmitting */
    while ((uart->SR & BIT(7)) == 0) spin(1);
}

void uart_write_buf(struct uart *uart, char *buf, size_t len) {
    while (len-- > 0) uart_write_byte(uart, *(uint8_t *) buf++);
}
