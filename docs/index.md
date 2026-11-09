---
title: Home
nav_order: 1
---

# BB-8 Computer

Welcome to the technical documentation for BB-8.

BB-8 is a custom-built 8-bit computer, designed around
a von Neumann architecture.

## Abstract
Over the past few years I have become increasingly interested in computer programming and hardware. And after finding myself with nothing to do one evening, I decided it would be fun to create a custom CPU architecture. Since I'd recently learned about microcode, that was my design of choice. Having watched many videos on similar projects (like [Ben Eater's 8-bit breadboard CPU][ben-eater-cpu] and [Astro Sam's Astro-8][astro-8]), I was familiar with the different subcomponents of a processor:
- Registers
- Memory management
- ALU (arithmetic logic unit)
- External Ports
- With the addition of the MCU (microcode unit)

I wanted to eventually build the CPU on breadboards and perfboards, so it was necessary to ensure every part of the CPU could be tested separately. Because of this crucial fact, I decided to utilise a single main data bus to communicate with every subcomponent. This allowed every part of the processor to be tested by hand or with a microcontroller (eg: arduino, esp32, etc). I also ditched memory mapped I/O, as it isn't important enough to me when using a basic text terminal. I wrote the outline for the instruction set on paper - what it should be capable of at minimum - before creating a working model in Logisim (a tool for designing and simulating logical circuits).

# Whats to come
For a while now I have been consistently working on this project, and am currently developing:
- an emulator written in C++ for faster programs
- a compiler for a higher level language called 'Malt'
- continuing updating and fixing the existing software

[ben-eater-cpu]: https://www.youtube.com/playlist?list=PLowKtXNTBypGqImE405J2565dvjafglHU
[astro-8]: https://www.youtube.com/watch?v=Zt0JfmV7CyI
[boxasm-git]: https://github.com/Dabigboi2000/boxasm


## Documentation

Explore the different sections:

- [Architecture](architecture.md)
- [Programming](programming.md)
- [BoxASM](boxasm.md)

## Project overview

Add an introduction to your computer here.