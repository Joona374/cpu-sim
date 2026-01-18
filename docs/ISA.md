# Instruction Set Architecture (ISA)
## Table of Contents
* [Overview](#overview)
* [CPU State](#cpu-state)
* [Instruction Encoding](#instruction-encoding)
* [Instruction Set](#instruction-set)
* [Memory Map](#memory-map)
* [Addressing Modes](#addressing-modes)
* [Control Flow Semantics](#control-flow-semantics)
* [Notes on Microarchitecture (Optional)](#notes-on-microarchitecture-optional)


## Overview
The ISA used in this CPU simulator is my attempt at designing a set of 16 instructions that balance simplicity and functionality. The ISA supports basic arithmetic, data movement, and control flow operations. It uses a fixed 4-bit opcode format, variable-length operands, and an 8-bit address space.

## CPU State
While the CPU is ACC centric, it also includes a multiple other registers to facilitate various operations. Some of the registers are visible to the programmer, while others are used internally by the CPU for control and temporary storage.
Programmer-visible registers are those that can be directly read, written, or affected through ISA instructions.  
Internal registers exist solely to support instruction execution and are not directly accessible via software.


### Visible Registers:
- **Accumulator**: Primary register for arithmetic and logic operations. 4 bits wide.
- **Program Counter**: Holds the address of the next instruction to be executed. 8 bits wide.
- **MAR**: Memory Address Register, holds the address of the memory location (ROM, RAM or I/O) to be accessed. 8 bits wide.
- **Flag Register**: Contains status flags (Zero). 1 bit wide.
- **R0** General Purpose Register 0. 4 bits wide.
- **RET_PC**: Return Program Counter, holds the return address for subroutine calls. 8 bits wide.

### Internal Registers:
- **Instruction Register**: Holds the currently executing instruction. 4 bits wide.
- **ALU_Temp**: Temporary register used to hold the value of ALU to allow atomic updates to the ACC. 4 bits wide.
- **TEMP_LO & TEMP_HI**: Temporary address registers used to hold low and high nible of addresses. 4 + 4 bits wide.
- **Microprogram Counter (µPC)**: Holds the address of the next micro-instruction to be executed in the microcode ROM. 5 bits wide.


## Instruction Encoding
All instructions are encoded in binary using a fixed 4-bit opcode followed by zero or more operand words. 
Instructions may consume between 1 and 3 words depending on operand requirements. For example, STA uses one opcode word followed by two address words.

Example (STA 0xC2):
- Opcode word: STA
- Operand word 1: 0xC (high nibble)
- Operand word 2: 0x2 (low nibble)


## Instruction Set
> **Note:** For the purposes if this table imm4 refers to a 4-bit immediate value, MEM[7:4] refers to the high nibble of an 8-bit memory address, and MEM[3:0] refers to the low nibble of an 8-bit memory address.

| BIN | MNE | 2. word | 3. word | DESCRIPTION |
| ----|-----|----------|----------|-------------|
|0000 | NOP | - | - | No operation
|0001 | LDI | imm4 | - | Immediate => Accumulator
|0010 | ACC2R0 | - | - | R0 <= ACC
|0011 | ADDI | imm4 | - | ACC += Immediate (might overflow)
|0100 | STA | MEM[7:4] | MEM[3:0] | Store Accumulator to Memory
|0101 | LDA | MEM[7:4] | MEM[3:0] | Load Accumulator from Memory
|0110 | SUBI | imm4 | - | ACC -= Immediate (might underflow)
|0111 | JMP | MEM[7:4] | MEM[3:0] | Jump PC to MEM[7:0]
|1000 | JZ | MEM[7:4] | MEM[3:0] | Jump PC to MEM[7:0] if Accumulator is Zero
|1001 | R02ACC | - | - | R0 => ACC
|1010 | CMPI | imm4 | - | Compare Immediate with Accumulator, sets flags (ZERO) without changing Accumulator
|1011 | CALL | MEM[7:4] | MEM[3:0] | Jump PC to MEM[7:0] and save following PC to RET_PC
|1100 | RET | - | - | Return from Subroutine (set PC) to address in RET_PC
|1101 | LDMAR | imm4 | - | MAR[7:4] <= Immediate, MAR[3:0] <= ACC, used for indirect addressing
|1110 | STA_MAR | - | - | Store ACC at MEM[MAR] (indirect addressing)
|1111 | LDA_MAR | - | - | Load from MEM[MAR] to ACC (indirect addressing)

## Memory Map
| Address Range | Hex      | Description |
|--------------|----------|-------------|
| 0–127        | 0x00–0x7F | Program ROM |
| 128–143      | 0x80–0x8F | Data RAM (16 words) |
| 192–255      | 0xC0–0xFF | Memory-mapped I/O |
>**Note:** RAM is currently limited to 16 bytes, but the architecture allows for easy expansion by upgrading the RAM module.

## Addressing Modes
The ISA supports the following addressing modes:

- **Immediate**: Operand is encoded directly in the instruction (imm4).
- **Direct**: Instruction specifies an absolute 8-bit memory address.
- **Indirect**: Memory address is taken from the MAR register.

Immediate values are read from program memory during instruction execution and are not memory addresses.


## Control Flow Semantics
- The Program Counter (PC) normally increments sequentially after instruction fetch.
- Jump instructions (JMP, JZ, CALL) explicitly modify the PC.
- JZ performs a conditional jump based on the Zero flag.
- CALL stores the return address in RET_PC before jumping.
- RET restores execution by loading PC from RET_PC.