#include "cpu_config.h"
#include <stdint.h>

/* NOTE: It's alright, but could be a bigger struct and contain more than just
    1 register.  */
struct cpu_conf {
    volatile uint32_t CPACR; 
};

#define BIT(x) (1U << (x))
#define FPU ((struct cpu_conf *) (0xE000ED88))

/* Enable CP11 and CP10 of CPACR to enable floating point coprocesser */
void enable_fpu(void) {
    FPU->CPACR |= ((3U << 20U) | (3U << 22U));
}
