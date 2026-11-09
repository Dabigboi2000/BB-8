# Box ASM
## About
<img width="128" height="128" alt="BoxASM" src="https://github.com/user-attachments/assets/b59eea33-938d-44f9-8c8f-51dde80a9da6" />
(dont mind the logo, i know it sucks)

The native supported assembly for the BB-8 CPU is called BoxASM (or Box Assembly), and the syntax highlighting for VSCode can be found in [the BoxASM repository][boxasm-git]. Programs written in BoxASM are composed of 5 different elements:
- Comments and comment blocks
- Memory allocation
- CPU instructions
- Literal values
- Functions and declarations

### Comments and Comment Blocks

Comments are written with a ';' and last till the next line. Comment blocks are written as ";**;" and last till the closing pair:
```asm
;this is a comment

;*
this is a comment block
\_(*-*)_/
*;
```
### Memory Allocations
Memory allocations are placed at the start of every assembly file to be ran. This tells the compiler where the Program will be stored in memory, and adjusts the function calls to the correct offset:
```asm
@0000    ;this tells the compiler the program starts at 0x0000
```
> [!NOTE]
> For normal use, this rexample above is absolutely fine, but the address must match where it is stored and ran.

### CPU instructions
Every assembly instruction coresponds to a machine code instruction.
```asm
JMP    ;is converted to 0x02
AA0    ;is converted to 0x68
```
### Literal Values
Some instructions require constants:
- Register A + 1
- Register C = 3
and to load these constants, literal values are used. In both assembly and machine code, instructions that require an argument will take in the next line. In BoxASM a '$', a '%' or a '*' represents an immediate value. The '$' allows for a hex number, the '%' allows for a decimal number and the '*' an ASCII character. This means that each type of constant is converted differently:
```asm
$13    ;0x13
%10    ;0x0a
*a     ;0x61
```
as previously stated, some intructions require an argument to be passed through in the next space. We can now use these literal values to do that:
```asm
LDA $13 ;this sets register a to 0x13
```
> [!NOTE]
> Technicaly you could write an entire program just using literal values, because every CPU intruction coresponds to a number!

### Functions and Declarations
From my experience programming in machine code, I found the most annoying part to be the functions. Having to jump around aimlessly in memory and having to either keep a long page of notes or read each resulting chunk of code manualy (just to know where you are) is a brainf**k (no pun intended). I also wanted a way to easily create loops and indexes that move with the code, so every time you shift code over you dont have to update the adresses.

Let us create a simple program:
```asm
@0000    ;start of program

LDA *e
PAA
HLT
```
This will send the ASCII character 'e' (0x65) to port A. But what if you want to send it over and over endlessly for no apparent reason? To do such a thing we use functions. The '#' (hash) creates a function, and the compiler keeps track of its current address. The '&' symbol will insert the either high or low byte into the program when compiled, depending on the leading character being a 'H' or 'L' respectively. We can use this to add an infinite loop to the program:

```asm
@0000    ;start of program

#LOOP
  LDA *e
  PAA
  JMP    ;takes the next 2 lines as arguments for destination
  &LOOPH ;the high byte of the LOOP function address
  &LOOPL ;the low byte of the LOOP function address
```
> This will print: eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee...

This can also be used for jumping between functions:

```asm
@0000    ;start of program

#INIT
  LDA *[
  PAA
  JMP    ;takes the next 2 lines as arguments for destination (loop function)
  &LOOPH ;the high byte of the LOOP function address
  &LOOPL ;the low byte of the LOOP function address

#LOOP
  LDA *e
  PAA
  JMP    ;takes the next 2 lines as arguments for destination (itself)
  &LOOPH ;the high byte of the LOOP function address
  &LOOPL ;the low byte of the LOOP function address
```
> This will print: [eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee...

> [!IMPORTANT]
> The first function or line written in BoxASM will always run first, then continue down! Make sure to keep this in mind.
