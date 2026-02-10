#include "usd_card.h"
#include "peripherals/spi.h"
#include "peripherals/systick.h"

/* NOTE: Debugging */
#include <stdio.h>

#define BIT(x)  (1U << (x))
#define NOARGS  (0)
#define NOCRC   (0)
#define CHECK_PATTERN   (0xAA)
#define SUCCESS (0U)
#define FAIL    (1U)

/* NOTE: See SD Card Physical Layer Simplified Specification */

/* SPI mode Command, Response, and Data Token formats */
typedef struct command {
    uint8_t arr[6];
} cmd_t;

typedef struct response1 {
    uint8_t arr[1];
} r1_t;

typedef struct response3 {
    uint8_t arr[5];
} r3_t;

typedef struct response7 {
    uint8_t arr[5];
} r7_t;

/* An API that interacts with these command, response, and data tokens */
static void set_cmd(cmd_t *cmd, uint8_t cmdno, uint32_t args, uint8_t crc7);
static void send_cmd(cmd_t *cmd);
static void receive_r1(r1_t *response);
static void receive_r7(r7_t *response);

/* TODO: Implement timeout 
    Not entirely sure if response is aligned on 8-bit boundary, couldn't
    find info on this.
*/
static void receive_r1(r1_t *response) {
    while ((response->arr[0] = spi_transfer((uint8_t) 0xFF)) == 0xFF) {
        (void) 0;
    }
    #ifdef DEV_ENV
        printf("R1 Byte 0: %x\r\n", response->arr[0]);
    #endif
}

/* TODO: Implement timeout */
static void receive_r7(r7_t *response) {
    while ((response->arr[0] = spi_transfer((uint8_t) 0xFF)) == 0xFF) {
        (void) 0;
    }
    #ifdef DEV_ENV
        printf("R7 Byte 0: %x\r\n", response->arr[0]);
        for(int i = 1; i < 5; i++) {
            response->arr[i] = spi_transfer((uint8_t) 0xFF);
            printf("R7 Byte %d: %x\r\n", i, response->arr[i]);
        }
    #endif
}

static void set_cmd(cmd_t *cmd, uint8_t cmdno, uint32_t args, uint8_t crc7) {
    uint8_t *ptr = cmd->arr;
    cmdno &= 0b111111;  /* Mask excess bits */
    crc7 &= 0b1111111;
    *ptr++ = (uint8_t) ((0b01 << 6U) | cmdno);  /* Set command index */
    for (int i = 3; i >= 0; i--) {              /* Set arguments */
        *ptr++ = (uint8_t) (args >> (i * 8));
    }
    *ptr = (uint8_t) ((crc7 << 1U) | 1U);       /* Set CRC */
}

static void send_cmd(cmd_t *cmd) {
    uint8_t *ptr = cmd->arr;
    for(int i = 0; i < 6; i++, ptr++) {
        spi_transfer(*ptr);
        #ifdef DEV_ENV
            printf("CMD Byte %d: %x\r\n", i, *ptr);
        #endif
    }
}

static void     sd_card_power_on(void);
static uint8_t  sd_card_enter_spi(void);
static uint8_t  sd_card_spi_initialize(void);

/*  Delay for atleast 1ms for voltage in SPI module to ramp up to 3.3V.
    Set MOSI and CS high for atleast 74 cycles at 100-400KHz. */
static void sd_card_power_on() { 
    delay(3);
    SPI2->CR1 &= ~(0b111U << 3U);
    SPI2->CR1 |= 0b101U << 3U;          // Baud rate: 250KHz
    SPI2->CR1 |= BIT(8);    
    SPI2->CR1 |= BIT(9);                // CS High
    for (int i = 0; i < 10; i++) {
        spi_transfer((uint8_t) 0xFF);   // MOSI High
    }
    SPI2->CR1 &= ~(BIT(9)); // Reset SSM to allow HW control of CS pin.
}

/* Issues CMD0 and CS low, which resets the SD card into SPI mode. */
static uint8_t sd_card_enter_spi() {
    cmd_t CMD = {0};
    r1_t RES1 = {0};
    uint8_t attempt = 0;

    printf("----- Sending CMD0 -----\r\n");
    set_cmd(&CMD, 0, NOARGS, 0b1001010);

    resend:
    send_cmd(&CMD);
    receive_r1(&RES1);

    if (RES1.arr[0] != 0x01) {
        printf("Error in expected r1 response of CMD0: %u\r\n", RES1.arr[0]);
        if (attempt++ < 3) {
            printf("Resending CMD0...\r\n");
            goto resend;
        }
        else {
            printf("SD card failed to enter SPI mode\r\n");
            return FAIL;
        }
    }

    printf("SD card has entered SPI operation mode: %u\r\n", RES1.arr[0]);
    return SUCCESS;
}

static uint8_t sd_card_spi_initialize(void) {
    cmd_t CMD = {0};
    r1_t RES1 = {0};
    r7_t RES7 = {0};
    uint8_t attempt = 0;

    printf("----- Sending CMD8 -----\r\n");
    
    /* Send CMD8 w/ VHS set to 3.3V range and 0xAA check pattern */
    set_cmd(&CMD, 8, BIT(8) | CHECK_PATTERN, NOCRC);

    resend_cmd8:
    send_cmd(&CMD);
    receive_r7(&RES7);
    /* Expected Response:
        1st byte 0x1 to indicate idle state.
        4th byte last 4 bits 0b0001 to indicate 3.3V support.
        5th byte echo of check pattern. */
    if ((RES7.arr[0] != 0x1) ||
        (RES7.arr[3] & 15U) != 0x1 ||
        (RES7.arr[4] != CHECK_PATTERN)) {
        if (attempt < 3) {
            printf("Resending CMD8...\r\n");   
            goto resend_cmd8;
        }
        else {
            printf("SD card failed CMD8 response check\r\n");
        }
    }

    /* Send CMD55 to indicate next command is application cmd. 
       Receive an R1 response.

       Send ACMD41 with HCS set to 1 (our SD card is 4GB SDHC)
        Receive R1 response, and wait until idle bit is cleared to indicate
        successful initialization.
    
        Card identification mode is optional
        But Send CMD58 and receive R3 response and check CCS field
    */
    attempt = 0;
    resend_acmd41:
    set_cmd(&CMD, 55, NOARGS, NOCRC);
    printf("----- Sending CMD55 -----\r\n");
    send_cmd(&CMD);
    receive_r1(&RES1); 
    
    printf("----- Sending ACMD41 -----\r\n");
    set_cmd(&CMD, 41, (1U << 30U), NOCRC);
    send_cmd(&CMD);
    receive_r1(&RES1);

    if (RES1.arr[0] != 0x0) {
        if (attempt++ < 3) {
            printf("ACMD41 response indicates SD card hasn't been init\r\n");    
            goto resend_acmd41;
        } 
        else {
            printf("ACMD41 failed to initialize SD card\r\n");
            return FAIL;
        }
    }
    return SUCCESS;
}

void sd_card_init() {
    sd_card_power_on();
    if (sd_card_enter_spi() == FAIL) { return; } 
    if (sd_card_spi_initialize() == FAIL) { return; }
    printf("SD card successfully initialized! Good for read/write ops...\r\n");
};
