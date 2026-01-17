# Blinky LED Writeup

## Setup

### Components

* arm-none-eabi-gcc, toolchain for compiling and linking
* st-link, utility for flashing
* make, for building
* STM32F446RE Nucleo Board
* jumper wires
* breadboard
* usbA to miniB cable for power
* small red LED
* 68-ohm resistor

### Hardware Layout

I could draw a diagram or take a picture, but I'll do it later because lazy.

1. Power will be coming in from a GPIO pin. For me, its set to Pin 1 of CN7.
   Fit a jumper cable on that pin for a voltage source of 3.3V.

2. Connect a red LED and 68-ohm resistor in series to protect the LED.

3. Fit a jumper cable from the end of your load to any ground pin on the board
   to complete the circuit.

4. Finally power up the board by connecting the USB to your host PC and board.

## What I Learned

I ended up with a better understanding of how bare-metal programming works
from the ground up.

### A general overview of how a program runs on an MCU.

Upon powerup (connecting the USB to the Nucleo board), the hardware on the
board gets initialized, then the first program related thing that executes
is the reset handler in the interrupt vector table. The interrupt vector table
(IVT) is an array of pointers (addresses) that point to various interrupt
service routines (functions). These interrupt service routines are only
executed upon the CPU getting an interrupt on the interrupt line, either from
hardware or software interrupt. Then by examining a specific code in the
interrupt, the interrupt handler (the software that handles interrupts) can
determine the index in the IVT to determine which interrupt service routine
to execute next.

Anyway, the reset handler is the 2nd entry in the IVT, and is the first
function to execute in our firmware. The responsibility of the reset handler
is to set up a couple of things in order for any C program to be able to run
smoothly. Our reset handler is basically our startup code, the stuff that
runs before our main function. In our case, the reset handler initializes our
global data variables in the data section by copying the values over from
flash memory (non-volatile) into sRAM (volatile). Then it zeroes out the
bss section in sRAM.

Next we enter into our main function, and this is where we can begin setting
up our peripherals. We could've set our peripherals in our startup code, but
usually the startup code is bare minimal and should only be used to set up
a working C environment (like what crt0 does). So in the main, we set up
the GPIO pins that we'll need to blink our LED.

We defined our own API for interacting with the GPIO pins. This part was
pretty fun as we get to set bits in registers, and really consolidate
this concept of memory-mapped I/O. I feel like most of what embedded
programming is is just memory-mapped I/O... interacting with registers in
order to make something happen in the hardware, that'll have an effect in
the real world. While creating this API, I learned about the RCC which is
the Reset and Clock Control unit. It's closely related to the system clock
on the board, which has a specific frequency (cycles per second). And any
machine-code instructions that executes takes a specific amount of cycles
to complete, we call this cycles-per-instruction (CPI).

The RCC also has a register that allows us to enable our GPIO peripherals,
which is what we did in our main function. After setting up our GPIO correctly,
we enter an infinite loop where we blink our LED.

Our program only stops when we power it off.

So far I've only explained how the program runs on our MCU, but there's way
more stuff that I learned before actually programming on the MCU.

### An examination of compilers, linkers, and memory.

In construction work (I'm talking the manual labour kind, where you build
houses or something), before we get to building, we need the right tools.
And before we use our tools, we need to understand their purpose and what
they accomplish. We don't need to know anything about the internals of how
our tools are implemented, just how to operate it and accomplish the goals
we set out for ourselves (we don't really need to know how a drill works
internally, just how to use it and get that screw in the wooden board).

This is analogous to the toolchain and commands that are provided to us
when we get started in bare-metal programming. Now keep in mind, I didn't
just go into this blindly. I followed a bare-metal programming guide on
GitHub, with the goal of understanding what all these tools are. And this
is what I gleamed from my research using online guides, videos, and
documentation.

### Compiler

The arm-none-eabi-gcc toolchain consists of a compiler that is meant to
compile source code into object code. The command to invoke the compiler
is the same as before; arm-none-eabi-gcc.

What the compiler does is it examines your source code, determines which
parts of your code goes into various sections (data, text, bss, or user-defined
sections like .vector), and produces an object file consisting of machine-code
instructions and data.

Compilers provide certain features or extensions to the C programmer that allow
them to tell the compiler to do something. One of these is the __attribute__
keyword, which is specific to GCC. And we use this __attribute__ keyword to
place our IVT in its own section .vectors.

So the main purpose of a compiler is to translate human-readable source code
into something a computer can understand. But we can also tell the compiler
to do some things different using compiler specific extensions or features.

### Linker and Memory

I used to program in C, but not in the embedded space. I wrote C programs that
were meant to run on Linux operating systems, and they were meant to do
something useful and exit gracefully. During this time, I had no need to
dive deeply into linkers. I knew what they did at the surface level, and what
it did by default was completely fine for my purposes. But this time, the
MCU is a completely different environment, so I had to write my own linker
script to tell it what to do.

To me, the linker has two primary purposes. One is to combine all object files
and resolve any external symbol references in each file. That was the surface
level point of knowledge that I knew before starting bare-metal programming.

The second one, and the one I learned about while writing the linker script,
is to tell the linker how I want my program to be laid out in the MCU's
memory. What memory is exposed to the linker? Flash and sRAM. What output
sections do I want in my final executable? .vectors, .data, .bss, etc. Where
should these sections be loaded or flashed in the MCU's memory? What are
each of the sections virtual memory address, the address it will have at
runtime?

These are all questions that need to be answered in the linker script, and
told to the linker in order to have an environment where we can run our C
programs smoothly.

## What issues I had

The concept of memory-mapped I/O, interacting registers, and memory sections
aren't entirely foreign to me (they're covered in most CS degree programs I'd
imagine). But having to actually work with them and interact with them
directly was difficult, as it was my first time doing so. Working was slow,
but that was because I really took my time understanding and researching ideas
and concepts that seemed new to me.

I spent a lot of time sifting through documentation, articles, and community
forums to understand a lot of the flags passed into the compilers and linkers,
learning about linker scripts, the build process, and MCU architecture.

These aren't really issues tbh, and I feel these are natural obstacles that
come up when diping your toes into a new environment.

## How I overcame them

Again, just doing a lot of reading and understanding before moving on.
