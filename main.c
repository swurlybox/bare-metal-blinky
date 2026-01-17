#include "gpio.h"
/* 
    Which gpio pin do we want to enable to blind our LED? 
    Let's do PC10, which corresponds to GPIOC13, which is located at
    Pin 1 of CN7.
*/
int main(void) {
    uint16_t led = PIN('C', 10);            /* vdd pin for led */
    gpio_bank_enable((uint8_t) PINBANK(led));         /* enable bank c*/
    gpio_set_mode(led, GPIO_MODE_OUTPUT);   /* set pin for output */
    for (;;) {
        gpio_write(led, true);
        spin(999999);   // delay depends on clock frequency and ipc.
        gpio_write(led, false);
        spin(999999);
    }
    return 0;
}

// Startup code (firmware's entry point)
__attribute__((naked, noreturn)) void _reset(void) {
    /* As specified by our ld linker script, our executable has the
        following memory layout, from low address to high address 

    .vectors        contains ivt
    .text           program instuctions (functions)
    .rodata         read only data, (string literals)
    .data           rw global initialized data
    .bss            rw global uninitialized data (default 0)

    when the program runs,
    .vectors, .text, and .rodata all live within flash memory, and is read
    from there. These are not writable.

    .data lives in both areas, flash and sram.
    The reason is that we want our .data to be both writable and persist
    on power off.
    Well, flash isn't writable during program run-time, but data does persist.
    Sram is writable during program run-time, but doesn't persist on power off.

    So the design decision is to have the .data's LMA (load memory address)
    on flash memory, and its VMA (Virtual memory address) in the sRAM. In
    other words, access to the .data section will refer to the .data section
    in sRAM, while the actual data lives in flash.

    .bss section will only live in sram, and we know its all zeroes, we can
    zero that section out in our reset function.
    */

    /* zero out bss */
    /* Initialize the data section, loading from flash to ram */
    extern long _sbss, _ebss, _sdata, _edata, _sidata;
    for (long *dst = &_sbss; dst < &_ebss; dst++) *dst = 0;
    for (long *dst = &_sdata, *src = &_sidata; dst < &_edata;) *dst++ = *src++;

    main();             // Call main()
    for (;;) (void) 0;  // Infinite loop in case main() returns
}

/* stack point register */
extern void _estack(void);  // Defined in link.ld

// 16 standard and 96 STM32F446RE-specific handlers (may be more, check F446RE
// datasheet). First 2 pointers in the vector table point to _estack and
// _reset. Store this array in the .vectors section of memory.
__attribute__((section(".vectors"))) void (*const tab[16 + 91])(void) = {
    _estack, _reset
};
