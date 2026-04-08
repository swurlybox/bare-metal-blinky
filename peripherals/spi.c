#include "spi.h"
#include "rcc.h"
#include "gpio.h"
#include "dma.h"
#include "setup/nvic.h"
#include "states/audio_playback.h"


/* NOTE: Debugging */
#include <stdio.h>

#define BIT(x) (1U << (x))
#define DMA1_STREAM7_INTERRUPT_NO   (47)

void i2s3_init(void) {
    /*
        Set up PLLI2S dividers
        PLLI2S ON
        Set up RCC clock to feed 48MHz to I2S_CLK.
    */
    RCC->PLLI2SCFGR &= ~(0x70007FFFU);  
    RCC->PLLI2SCFGR |= (0x30001208U);   /* R=3, N=72, M=8 */
    RCC->CR |= BIT(26);  /* PLLI2S ON */

    /* DMA specific configurations: memory / peripheral addresses etc. */
    RCC->AHB1ENR |= BIT(21); /* DMA1 clock enable */
    enable_irq(DMA1_STREAM7_INTERRUPT_NO);
    /* DMA1 Channel 0, Stream 7 */
    /* DMA1->HISR & BIT(27) Stream 7 transfer complete interrupt */
    /* DMA1->HIFCR & BIT(27) Stream 7 clear transfer complete interrupt */
    DMA1->S7CR |= BIT(18);  /* double buffer mode. */
    DMA1->S7CR |= BIT(11);  /* half-word 16bit data size */
    DMA1->S7CR |= BIT(10);  /* memory increment mode */
    DMA1->S7CR |= BIT(6);   /* Memory-to-peripheral */
    /* TODO: hook up to actual interrupt service routine. */
    DMA1->S7CR |= BIT(4);   /* transfer complete interrupt enable */
    DMA1->S7NDTR = MINIMP3_MAX_SAMPLES_PER_FRAME;
    DMA1->S7PAR = (uint32_t) &I2S3->DR;
    DMA1->S7M0AR = (uint32_t) &audio_player.pcm0;
    DMA1->S7M1AR = (uint32_t) &audio_player.pcm1;
    /* NOTE: configure stream priority */

    DMA1->S7CR |= BIT(0);   /* DMA enable */

    /* I2S specific configurations; */
    RCC->APB1ENR |= BIT(15);    /* SPI3 Clock EN */
    I2S3->I2SCFGR |= BIT(11);   /* I2S mode selected */
    I2S3->I2SCFGR |= BIT(9);    /* Master transmit */
    I2S3->I2SPR |= BIT(8);      /* I2SODD = 1 */
    I2S3->I2SPR &= ~(0xFFU);
    I2S3->I2SPR |= (15U);       /* I2SDIV = 15 */
    I2S3->CR2 |= BIT(1);        /* DMATXIE enable*/
    
    /* check status of dma and i2s */
    printf("DMA->HISR: %lx\r\n", DMA1->HISR);
    printf("DMA->S7CR: %lx\r\n", DMA1->S7CR); 
    printf("DMA->NDTR: %lx\r\n", DMA1->S7NDTR);
    printf("DMA->S7PAR: %lx\r\n", DMA1->S7PAR);
    printf("DMA->S7M0AR: %lx\r\n", DMA1->S7M0AR);
    printf("DMA->S7M1AR: %lx\r\n", DMA1->S7M1AR);

    printf("I2S3->I2SCFGR: %lx\r\n", I2S3->I2SCFGR); 
    printf("I2S3->I2SPR: %lx\r\n", I2S3->I2SPR);
    printf("I2S3->CR2: %lx\r\n", I2S3->CR2);

    /* hook up GPIOs for I2S */
    uint16_t SD, WS, CK;
    SD = PIN('C', 12);
    WS = PIN('A', 15);
    CK = PIN('C', 10);

    gpio_set_mode(SD, GPIO_MODE_AF);    
    gpio_set_mode(WS, GPIO_MODE_AF);    
    gpio_set_mode(CK, GPIO_MODE_AF);    
    gpio_set_af(SD, 6);
    gpio_set_af(WS, 6);
    gpio_set_af(CK, 6);
}

