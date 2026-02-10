#include "usart.h"
#include "gpio.h"
#include "rcc.h"
#include <math.h>

#define FREQ (16000000U)
#define BIT(x) (1U << (x))

/* Initialize USART2 for serial debug output, sent to the host PC
    USART Configurations:
        Data Frame:     8 bits
        Parity:         1-bit
        Oversampling:   16 bits
        Baud-Rate:      (decided by user; 115200 is what I would use)

    NOTE: Could generalize this to initialize other USART interfaces, but
        there's no good reason for me to do so at the moment.
*/
void uart_init(struct uart *uart, unsigned long baud) {
    uint8_t af;
    uint16_t tx, rx;

    /* Choice of A2 and A3 because that has been hardwired/soldered on
        the board to direct USART2 communications to our host PC */
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
