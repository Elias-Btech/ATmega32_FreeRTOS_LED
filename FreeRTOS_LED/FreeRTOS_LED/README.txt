================================================================
  FreeRTOS LED Priority Control - ATmega32
  Assignment: Embedded Systems with FreeRTOS
================================================================

PROJECT FILES:
--------------
main.c              -> Main application code (tasks + hardware init)
FreeRTOSConfig.h    -> FreeRTOS configuration for ATmega32

FreeRTOS/           -> FreeRTOS kernel source files
  tasks.c
  list.c
  queue.c
  include/          -> All FreeRTOS header files (.h)
  portable/
    GCC/
      ATMega323/
        port.c      -> AVR-specific context switching
        portmacro.h
    MemMang/
        heap_1.c    -> Simple memory allocator

================================================================
STEP 1: DOWNLOAD FREERTOS
================================================================
1. Go to: https://www.freertos.org/a00104.html
2. Download the zip and extract it
3. Copy files as described below into the FreeRTOS/ folder

FILES TO COPY FROM FREERTOS DOWNLOAD:
  FreeRTOS/Source/tasks.c          -> FreeRTOS/tasks.c
  FreeRTOS/Source/list.c           -> FreeRTOS/list.c
  FreeRTOS/Source/queue.c          -> FreeRTOS/queue.c
  FreeRTOS/Source/include/*        -> FreeRTOS/include/
  FreeRTOS/Source/portable/GCC/ATMega323/port.c      -> FreeRTOS/portable/GCC/ATMega323/
  FreeRTOS/Source/portable/GCC/ATMega323/portmacro.h -> FreeRTOS/portable/GCC/ATMega323/
  FreeRTOS/Source/portable/MemMang/heap_1.c          -> FreeRTOS/portable/MemMang/

================================================================
STEP 2: MICROCHIP STUDIO SETUP
================================================================
1. Open Microchip Studio 7
2. File -> New -> Project -> GCC C Executable Project
3. Name: FreeRTOS_LED | Device: ATmega32
4. Copy all files from this folder into the project folder

ADD FILES TO PROJECT (Solution Explorer):
  Right-click project -> Add -> Existing Item
  Add: tasks.c, list.c, queue.c, port.c, heap_1.c

ADD INCLUDE PATHS:
  Project -> Properties -> Toolchain -> AVR/GNU C Compiler -> Directories
  Add: ../FreeRTOS/include
  Add: ../FreeRTOS/portable/GCC/ATMega323

ADD SYMBOL:
  Project -> Properties -> Toolchain -> AVR/GNU C Compiler -> Symbols
  Add: F_CPU=8000000UL

5. Press F7 to Build
6. HEX file will be at: Debug/FreeRTOS_LED.hex

================================================================
STEP 3: PROTEUS SIMULATION
================================================================
Components needed:
  - ATmega32 (ATMEGA32)
  - 3x LEDs (Green, Yellow, Red)
  - 3x 330 Ohm Resistors
  - 8MHz Crystal
  - 2x 22pF Capacitors

Connections:
  PA0 -> 330R -> Green LED  -> GND
  PA1 -> 330R -> Yellow LED -> GND
  PA2 -> 330R -> Red LED    -> GND
  VCC (Pin 10, 30) -> +5V
  GND (Pin 11, 31) -> GND
  RESET (Pin 9)    -> 10k pull-up to VCC
  XTAL1/XTAL2      -> 8MHz Crystal with 22pF caps to GND

Load HEX:
  Double-click ATmega32 in Proteus
  Program File -> browse to Debug/FreeRTOS_LED.hex
  Clock Frequency -> 8MHz
  Press Play

================================================================
EXPECTED BEHAVIOR
================================================================
  Green LED  -> Blinks every 500ms  (fastest)
  Yellow LED -> Blinks every 1000ms (medium)
  Red LED    -> Blinks every 2000ms (slowest)

  All 3 LEDs blink simultaneously and independently.
  This demonstrates FreeRTOS multitasking.

================================================================
HARDWARE PIN SUMMARY
================================================================
  LED     | ATmega32 Pin | Port | Priority | Period
  --------|--------------|------|----------|--------
  Green   | Pin 40       | PA0  | 3 High   | 500ms
  Yellow  | Pin 39       | PA1  | 2 Medium | 1000ms
  Red     | Pin 38       | PA2  | 1 Low    | 2000ms

================================================================
TROUBLESHOOTING
================================================================
  Error: FreeRTOS.h not found
    -> Add ../FreeRTOS/include to compiler include paths

  Error: undefined reference to vTaskDelay
    -> Add tasks.c to the project in Solution Explorer

  Error: undefined reference to pvPortMalloc
    -> Add heap_1.c to the project

  Scheduler crashes at runtime
    -> Reduce configTOTAL_HEAP_SIZE to 1200 in FreeRTOSConfig.h

  LEDs don't blink in Proteus
    -> Rebuild project and reload the .hex file
    -> Make sure Proteus clock is set to 8MHz
