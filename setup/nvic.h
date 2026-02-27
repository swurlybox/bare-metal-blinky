#ifndef NVIC_H
#define NVIC_H

#include <stdint.h>

/* TODO: ARM Cortex M4 Structure of NVIC */
struct nvic {
    volatile uint32_t ICTR, RESERVED[62], ISER[8], RESERVED0[24], ICER[8], 
        RESERVED1[24], ISPR[8], RESERVED2[24], ICPR[8], RESERVED3[24],
        IABR[8], RESERVED4[24], IPR[8]; 
};

/* NOTE: More like a programmer's model of the NVIC */
#define NVIC ((struct nvic *) 0xE000E004)

void nvic_status(void);
void enable_irq(uint32_t interruptno);

#endif
