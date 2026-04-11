# General Info

From blinky to MP3 player. I wanted to learn bare-metal programming on STM32,
and decided to choose an MP3 player project. Why? Because I like listening
to music, it seemed easy from a high-level design perspective, and I did it
for the love of the game.

I didn't depend on any vendor-specific code, though I did occasionally look
at their code examples and suggested clock configurations for my specs.

Every piece of code, I wrote and integrated by hand. No auto-generated code,
no AI, no HAL library. I wrote every peripheral and device drivers on my own.
As a result, I learned quite a lot about how embedded development works under
the hood, just by peeling off the layers of abstraction and doing things the
hard way.

The exception to the above statement are the two open-source libraries I
integrated to help make some core functionalities easier to deal with. The
FatFs module is a filesystem driver that has excellent documentation, so I
understand how the FAT filesystem works, as well as how to use the library.
The minimp3 module is an mp3 decoder, which reads a chunk of mp3 data from
a file and uncompresses into raw PCM to be sent to a DAC for audio playback.
I've read the mp3 specification (though most of it is paywalled, there are
resource out there...) and I can tell you, implementing it myself would be
like pushing a boulder up a hill.

# Features

- Basic FAT32 Filesystem Navigation
    - Up and Down buttons: cycle through directory contents.
    - Select button: On a subdirectory, opens that subdirectory for viewing
        (similar to cd). On a file, doesn't do anything special yet, but
        if its a valid audio file (we'll stick with mp3), the system
        will change into a media-playing state and play the audio.
    - Cancel button: Go back to parent directory.
- Audio Playback
    - Upon selecting an mp3 file, audio playback will begin.
    - Select button: Pauses/Plays the file.
    - Cancel button: Go back to filesystem navigation.

# Notes / Considerations

I'm sure there are thing that I could've done a lot better. If you care, you
can leave me some feedback by sending an email.

There were a few things I wanted to implement, but haven't for whatever reason:
    - audio visualizer in playback state
    - seeking forward and backward in a song
    - bluetooth support for audio
    - battery management system

I haven't included diagrams yet, but I'll get to it soon (tm).

# Hardware

- STM32F446RE Nucleo Board
- microSD card reader w/ SPI interface module
- 4GB (SDHC) microSD card (anything higher than 32GB (SDXC) won't work)
- I2C OLED display (SSD1306 driver)
- TLV320DAC3100 audio DAC
- wired earbuds, or a speaker
- red LED, w/ an accompanying LED protecting 68-ohm resistor
- usbA to miniB cable for power
- breadboard, whole lotta jumper wires

# Software

## Build Tools

- arm-none-eabi-gcc, cross-compiler
- st-link, binary image flashing utility
- make, for build system

## Dependencies

- FatFs - Generic FAT Filesystem Module by ChaN
- minimp3 - MP3 Decoding library by lieff and contributors
