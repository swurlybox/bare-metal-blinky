#include "usd_card.h"
#include "peripherals/spi.h"
#include "peripherals/systick.h"

/* NOTE: Debugging */
#include <stdio.h>

#define BIT(x)  (1U << (x))
#define NOARGS  (0)
#define NOCRC   (0)
#define CHECK_PATTERN   (0xAA)
#define SECTOR_SIZE (512)
#define DATA_START_TOKEN    ((uint8_t) 0xFC)
#define STOP_TRANS_TOKEN    ((uint8_t) 0xFD)
#define DAT_RES_SUCCESS     ((uint8_t) 0x05)

/* NOTE: See SD Card Physical Layer Simplified Specification */

/* SPI mode Command, Response, and Data Token formats */
typedef struct command {
    uint8_t arr[6];
} cmd_t;

typedef struct response1 {
    uint8_t arr[1];
} r1_t;

typedef struct response2 {
    uint8_t arr[2];
} r2_t;

typedef struct response3 {
    uint8_t arr[5];
} r3_t;

typedef struct response7 {
    uint8_t arr[5];
} r7_t;

/*  Helper functions.
    NOTE: the send/receive bytes don't stall until a non-0xFF byte
        is encountered. Meaning it will accept any byte including 0xFF.

        A timeout mechanism for these SPI-facing functions would be nice,
        so that we don't block our CPU in the event something stalls.
 */
static void set_cmd(cmd_t *cmd, uint8_t cmdno, uint32_t args, uint8_t crc7);
static void send_cmd(cmd_t *cmd);
static void send_byte(uint8_t byte);
static void receive_r1(r1_t *response);
static void receive_r2(r2_t *response);
static void receive_r7(r7_t *response);
static void receive_byte(r1_t *response);

static void     sd_card_power_on(void);
static uint8_t  sd_card_enter_spi(void);
static uint8_t  sd_card_spi_initialize(void);

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

static void send_byte(uint8_t byte) {
    spi_transfer(byte);
    #ifdef DEV_ENV
    printf("Byte Sent: %x\r\n", byte);
    #endif    
}

static void receive_r1(r1_t *response) {
    while ((response->arr[0] = spi_transfer((uint8_t) 0xFF)) == 0xFF) {
        (void) 0;
    }
    #ifdef DEV_ENV
    printf("R1 Byte 0: %x\r\n", response->arr[0]);
    #endif
}

static void receive_r2(r2_t *response) {
    while ((response->arr[0] = spi_transfer((uint8_t) 0xFF)) == 0xFF) {
        (void) 0;
    }
    response->arr[1] = spi_transfer((uint8_t) 0xFF);
    #ifdef DEV_ENV
    for (int i = 0; i < 2; i++) {
        printf("R2 Byte %d: %x\r\n", i, response->arr[i]);
    }
    #endif
}

static void receive_r7(r7_t *response) {
    while ((response->arr[0] = spi_transfer((uint8_t) 0xFF)) == 0xFF) {
        (void) 0;
    }
    for (int i = 1; i < 5; i++) {
        response->arr[i] = spi_transfer((uint8_t) 0xFF);
    }
    #ifdef DEV_ENV
    for (int i = 0; i < 5; i++) {
        printf("R7 Byte %d: %x\r\n", i, response->arr[i]);
    }
    #endif
}

static void receive_byte(r1_t *response) {
    response->arr[0] = spi_transfer((uint8_t) 0xFF);
    #ifdef DEV_ENV
    printf("Byte Received: %x\r\n", response->arr[0]);
    #endif
}

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
    printf("SD card has finished powering on!\r\n");
}

