# Control LEDs on the LED board using the I/O Controller device.

## Overview
This program lights up one set of LEDs at a time on the LED board. You can control which LEDs are lit, using the UP, DOWN, LEFT, and RIGHT buttons on the I/O Controller device. The program continuously checks the button states and updates the LED position accordingly. 
The program is quite long, so executing a single cycle takes up to 15 seconds.
There is also a chance the user inputs the value between controller reading and reseting, which causes the input to be ignored.

## Machine code
```
0x1 0xF 0x4 0xC 0x0 0x5 0xD 0x0 0x2 0xB 0x4 0x4 0x9 0xA 0x0 0x8 0x2 0xF 0xA 0x4 0x8 0x5 0x5 0xA 0x2 0x8 0x4 0xA 0xA 0x1 0x8 0x6 0x0 0xA 0x8 0x8 0x6 0xB 0x4 0x8 0x0 0xB 0x3 0x2 0xB 0x3 0xB 0x7 0x0 0x5 0x5 0x8 0x1 0xD 0xC 0x1 0x0 0xE 0xC 0x5 0x8 0x0 0xD 0xC 0x1 0xF 0xE 0xC 0x1 0x0 0x4 0xD 0x0 0xC 0x5 0x8 0x0 0x4 0x8 0x1 0x6 0x1 0x7 0x2 0x6 0x5 0x8 0x0 0x4 0x8 0x1 0x3 0x1 0x7 0x2 0x6 0x5 0x8 0x0 0x4 0x8 0x1 0x6 0x2 0x7 0x2 0x6 0x5 0x8 0x0 0x4 0x8 0x1 0x3 0x2 0x7 0x2 0x6 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 
```

## Assembly code
```
POS EQU 0x80        ; Current LED position
PREV EQU 0x81       ; Previous LED position

LED0 EQU 0xC0       ; LED 0 output register
CTRL EQU 0xD0       ; Register where the controller writes inputs

ZEROS EQU 0x0       ; Constant 0x0
ONES EQU 0xF        ; Constant 0xF

START:              ; Initialization - Turn all LEDs on at start
    LDI ONES        
    STA LED0

LOOP:               ; Main loop
    LDA CTRL        ; Check the controller input
    ACC2R0          ; Move ACC temporarily value to R0
    CALL CLEAR_CTRL ; Clear the controller input
    
    R02ACC          ; Move R0 value back to ACC

    CMPI 0          ; Check if input is zero, write ZERO flag accordingly
    JZ SKIP         ; Jump to SKIP if zero

    CMPI 4          ; Check for other inputs
    JZ ADD1         ; Add 1 (X + 1) if input is 4 
    CMPI 2
    JZ SUB1         ; Subtract 1 (X - 1) if input is 2
    CMPI 1
    JZ SUB2         ; Subtract 2 (Y - 1) if input is 1
    CMPI 8
    JZ ADD2         ; Add 2 (Y + 1) if input is 8

RENDER:             ; Render the LED at new position
    STA POS         ; Store current position in POS
    CALL CLEAR_LED  ; Clear previous LED
    CALL RENDER_LED ; Render new LED

SKIP:               ; Skip rendering if no input
    JMP LOOP        ; Repeat the loop

CLEAR_LED:          ; Clear the LED at previous position
    LDA PREV        ; Load previous position to ACC (Treat it as a pointer)
    LDMAR 0xC       ; Set MAR to RAM_START + ACC
    LDI ZEROS       ; Load 0x0 to ACC
    STA_MAR         ; Store 0x0 at MEM[MAR] (to turn off the previous position LED)
    RET             ; Return from subroutine

RENDER_LED:         ; Render the LED at current position
    LDA POS         ; Load current position to ACC (Treat it as a pointer)
    LDMAR 0xC       ; Set MAR to RAM_START + ACC
    LDI ONES        ; Load 0xF to ACC
    STA_MAR         ; Store 0xF at MEM[MAR] (to turn on the current position LED)
    RET             ; Return from subroutine 

CLEAR_CTRL:         ; Clear the controller input register 
    LDI ZEROS       ; Load 0x0 to ACC
    STA CTRL        ; Store the 0x0 in ACC at CTRL
    RET             ; Return from subroutine

SUB1:               ; Subtract 1 from current position (treat as X - 1)
    LDA POS
    STA PREV
    SUBI 1
    JMP RENDER      ; Jump back to RENDER
ADD1:               ; Add 1 to current position (treat as X + 1)
    LDA POS         
    STA PREV
    ADDI 1
    JMP RENDER      ; Jump back to RENDER

SUB2:               ; Subtract 2 from current position (treat as Y - 1)
    LDA POS
    STA PREV
    SUBI 2
    JMP RENDER      ; Jump back to RENDER

ADD2:               ; Add 2 to current position (treat as Y + 1)
    LDA POS
    STA PREV
    ADDI 2
    JMP RENDER      ; Jump back to RENDER
```