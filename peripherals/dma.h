#ifndef DMA_H
#define DMA_H

struct dma {
    volatile uint32_t LISR, HISR, LIFCR, HIFCR, S0CR, S0NDTR, S0PAR, S0M0AR,
        S0M1AR, S0FCR, S1CR, S1NDTR, S1PAR, S1M0AR, S1M1AR, S1FCR, S2CR,
        S2NDTR, S2PAR, S2M0AR, S2M1AR, S2FCR, S3CR, S3NDTR, S3PAR, S3M0AR,
        S3M1AR, S3FCR, S4CR, S4NDTR, S4PAR, S4M0AR, S4M1AR, S4FCR, S5CR,
        S5NDTR, S5PAR, S5M0AR, S5M1AR, S5FCR, S6CR, S6NDTR, S6PAR, S6M0AR,
        S6M1AR, S6FCR, S7CR, S7NDTR, S7PAR, S7M0AR, S7M1AR, S7FCR; 
};

#define DMA1 ((struct dma *) 0x40026000)
#define DMA2 ((struct dma *) 0x40026400)

/*
    NOTE: Each entry's index indicates its associated stream, so the first
        element of a channel is at stream 0, next stream 1, and so on.

        There are 8 streams per channel. Undedicated streams can be used
            for general-purpose DMA. Those with a '|' can handle multiple 
            peripherals.

        See Table 28 and 29 on STM32 Reference Manual.

    DMA1 Request Mapping:

    Channel 0: SPI3_RX, SPDIFRX_DT, SPI3_RC, SPI2_RX, SPI2_TX, SPI3_TX,
        SPDIFRX_CS, SPI3_TX;

    Channel 1: I2C1_RX, I2C3_RX, TIM7_UP, -, TIM7_UP, I2C1_RX, I2C1_TX,
        I2C1_TX;

    Channel 2: TIM4_CH1, -, FMPI2C1_RX, TIM4_CH2, -, FMPI2C1_TX, TIM4_UP,
        TIM4_CH3;

    Channel 3: -, TIM2_UP | TIM2_CH3, I2C3_RX, -, I2C3_TX, TIM2_CH1, TIM2_CH2 |
        TIM2_CH4, TIM2_UP | TIME2_CH4

    Channel 4: UART5_RX, USART3_RX, UART4_RX, USART3_TX, UART4_TX, USART2_RX,
        USART2_TX, UART5_TX;

    Channel 5: -, -, TIM3_CH4 | TIM3_UP, -, TIM3_CH1 | TIM3_TRIG, TIM3_CH2, -,
        TIM3_CH3;

    Channel 6: TIM5_CH3 | TIM5_UP, TIM5_CH4 | TIM5_TRIG, TIM5_CH1, TIM5_CH4 |
        TIM5_TRIG, TIM5_CH2, -, TIM5_UP, -;

    Channel 7: -, TIM6_UP, I2C2_RX, I2C2_RX, USART3_TX, DAC1, DAC2, I2C2_TX;


    DMA2 Request Mapping:

    Channel 0: ADC1, SAI1_A, TIM8_CH1 | TIM8_CH2 | TIM8_CH3, SAI1_A, ADC1,
        SAI1_B, TIM1_CH1 | TIM1_CH2 | TIM2_CH3, SAI2_B;

    Channel 1: -, DCMI, ADC2, ADC2, SAI1_B, -, -, DCMI;

    Channel 2: ADC3, ADC3, -, -, -, -, -, -;

    Channel 3: SPI1_RX, -, SPI1_RX, SPI1_TX, SAI2_A, SPI1_TX, SAI2_B, QUADSPI;

    Channel 4: SPI4_RX, SPI4_TX, USART1_RX, SDIO, -, USART1_RX, SDIO,
        USART1_TX;

    Channel 5: -, USART6_RX, USART6_RX, SPI4_RX, SPI4_TX, -, USART6_TX, 
        USART6_TX;

    Channel 6: TIM1_TRIG, TIM1_CH1, TIM1_CH2, TIM1_CH1, TIM1_CH4 | TIM1_TRIG |
        TIM1_COM, TIM1_UP, TIM1_CH3, -;

    Channel 7: -, TIM8_UP, TIM8_CH1, TIM8_CH2, TIM8_CH3, -, -, TIM8_CH4 |
        TIM8_TRIG | TIM8_COM;

*/

#endif
