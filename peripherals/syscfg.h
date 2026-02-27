#ifndef SYSCFG_H
#define SYSCFG_H

#include <stdint.h>

struct syscfg {
    volatile uint32_t MEMRMP, PMC, EXTICR1, EXTICR2, EXTICR3,
        EXTICR4, CMPCR, CFGR;
};

/* APB2 : must enable bit 14 in rcc->apb2enr */
#define SYSCFG ((struct syscfg *) 0x40013800)

void syscfg_init(void);

#endif
