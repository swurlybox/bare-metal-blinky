/* API for interacting with the USART peripheral */
#ifndef USART_H
#define USART_H

#include <stdint.h>
#include <stddef.h>

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

void uart_init(struct uart *uart, unsigned long baud);
uint8_t uart_read_ready(struct uart *uart);
uint8_t uart_read_byte(struct uart *uart);
void uart_write_byte(struct uart *uart, uint8_t byte);
void uart_write_buf(struct uart *uart, char *buf, size_t len);

#endif
