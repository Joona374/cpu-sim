# Microarchitecture & Control Signals
This document describes the internal implementation of the CPU. The microcode implementation is not programmer-visible and does not form part of the ISA. 

## Overview
The CPU uses a microcode-based control unit to orchestrate operations. Each instruction is broken down into a series of microsteps, each of which activates specific control signals to manipulate data within the CPU datapath. It follows a traditional fetch-decode-execute cycle. Here is a high-level overview of the process:

1. New instruction is loaded from ROM into the Instruction Register (IR) on the first microstep (*Fetch*).
2. In step two (*Dispatch*) the IR is then loaded into the Control Unit, where its value is used to index into the Dispatch ROM to determine the starting microstep for that instruction. The address of the next microstep is loaded into the microprogram counter (µPC).
3. CPU starts executing the microsteps for the instruction, activating control signals as defined in the Microcode ROM. Once it reaches the final microstep for the instruction, it loops back to the Fetch step to process the next instruction.


## Control Signals

### Single-Bit Control Signals
| Label | Description |
| :--- | :--- |
| `TEMP_LO_load` | Enable loading low byte to LO_TEMP register |
| `TEMP_HI_load` | Enable loading high byte to HI_TEMP register |
| `ALUTMP_load`  | Enable loading to ALU temporary register |
| `RD_en`        | Enable read operation from memory |
| `WR_en`        | Enable write operation to memory |
| `PC_L_en`      | Enable loading low byte from LO_TEMP to Program Counter |
| `PC_H_en`      | Enable loading high byte from HI_TEMP to Program Counter |
| `IR_load`      | Instruction Register load enable |
| `ACC_load`     | Accumulator (ACC) load enable |
| `MAR_L`        | Memory Address Register low nibble load enable |
| `MAR_H`        | Memory Address Register high nibble load enable |
| `PC_src`       | Enable signal to write nibbles from temp register to PC |
| `PC_inc`       | Increments PC by one on clock edge |
| `FLAGS_en`     | Loads status signals from ALU to FLAGS register |
| `RET_PC_en`    | Load value from PC to RET_PC (for CALL operations) |

### Multi-Bit Control Signals
| Signal | Bits | Function |
| :--- | :--- | :--- |
| **ADDRBUS_sel** | `0 / 1` | `0` = PC (Program Counter), `1` = MAR (Memory Address Register) |
| **ALU_op[1:0]** | `00 / 01` | `00` = ADD, `01` = SUB |
| **BUS_sel[2:0]** | `000` | **DBUS_in**: External Data Bus |
| | `001` | **R0**: General Purpose Register 0 |
| | `010` | **ACC**: Accumulator |
| | `011` | **ALU**: ALU Result Output |
| | `100` | **TEMP_lo**: Low Temporary Register |
| | `101` | **TEMP_hi**: High Temporary Register |
| | `110` | **RET_LO**: Return Address Low |
| | `111` | **RET_HI**: Return Address High |
| **µPC_sel[1:0]** | `00` | **Increment**: Move to next microstep (+1) |
| | `01` | **Dispatch**: Jump to address in DispatchROM[IR] |
| | `10` | **Fetch**: Reset to `00000000` |
| | `11` | **JZ**: If Not Zero, increment by 4 |

## Instruction Microsteps 


