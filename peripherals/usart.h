#ifndef USART_H
#define USART_H

#include <stdint.h>
#include <stddef.h>

/* Status, Data, Baud Rate, Control 1, Control 2, Control 3, Guard time and
    prescaler */
struct uart {
    volatile uint32_t SR, DR, BRR, CR1, CR2, CR3, GTPR;
};

/* APB2 */
//#define USART6 ((struct uart *) (0x40011400))
//#define USART1 ((struct uart *) (0x40011000))

/* APB1 */
//#define UART5 ((struct uart *) (0x40005000))
//#define UART4 ((struct uart *) (0x40004C00))
//#define USART3 ((struct uart *) (0x40004800))
#define USART2 ((struct uart *) (0x40004400))

/* 
    To configure UART we need:

        - enable UART clock in RCC
        - set alternate function pin mode for RX and TX pins
            PA2 and PA3 (USART2_TX and USART2_RX) respectively
        - set UART configs (baud rate, parity bits, frame format, etc)
        - enable the peripheral, receive and transmit via the cr1 register. 

        GPIOA2 and GPIOA3

        - Set alternate functions AFRL2 and AFRL3
            to AF7 (0111)

        certain USARTs require us to enable certain pins. 
*/

void uart_init(struct uart *uart, unsigned long baud);
uint8_t uart_read_ready(struct uart *uart);
uint8_t uart_read_byte(struct uart *uart);
void uart_write_byte(struct uart *uart, uint8_t byte);
void uart_write_buf(struct uart *uart, char *buf, size_t len);

#endif
