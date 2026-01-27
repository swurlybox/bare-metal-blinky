/*
    API for enabling the RCC (Reset and Clock Control) unit for specific
    peripherals.

    By default, the STM32 has all peripherals disabled on power up in order to
    save power. The peripherals we wish to use must have their clock enabled 
    during runtime, and we do this by setting certain registers in the RCC 
    unit.

    AHB1 is where the GPIO's live. Thus we need to interact with:
    
    RCC_AHB1ENR, bits 0-7 enable the gpio banks A-H peripherals.
*/
#ifndef RCC_H
#define RCC_H

struct rcc {
    volatile uint32_t CR, PLLCFGR, CFGR, CIR, AHB1RSTR, AHB2RSTR, AHB3RSTR,
    RESERVED0, APB1RSTR, APB2RSTR, RESERVED1[2], AHB1ENR, AHB2ENR, AHB3ENR,
    RESERVED2, APB1ENR, APB2ENR, RESERVED3[2], AHB1LPENR, AHB2LPENR, AHB3LPENR,
    RESERVED4, APB1LPENR, APB2LPENR, RESERVED5[2], BDCR, CSR, RESERVED6[2],
    SSCGR, PLLI2SCFGR, PLLSAICFGR, DCKCFGR, CKGATENR, DCK_CFGR2; 
};

#define RCC ((struct rcc *) 0x40023800)

#endif