/* NOTE: this probably doesn't belong here? */
void DMA1_Stream7_Handler(void) {
    /* to my knowledge: the DMA seems really fast, I don't know if our
    audio processing pipeline can keep up? TEST: */
    DMA1->HIFCR |= BIT(27); /* clear interrupt flag */
    /* triggered when tx transaction has completed */ 
    /* set dmaptr->status to FREE, then switch dmaptr to other pcm */
    audio_player.dma_ptr->status = FREE;
    if (audio_player.dma_ptr == &audio_player.pcm0) {
        audio_player.dma_ptr = &audio_player.pcm1;
    } else {
        audio_player.dma_ptr = &audio_player.pcm0;
    }
    if(audio_player.dma_ptr->status == FREE) {
        /* dma consumption faster than producer. */
        printf("faster than producer\r\n");
    }
}


void i2s3_start(void) {
    I2S3->I2SCFGR |= BIT(10);
};

void i2s3_stop(void) {
    I2S3->I2SCFGR &= ~(BIT(10));
};

/* NOTE: Initializes SPI2 interface with very specific configurations,
    according to my application needs.

    SPI2 Pin Configurations:
        NSS:    PB12
        SCK:    PB13
        MISO:   PB14
        MOSI:   PB15

    Baud Rate:                  250KHz initially, ramped up to 8MHz in SD 
                                initialization.
    Data Frame Format:          8 bits
    MSBFIRST:                   Yes
    Master Mode:                Yes
    Software Slave Management:  Enabled initially to drive CS pin manually,
                                then disabled to let hardware manage CS pin 
                                automatically on data transfers.
*/
void spi_init(void) {
    RCC->APB1ENR |= BIT(14);    /* Enable SPI peripheral clock */

    /* Configure corresponding GPIO pins */
    uint16_t nss, sck, miso, mosi;
    nss = PIN('B', 12);
    sck = PIN('B', 13);
    miso = PIN('B', 14);
    mosi = PIN('B', 15);

    gpio_set_mode(nss, GPIO_MODE_AF);
    gpio_set_mode(sck, GPIO_MODE_AF);
    gpio_set_mode(miso, GPIO_MODE_AF);
    gpio_set_mode(mosi, GPIO_MODE_AF);

    /* 5 is the AF_NUM for SPI */
    gpio_set_af(nss, 5);
    gpio_set_af(sck, 5);
    gpio_set_af(miso, 5);
    gpio_set_af(mosi, 5);

    /* why 250KHz? */
    /* Set SPI-related configurations. NOTE: May want to enable DMA bits l8r */ 
    SPI2->CR1 |= 0b101 << 3U;   /* fpclk/64 ~ 250KHz, which is safe for SD */
    SPI2->CR1 |= BIT(2);        /* Master mode */
    SPI2->CR2 |= BIT(2);        /* SSOE */

    /* change the frequency back to fastest speed possible. */

    #ifdef DEV_ENV
        /* NOTE: Diagnostics, verify SPI configurations. */
        printf("------ Checking Initial SPI configurations ------\r\n");
        printf("Baud Rate Field Value: %ld\r\n", 
            (SPI2->CR1 & (BIT(3) | BIT(4) | BIT(5))) >> 3U);
        printf("Master mode: %ld\r\n", (SPI2->CR1 & BIT(2)) >> 2U);
        printf("SSOE enable (CS controlled by HW): %ld\r\n", 
            (SPI2->CR2 & BIT(2)) >> 2U);
    #endif 
}

/* pointer to the data we're trying to send. */
void spi_block_transfer_read(uint8_t *srcbuf) {
    //SPI2->CR1 |= BIT(6);
    int i = 0;

    while (i++ < 512) {
        SPI2->DR = (uint32_t) 0xFF;
        while ((SPI2->SR & BIT(7)) || !(SPI2->SR & BIT(0))) {
            (void) 0;   /* Wait till RX_buffer non-empty and SPI isn't busy. */
        }
        *srcbuf++ = (uint8_t) SPI2->DR;
    }
    //SPI2->CR &= ~BIT(6);
    return;
}

/* Single byte transfer. Nice to have for its granularity. */
uint8_t spi_transfer(uint8_t tx_data) {
    uint8_t rx_data = 0;
    //SPI2->CR1 |= BIT(6);                /* Enable SPI communication */
    SPI2->DR = (uint32_t) (tx_data);    /* TX_buffer transmit, toggles SCLK */
    while ((SPI2->SR & BIT(7)) || !(SPI2->SR & BIT(0))) {
        (void) 0;   /* Wait till RX_buffer non-empty and SPI isn't busy. */
    } 
    rx_data = (uint8_t) SPI2->DR;       /* Read RX_buffer, clears RXNE bit */
    //SPI2->CR1 &= ~BIT(6);               /* Disable SPI communication */
    return rx_data;
}
