# Leds traveling accross the LED board.

## Overview
This program lights up one set of LEDs at a time on the LED board. On each cycle of the main loop, it turns off the previously lit LEDs and lights up the next set in sequence. The program continuously loops through the LED positions, creating a traveling light effect. Once it reaches the end of the LED board, it halts execution.

## Machine code
```
0x1 0xF 0x4 0xC 0x0 0x1 0x0 0x4 0x8 0x1 0x4 0x8 0x0 0xB 0x2 0x1 0xB 0x2 0xD 0xB 0x3 0x6 0x5 0x8 0x0 0xA 0xF 0x8 0x3 0xF 0x7 0x0 0xD 0x5 0x8 0x0 0x4 0x8 0x1 0x3 0x1 0x4 0x8 0x0 0xC 0x5 0x8 0x1 0xD 0xC 0x1 0x0 0xE 0xC 0x5 0x8 0x0 0xD 0xC 0x1 0xF 0xE 0xC 0x0 0x7 0x3 0xF 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 
```

## Assembly code
```
POS EQU 0x80                ; Current LED position RAM address
PREV EQU 0x81               ; Previous LED position RAM address
LED0 EQU 0xC0               ; First register to control the LED output (16 LEDs mapped to 0xC0 - 0xCF)

ZEROS EQU 0x0               ; Constant 0x0
ONES EQU 0xF                ; Constant 0xF
MAX_POS EQU 0xF             ; Maximum position (16 LEDs)

START:                      ; Initialization - Turn first LEDs on at start
    LDI ONES                ; Load 0xF to ACC
    STA LED0                ; Turn on first LED (position 0)
    LDI ZEROS               ; Load 0x0 to ACC
    STA PREV                ; Store 0x0 to PREV
    STA POS                 ; Store 0x0 to POS
    
LOOP:                       ; Main loop
    CALL INCREMENT_POS  
    CALL CLEAR_LED          ; Clear previous LED
    CALL RENDER_LED         ; Render new LED
    
    LDA POS                 ; Load current position
    CMPI MAX_POS            ; Compare position with 15 (number of LEDs) to set zero flag
    JZ HALT                 ; If position == 15, halt the program
    
    JMP LOOP                ; Repeat the loop

INCREMENT_POS:              ; Increment the current position and store previous position
    LDA POS
    STA PREV                ; Store current position to previous position
    ADDI 1                  ; Increment position
    STA POS                 ; Store back incremented position
    RET                     ; Return from subroutine

CLEAR_LED:                  ; Clear the LED at previous position
    LDA PREV                ; Load previous position to ACC (Treat it as a pointer)
    LDMAR 0xC               ; Set MAR to RAM_START + ACC
    LDI ZEROS               ; Load 0x0 to ACC
    STA_MAR                 ; Store 0x0 at MEM[MAR] (to turn off the previous position LED)
    RET                     ; Return from subroutine

RENDER_LED:                 ; Render the LED at current position
    LDA POS                 ; Load current position to ACC (Treat it as a pointer)
    LDMAR 0xC               ; Set MAR to RAM_START + ACC
    LDI ONES                ; Load 0xF to ACC
    STA_MAR                 ; Store 0xF at MEM[MAR] (to turn on the current position LED)
    RET                     ; Return from subroutine 

HALT:       
    NOP                     ; No operation (just to have a place to jump to)
    JMP HALT                ; Infinite loop to halt the program
```