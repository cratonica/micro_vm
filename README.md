# micro\_vm
A small virtual machine written in C++, complete with an assembler and a debugger.

### Overview

This is a virtual machine that simulates execution of a nonexistent 16-bit CPU. It includes tools for assembling machine code from assembly code, a dissasembler, and a graphical debugger. 

### Building

micro\_vm uses [SCons](http://www.scons.org/) to build. Use the `scons` command to build, and `scons -c` to clean up. This is also wrapped up in a Makefile, but you need to have SCons installed in order to build. 

You will also need to install the ncurses development headers; on Debian-based Linux distros this can be found in the `ncurses-dev` package.

### Running
Once you have built the VM, you can execute the test program by running `./cpu out.bin`. It will present a blank prompt. Enter 2 numbers and the VM will add them together:

```
./cpu out.bin
2
4
6
```

More test program can be found in the progs/ directory

### Datasheet

#### CPU
The CPU is a 16-bit little-endian RISC processor with a few high-level instructions for interacting with console input and output.

##### Registers
* `A` (8 bit) 
* `X` = I[High] 
* `Y` = I[Low] 
* `FP` (16-bit) 
* `SP` (8-bit offset from FP)
* `IP` (16-bit) 
* `F` (flags)
  * Zero
  * Carry
  * Sign

`SP` points to next available element in stack (uninitialized). Therefore, it should never be accessed directly by program code.

##### Instruction Set

* Operations
  * `BAD`:   Invalid
  * `NOP`:   Nothing
  * `ADD`:   `A = A + *X:Y`
  * `ADD1`:  `A = A + Literal`
  * `AND`:   `A = A & *X:Y`
  * `AND1`:  `A = A & Literal`
  * `OR`:    `A = A | *X:Y`
  * `ORL`:   `A = A | Literal`
  * `CMPL`:  `A = ~A`
  * `INCA`:  `++A`
  * `INCX`:  `++X`
  * `INCY`:  `++Y`
  * `INCI`:  `++I`
  * `DECA`:  `--A`
  * `DECX`:  `--X`
  * `DECY`:  `--Y`
  * `DECI`:  `--I`
  * `DIV`:   `I = I / A; A = I % A`
  * `MUL`:   `I = A * Y` (8-bit operands, 16-bit result)
* Compare
  * `CMP`:   `Flags` = `A` Compared to `*X:Y`
  * `CMP1`:  `Flags` = `A` Compared to `Literal`
* Memory
  * `LDA`:   `A = *X:Y`
  * `LDA1`:  `A = Literal`
  * `LDX1`:  `X = Literal`
  * `LDY1`:  `Y = Literal`
  * `DREF`:  `I = *I`
  * `LAS1`:  `A = SP[Literal]`
  * `LIS1`:  `I = SP[Literal]`
  * `STOR`:  `*I = A`
  * `TXA`:   `A = X`
  * `TYA`:   `A = Y`
  * `TAX`:   `X = A`
  * `TAY`:   `Y = A`
  * `TSPI`:  `I = FP + SP`
  * `LDI2`:  `I = Literal`
* Stack manipulation
  * `PSHA`:  `*SP = A; ++SP`
  * `PSHX`:  `*SP = A; ++SP`
  * `PSHY`:  `*SP = A; ++SP`
  * `PSH1`:  `*SP = Lit8; ++SP`
  * `PSH2`:  `*SP = Lit16; SP += 2`
  * `POPA`:  `--SP; A = *(FP + SP)`
  * `POPX`:  `--SP; X = *(FP + SP)`
  * `POPY`:  `--SP; Y = *(FP + SP)`
  * `POP1`:  `SP -= Lit`
  * `POP`:   `--SP`
  * `LAL1`:  `A = FP[Literal]` (Load `A` from local)
  * `LAP1`:  `A = FP[-5 - Literal]` (Load `A` from parameter)
  * `LIP1`:  `I = FP -5 - Literal` (Load `I` from parameter address)
  * `LIL1`:  `I = FP + Literal` (Load `I` from local address)
  * `STL1`:  `FP[Literal] = A`
* Flow Control
  * `JMP2`:  `IP = Literal`
  * `JGE2`:  `If F[Zero] || !F[Sign], IP = Literal`
  * `JG2`:   `If !F[Sign], IP = Literal`
  * `JL2`:   `If F[Sign], IP = Literal`
  * `JLE2`:  `If F[Zero] || F[Sign], IP = Literal`
  * `JE2`:   `If F[Zero], IP = Literal`
  * `JNE2`:  `If !F[Zero], IP = Literal`
  * `JC2`:   `If F[Carry], IP = Literal`
  * `CAL2`:  `*(FP+SP)=FP; *(FP+SP+2)=SP; *(FP+SP+3)=IP; FP+=SP+5; SP = 0; IP = Literal` (Call subroutine)
  * `RET`:   `IP = FP[-2]; SP = FP[-3]; FP = FP[-5]` (Return from subroutine)
* I/O
  * `PRNT`:  Print `A` as ASCII byte
  * `KEYB`:  Read key into `A`. \0 is end of input
* Flags
  * `CC`:    Clear Carry
* Debug
  * `BRK`:    Debug break (`NOP` if no debugger)

### Tools
#### Assembler
Assembles input assembly code into a machine code file (`out.bin`) and a debug metadata file (`out.dbg`)

Usage: `./asm input.asm`

#### Disassembler
Disassembles machine code into human-readable assembly code and outputs it to stdout.

Usage: `./disasm out.bin`

#### Graphical Debugger
Runs a program interactively on a debugger GUI (written in ncurses).

Usage: `./debug out` (Notice no '.bin')

The enter key steps the program forward, and the 'r' steps the program backwards. 