/* Issues CMD0 and CS low, which resets the SD card into SPI mode. */
static uint8_t sd_card_enter_spi() {
    cmd_t CMD = {0};
    r1_t RES1 = {0};
    uint8_t attempt = 0;

    /* CMD0: RESET */
    set_cmd(&CMD, 0, NOARGS, 0b1001010);
    
    resend:
    send_cmd(&CMD);
    receive_r1(&RES1);

    /* Expecting 0x01 (idle bit set) as as response. */
    if (RES1.arr[0] != 0x01) {  
        if (attempt++ < 3) {
            goto resend;
        }
        else {
            printf("SD card failed to enter SPI. Aborting...\r\n");
            return FAIL;
        }
    }

    printf("SD card entered SPI mode!\r\n");
    return SUCCESS;
}

/* Transition from SPI idle mode to SPI data transfer mode. */
static uint8_t sd_card_spi_initialize(void) {
    cmd_t CMD = {0};
    r1_t RES1 = {0};
    r7_t RES7 = {0};
    uint8_t attempt = 0;
 
    /*  Send CMD8 w/ VHS set to 3.3V range and 0xAA check pattern */
    set_cmd(&CMD, 8, BIT(8) | CHECK_PATTERN, NOCRC);

    resend_cmd8:
    send_cmd(&CMD);
    receive_r7(&RES7);
    /*  Expected Response:
        1st byte 0x1 to indicate idle state.
        4th byte last 4 bits 0b0001 to indicate 3.3V support.
        5th byte echo of check pattern. */
    if ((RES7.arr[0] != 0x1) ||
        (RES7.arr[3] & 15U) != 0x1 ||
        (RES7.arr[4] != CHECK_PATTERN)) {
        if (attempt < 3) {
            goto resend_cmd8;
        }
        else {
            printf("SD card failed CMD8 response check. Aborting...\r\n");
            return FAIL;
        }
    }

    /*  Send CMD55 to indicate next command is an APPCMD.
        Send ACMD41 with HCS set to 1 (our SD card is 4GB SDHC)
        Receive R1 response, and wait for idle bit to clear.
        Card identification mode is optional */
    attempt = 0;
    resend_acmd41:
    set_cmd(&CMD, 55, NOARGS, NOCRC);
    send_cmd(&CMD);
    receive_r1(&RES1); 
    
    set_cmd(&CMD, 41, (1U << 30U), NOCRC);
    send_cmd(&CMD);
    receive_r1(&RES1);

    if (RES1.arr[0] != 0x0) {
        if (attempt++ < 3) {
            goto resend_acmd41;
        } 
        else {
            printf("ACMD41 failed to initialize SD card\r\n");
            return FAIL;
        }
    }
    printf("SD card entered SPI data transfer mode!\r\n");
    return SUCCESS;
}

/* Public-facing microSD card API: usd_card.h */
uint8_t sd_card_init() {
    sd_card_power_on();
    if (sd_card_enter_spi() == FAIL) { return FAIL; } 
    if (sd_card_spi_initialize() == FAIL) { return FAIL; }
    /* crank up spi frequency: 8MHz on clear */
    SPI2->CR1 &= ~(0b111U << 3U);
    return SUCCESS;
};

uint8_t sd_card_get_status(void) {
    cmd_t CMD = {0};
    r2_t RES2 = {0};

    /* CMD13: SEND_STATUS */
    set_cmd(&CMD, 13, NOARGS, NOCRC);
    send_cmd(&CMD);
    receive_r2(&RES2);
    if (RES2.arr[0] != 0 || RES2.arr[1] != 0) {
        printf("SD card status has an error bit set\r\n");
        return FAIL;
    }
    #ifdef DEV_ENV
    printf("SD card status good\r\n");
    #endif
    return SUCCESS;
}

/* NOTE: To myself, I did consider trying to use DMA somewhere in here,
    but due to how tightly-coupled our tasks are (display and audio playback
    depend on the sd card reads) and the importance of the sequential order
    of our tasks, there doesn't seem to be much benefit to using DMA here. 

    Like, what else could the CPU be doing while waiting for MP3 chunks
    from storage? Not much else to be honest, one could argue the CPU
    could be handling user inputs in the meanwhile, but that rarely happens
    so its wasted CPU cycles, and the user-input handling probably won't
    slow down audio playback too much. In the media-player state, button
    inputs would either be intended to 1) pause/play, 2) volume control, 3)
    transition back into filesystem navigation, 4) seeking. 3 of these options
    involve interrupting audio playback, which is okay. For volume control, 
    audio playback shouldn't be noticably interrupted, and it involves
    setting a volume control register via i2c with the DAC. */
