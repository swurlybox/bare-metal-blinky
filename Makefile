# Enable a bunch of warnings, maximum debug info, optimize for size, place \
each function and data item in its own section, look relative to current dir \
for included header files, which target architecture we're compiling for, \
operate on thumb state (16-bit ISA), use hardware floating point instructions \
fpu version is: fpv4 architecture, single precision, 16 registers
CFLAGS  ?=  -W -Wall -Wextra -Werror -Wundef -Wshadow -Wdouble-promotion \
            -Wformat-truncation -fno-common -Wconversion \
            -g3 -Os -ffunction-sections -fdata-sections -I. \
            -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 \
            $(EXTRA_CFLAGS)
# Use our own linker script, do not use standard system startup files (like \
crt0 or entry point to main, since we're basically defining our startup in our\
firmware code), don't include standard library, use the size optimized version\
of Newlib (the C standard library used by ARM GCC), link in libc and libgcc \
from Newlib (former is standard c library, latter is low-level arithmetic api\
thats used if the target processor can't perform some computation in hardware)\
, garbage collect unused sections of code TODO: -Wl,--gc-sections , print a \
link map to a mapfile
LDFLAGS ?=  -Tlink.ld -nostartfiles -nostdlib --specs nano.specs -lc -lgcc \
            -Wl,--gc-sections -Wl,-Map=$@.map
SOURCES =   main.c startup.c gpio.c systick.c


build: firmware.elf

firmware.elf: $(SOURCES)
	arm-none-eabi-gcc $(SOURCES) $(CFLAGS) $(LDFLAGS) -o $@

firmware.bin: firmware.elf
	arm-none-eabi-objcopy -O binary $< $@

flash: firmware.bin
	st-flash --reset write $< 0x8000000

clean:
	rm -rf firmware.*
