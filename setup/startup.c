/*
    The startup firmware code for the STM32F446RE-NUCLEO board. Initializes
    the data and bss sections in RAM, as well as initialize our Interrupt
    Vector Table.
*/

// RESET_HANDLER (Firmware entry point)
__attribute__((naked, noreturn)) void _reset(void) {

    extern int main();
    extern void enable_fpu();
    /* Symbols are defined in link.ld */
    extern long _sbss, _ebss, _sdata, _edata, _sidata;
    for (long *dst = &_sbss; dst < &_ebss; dst++) *dst = 0;
    for (long *dst = &_sdata, *src = &_sidata; dst < &_edata;) *dst++ = *src++;

    /* enable floating point coprocessor */
    enable_fpu(); 

    main();             
    for (;;) (void) 0;  
}

extern void _estack(void);          // Defined in link.ld
extern void SysTick_Handler(void);  // Defined in systick.c

// 16 standard and 96 STM32F446RE-specific handlers
__attribute__((section(".vectors"))) void (*const tab[16 + 96])(void) = {
    _estack, _reset, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, SysTick_Handler
};