uint8_t sd_card_multi_read(uint32_t LBA, uint8_t *srcbuf, uint32_t sec_cnt) {
    cmd_t CMD = {0};    // Commands
    r1_t RES1 = {0};    // R1 responses, data_start and data_error tokens
    r2_t RES2 = {0};    // For 16-bit CRC

    /* SEND CMD18: MULTI_BLOCK_READ */
    set_cmd(&CMD, 18, LBA, NOCRC);
    send_cmd(&CMD);

    receive_r1(&RES1);
    if (RES1.arr[0] != 0x00) {
        printf("R1 response of CMD18 not good: %x\r\n", RES1.arr[0]);
        return FAIL;
    }
    
    /* SET CMD12: STOP_TRANSMISSION */
    set_cmd(&CMD, 12, NOARGS, NOCRC);

    printf("begin receiving blocks\r\n");

    /* RECEIVE SEC_CNT DATA BLOCKS */
    int i = 0;
    while (sec_cnt-- > 0) {
        /* Expect data start token */
        receive_r1(&RES1);
        if (RES1.arr[0] != 0xFE) {
            printf("Didn't receive data start token. Aborting. \r\n");
            send_cmd(&CMD);
            receive_r1(&RES1);
            return FAIL;
        }

        /* Start copying data blocks over to srcbuf. */
        while (i++ < SECTOR_SIZE) {    
            *srcbuf++ = spi_transfer((uint8_t) 0xFF);
        }
        /* Receive 16-bit CRC (We won't do anything w/ it though) */
        receive_r2(&RES2);
        i = 0;
    }
    printf("done reading blocks\r\n");

    /* CMD12: STOP_TRANSMISSION */
    send_cmd(&CMD);
    receive_byte(&RES1);    /* Discard garbage byte */
    do {
        receive_byte(&RES1);
    } while(RES1.arr[0] == 0x00);   // Till res and busy signal clear.

    return SUCCESS;
}

uint8_t sd_card_multi_write(uint32_t LBA, const uint8_t *databuf,
                             uint32_t sec_cnt) {
    cmd_t CMD = {0};
    r1_t RES1 = {0};

    /* CMD25: MULTI_BLOCK_WRITE */
    set_cmd(&CMD, 25, LBA, NOCRC);
    send_cmd(&CMD);

    receive_r1(&RES1);
    if(RES1.arr[0] != 0x00) {
        printf("R1 response of CMD25 was bad: %x\r\n", RES1.arr[0]);
        return FAIL;
    }

    /* SEND SEC_CNT DATA BLOCKS */
    int i = 0;
    while (sec_cnt-- > 0) {
        /* DATA_START_TOKEN */
        send_byte(DATA_START_TOKEN);                
        
        /* SEND 1 DATA BLOCK */
        while (i++ < SECTOR_SIZE) {
            send_byte(*databuf++);
        }

        /* DATA RESPONSE TOKEN */
        receive_r1(&RES1);
        if ((RES1.arr[0] & 31U) != DAT_RES_SUCCESS) {
            printf("Data Reponse token indicated error: %x\r\n", RES1.arr[0]);
            return FAIL;
        }
        /* WAIT FOR BUSY SIGNAL CLEAR */
        do {
            receive_byte(&RES1);
        } while (RES1.arr[0] == 0x00);
        i = 0;
    }
    
    /* STOP_TRANS_TOKEN */
    send_byte(STOP_TRANS_TOKEN);
    receive_byte(&RES1);    // Discard garbage byte
    do {
        receive_byte(&RES1);
    } while (RES1.arr[0] == 0x00);  // Wait till busy signal is cleared

    return SUCCESS;
}
