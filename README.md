# General Info

The repository is called bare-metal-blinky, because that's what this repo was
initially for. Just some firmware code to blink an LED on an STM32
microcontroller.

Now its transitioned into a portable media player project, which hopefully
teaches me a thing or two about embedded software development on bare-metal.

# Features

- microSD card storage
- FAT32 Filesystem Navigation

# Notes / Considerations

The focus is on understanding every part that goes into the system. The way
I'm approaching this project is that I'll try to write most of the software
on my own, but if I realize it's becoming too much of a hassle, I'm not shy
about using external open-source libraries to do some of the work for me.

Progress will be glacial, and that's okay. I'm doing my time reading and
learning about things as I go along. The end goal is something that I enjoyed
the process of doing, something that I'm proud of, and something I can show
off on my resume.

# Hardware

- STM32F446RE Nucleo Board
- microSD card reader w/ SPI interface module
- 4GB (SDHC) microSD card
- red LED
- usbA to miniB cable
- breadboard
- 68-ohm resistor
- jumper wires

# Software

## Build Tools

- arm-none-eabi-gcc, cross-compiler
- st-link, binary image flashing utility
- make, for build system

## Dependencies

- FatFs - Generic FAT Filesystem Module by ChaN
