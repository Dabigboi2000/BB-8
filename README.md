# BB-8
## Abstract
Over the past few years I have become increasingly interested in computer programming and hardware. And after finding myself with nothing to do one evening, I decided it would be fun to create a custom CPU architecture. Since I'd recently learned about microcode, that was my design of choice. Having watched many videos on similar projects (like [Ben Eater's 8-bit breadboard CPU][ben-eater-cpu] and [Astro Sam's Astro-8][astro-8]), I was familliar with the different subcomponents of a processor:
- Registers
- Memory management
- ALU (arithmatic logic unit)
- External Ports
With the addition of the MCU (microcode unit), and began creating a design. I wanted to eventualy build the CPU on breadboards and perfboards, so it was nessicary to ensure every part of the CPU could be tested seperately. Because of this crucial fact, I decided to utilise a single main data bus to communicate with every subcomponent. This allowed every part of the processor to be tested by hand or with a microcontroller (eg: arduino, esp32, etc). I also ditched memory mapped I/O, as it isn't important enough to me for basic a text terminal. After deciding I wrote the outline for the instruction set on paper - what it should be capable of at minimum - before creating a working model in Logisim (a tool for designing and simulating logical circuits).

## Functional Description
The internal organisation of the BB-8 is split into 3 main parts: 1) the register section, 2) the arithmetic section and 3) the control section. Instructions obtained from memory are stored in the IR (instruction register), which is located in the control section. The MCU then performs all the necessary data bus signals to perform transfers within and between the arithmetic section and the register section.

### Instruction Register (IR)
Byte long instructions fetched from memory are stored in the instruction register (IR). The instruction register value corresponds to an index in the MCU ROM. The IR increments as the MCU performs each data bus signal until the current instruction is complete, then fetching a new instruction.

### Microcode Unit (MCU)
The Microcode Unit provides all the internal data bus signals in the CPU. The value loaded in the Instruction Register corresponds to an address within an internal, pre-loaded 1kB Read Only Memory (ROM) chip. Each byte stored in the MCU ROM directly controls the internal data bus. As the IR counts, the MCU sends the appropriate data bus signals for whatever instruction is being executed. The data bus signal 0xFF corresponds to a new instruction fetch, and a new address is loaded into the IR. The full contents of the MCU ROM is shown later in the datasheet, alongside a more in-depth explanation of the MSD and LSD functions.

### Internal Data Bus
The BB-8 is built around an 8-bit internal data bus. This bus controls the individual functional units within the CPU (Eg: ALU, Memory management, etc.) The LSD (least significant digit or low nibble), corresponds to what unit is enabled at any given time, including ports, the ALU, memory, etc. The MSD (most significant digit or high nibble) is used as a control value for each unit, and is for:
Selecting a sub-section of a unit (addition in the ALU, register 3 of the registers, etc)
Sending an event to a unit or port (clearing a screen, IF statements, etc)

### Arithmetic Logic Unit  (ALU).
All arithmetic and logic takes place in the Arithmetic logic unit (ALU). The ALU, when enabled, is capable of loading the current accumulator value into 2 separate registers integrated into itself. It then uses these 2 8-bit values for either a conditional or arithmetic function. The ALU is capable of unsigned integer addition, subtraction, multiplication and division as well as a bit shifter. Alongside the arithmetic functions, the ALU is also capable of conditional statements. The ALU outputs 5 flags (ZER, CAR, LES, GRE and EQU) and 3 inverted flags (!ZER, !CAR and !EQU). The flags, when selected and if true, signal a new instruction fetch, interrupting the MCU’s current instruction, acting as a conditional statement.

### Accumulator Register (ACC)
The Accumulator Register (ACC) acts as a general purpose register for temporarily holding data from the ALU, memory or ports to transfer to other registers and vice versa. It is also used to transfer data from one register to another. Normal CPU instructions cannot access this register as it is only for moving data.

### Register 0 - Register 7 (R0 - R6)
7 general purpose, 8-bit registers that are accessible by the rest of the CPU via the ACC.

### Program Counter
The BB-8 features a 16-bit program counter used for accessing the current CPU instruction, and for reading and writing to and from memory.


[ben-eater-cpu]: https://www.youtube.com/playlist?list=PLowKtXNTBypGqImE405J2565dvjafglHU
[astro-8]: https://www.youtube.com/watch?v=Zt0JfmV7CyI