| µAddr | Instruction | Step | Active Signals |
| :--- | :--- | :---: | :--- |
| `00000` | **Fetch** | 1 | `IR_load`, `RD_en` |
| `00001` | **Dispatch** | 2 | `µPC_sel0`, `PC_inc` |
| `00010` | **NOP** | 1 | `µPC_sel1` |
| `00011` | **LDI** | 1 | `RD_en`, `ACC_load`, `µPC_sel1`, `PC_inc` |
| `00101` | **ACC2R0** | 1 | `R0_en`, `BUS_sel1`, `µPC_sel1` |
| `00111` | **ADDI** | 1 | `ALUTMP_load`, `RD_en`, `FLAGS_en` |
| `01000` | | 2 | `ACC_load`, `BUS_selo0`, `BUS_selo1`, `µPC_sel1`, `PC_inc` |
| `01001` | **STA** | 1 | `RD_en`, `TEMP_HI_load`, `PC_inc` |
| `01010` | | 2 | `RD_en`, `TEMP_LO_load`, `PC_inc` |
| `01011` | | 3 | `MAR_HI_load`, `BUS_sel2`, `BUS_sel0` |
| `01100` | | 4 | `MAR_LO_load`, `BUS_sel2` |
| `01101` | | 5 | `ADDRBUS_sel`, `WR_en`, `BUS_sel 010`, `µPC_sel1` |
| `01110` | **LDA** | 1 | `RD_en`, `TEMP_HI_load`, `PC_inc` |
| `01111` | | 2 | `RD_en`, `TEMP_LO_load`, `PC_inc` |
| `10000` | | 3 | `MAR_HI_load`, `BUS_sel2`, `BUS_sel0` |
| `10001` | | 4 | `MAR_LO_load`, `BUS_sel2` |
| `10010` | | 5 | `ADDRBUS_sel`, `RD_en`, `ACC_load`, `µPC_sel1` |
| `10011` | **SUBI** | 1 | `ALU_op 01`, `ALUTMP_load`, `RD_en`, `FLAGS_en` |
| `10100` | | 2 | `ACC_load`, `BUS_sel 011`, `PC_inc`, `µPC_sel1` |
| `10101` | **JMP** | 1 | `RD_en`, `TEMP_HI_load`, `PC_inc` |
| `10110` | | 2 | `RD_en`, `TEMP_LO_load`, `PC_inc` |
| `10111` | | 3 | `PC_H_en`, `BUS_sel2`, `BUS_sel0`, `PC_src` |
| `11000` | | 4 | `PC_L_en`, `BUS_sel2`, `PC_src`, `µPC_sel1` |
| `11001` | **JZ** | 1 | `µPC_sel0`, `µPC_sel1`, `RD_en`, `TEMP_HI_load`, `PC_inc` |
| `11010` | | 2 | `RD_en`, `TEMP_LO_load` |
| `11011` | | 3 | `PC_H_en`, `BUS_sel2`, `BUS_sel0`, `PC_src` |
| `11100` | | 4 | `PC_L_en`, `BUS_sel2`, `PC_src`, `µPC_sel1` |
| `11101` | | 5 | `µPC_sel1`, `PC_inc` |
| `11110` | **CMPI** | 1 | `ALU_op 01`, `ALUTMP_load`, `RD_en`, `FLAGS_en`, `PC_inc`, `µPC_sel1` |
| `100000` | **CALL** | 1 | `RD_en`, `TEMP_HI_load`, `PC_inc` |
| `100001` | | 2 | `RD_en`, `TEMP_LO_load`, `PC_inc` |
| `100010` | | 3 | `RET_PC_en` |
| `100011` | | 4 | `PC_H_en`, `BUS_sel2`, `BUS_sel0`, `PC_src` |
| `100100` | | 5 | `PC_L_en`, `BUS_sel2`, `PC_src`, `µPC_sel1` |
| `100101` | **RET** | 1 | `BUS_sel0`, `BUS_sel1`, `BUS_sel2`, `PC_H_EN`, `PC_SRC` |
| `100110` | | 2 | `BUS_sel1`, `BUS_sel2`, `PC_L_EN`, `PC_SRC`, `µPC_sel1` |
| `100111` | **LDMAR** | 1 | `PC_inc`, `RD_en`, `MAR_HI_load` |
| `101000` | | 2 | `MAR_LO_load`, `BUS_sel1`, `µPC_sel1` |
| `101001` | **STA_MAR** | 1 | `ADDRBUS_sel`, `WR_en`, `BUS_sel 010`, `µPC_sel1` |
| `101010` | **LDA_MAR** | 1 | `ADDRBUS_sel`, `RD_en`, `ACC_load`, `µPC_sel1` |
| `101011` | **R02ACC** | 1 | `BUS_sel1`, `ACC_load`, `µPC_sel1` |