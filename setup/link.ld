/* tells the linker the value of the entry point attribute in the generated
    ELF header - so this is a duplicate to what a vector table has. This is
    an aid for a debugger, that helps set a breakpoint at the beginning of
    the firmware. A debugger does not know about a vector table, so it relies
    on the ELF header. */
ENTRY(_reset);

/* Tells the linker we have two memory regions in the address space */
MEMORY {
    flash(rx)   : ORIGIN = 0x08000000, LENGTH = 512k
    sram(rwx)   : ORIGIN = 0x20000000, LENGTH = 128k
}

/* _estack symbol will resolve to a memory address at then end of sram 
    our stack will grow downwards in the sram.
*/
_estack = ORIGIN(sram) + LENGTH(sram);

/* where we define the data sections in our output executable */
SECTIONS {
    /* .vectors, .text, and .rodata in all input files are stored in the
        corresponding sections in flash memory. 
        KEEP has to do with garbage collection. If none of the input files
        passed into the linker has a specific section, the KEEP keyword ensures
        that section is still kept in the output file. */
    .vectors    : { KEEP(*(.vectors)) } > flash
    .text       : { *(.text*) }         > flash
    .rodata     : { *(.rodata*) }       > flash

    /* all .data sections and .data.* sections from all input files are
        stored in the .data section of the final executable, stored in
        sram and flash */
    .data       : {
        _sdata = .; /* .data section start */
        *(.first_data)
        *(.data SORT(.data.*))
        _edata = .; /* .data section end */
    } > sram AT > flash
    _sidata = LOADADDR(.data);

    /* All inputs files that contain a .bss section, .bss.* section, or
       uninitialized data (COMMON), are put into an output .bss section stored
       in sram */
    .bss : {
        _sbss = .;
        *(.bss SORT(.bss.*) COMMON)
        _ebss = .;
    } > sram

    . = ALIGN(8);
    _end = .;
}
