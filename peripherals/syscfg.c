#include "syscfg.h"
#include "rcc.h"

#define BIT(x) (1U << (x))

/* enable syscfg clock to allow external interrupts */
void syscfg_init(void) {
    RCC->APB2ENR |= BIT(14);
}
