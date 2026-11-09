
---
title: Programming
nav_order: 4
---

# Programming the BB-8

Programming the BB-8 can be done in either machine code or Assembly. On the surface, they are very different, but both fundamentally perform the same task.

## Assembly

[See more about BoxASM](boxasm.md).

Assembly is clean, supports comments, hexadecimal, decimal and ASCII conversion. It also allows for functions (and jumping to them), and is dynamically sized and automatically compresses code to reduce file size.

```asm
@0000

; this is an example program

#PROG
  LDA *e             ; 'e' as a literal ASCII character
  PAA

  LDB $20            ; 0x20, which corresponds to a space in ASCII
  PAB
  JMP &PROGH &PROGL  ; this loops back to the start
```

## Machine code

Machine code offers more direct control over the hardware, but is considerably less user-friendly.

It is written purely in hexadecimal; there are no comments, no built-in conversions from decimal or ASCII, and each function needs to be spaced out from one another to allow for changes that may alter the size and layout of the program.

```text
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
> These two examples are actually the same program! One was written in Assembly, and the other was compiled into hexadecimal machine code.
