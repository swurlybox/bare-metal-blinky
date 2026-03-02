#ifndef SYSCFG_H
#define SYSCFG_H

#include <stdint.h>

/* NOTE: In order to interact with the SYSCFG controller, its peripheral
    clock must be enabled through RCC.
    
    RCC->APB2ENR |= BIT(14) 
*/
struct syscfg {
    volatile uint32_t MEMRMP, PMC, EXTICR1, EXTICR2, EXTICR3,
        EXTICR4, CMPCR, CFGR;
};

#define SYSCFG ((struct syscfg *) 0x40013800)

#endif
