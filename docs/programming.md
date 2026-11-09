# Programming the BB-8
Programming the BB-8 can be done in either machine code or Assembly. On the surface they are very different, but both fundamentaly perform the same task.
## Assembly
- [see more about BoxASM](boxasm.md)
Assembly is clean, supports comments, hexadecimal, decimal and ASCII conversion. It also allows for functions (and jumping to them) and is dynamically sized and automatically compresses code to reduce filesize:
```asm
@0000

;this is an example program

#PROG
  LDA *e             ;'e' as a literal ASCII character
  PAA

  LDB $20            ;0x20, which corresponds to a space in ASCII
  PAB
  JMP &PROGH &PROGL  ;this loops back to the start
```
## Machine code
Whereas Machine code offers more direct control over the hardware, but is less user freindly by miles. It is written purely in hexadecimal; there are no comments, no inbuilt conversions from decimal or ASCII and each function needs to be spaced out from one another to allow for any changes that may alter the size and shape of the program. This means that Assembly usualy takes up more space in memory, and is less easy to program and read:
```
0x0000: eb
0x0001: eb
0x0002: 11
0x0003: 65
0x0004: 94
0x0005: 13
0x0006: 20
0x0007: 95
0x0008: 02
0x0009: 00
0x000a: 01
```
> [!NOTE]
> These 2 examples are actually the same program! One was written in assembly, and the other one was compiled into hex!
