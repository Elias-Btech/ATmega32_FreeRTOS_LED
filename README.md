# ATmega32 FreeRTOS LED Priority Control

An embedded system project using **ATmega32** microcontroller and **FreeRTOS** to control three LEDs based on task priority.

## Project Overview

| LED    | Pin  | Priority     | Blink Rate  |
|--------|------|--------------|-------------|
| Green  | PA0  | 3 (Highest)  | 500ms       |
| Yellow | PA1  | 2 (Medium)   | 1000ms      |
| Red    | PA2  | 1 (Lowest)   | 2000ms      |

## Folder Structure

```
ATmega32_FreeRTOS_LED/
├── FreeRTOS_LED/                   ← Microchip Studio project
│   ├── FreeRTOS_LED.atsln          ← Solution file
│   ├── main.c                      ← Application code
│   ├── FreeRTOSConfig.h            ← FreeRTOS configuration
│   └── FreeRTOS_LED/               ← Project folder
│       ├── FreeRTOS_LED.cproj      ← Project file
│       ├── FreeRTOS/               ← FreeRTOS kernel files
│       │   ├── tasks.c
│       │   ├── list.c
│       │   ├── queue.c
│       │   ├── include/            ← Header files
│       │   └── portable/
│       │       ├── GCC/ATMega323/  ← port.c, portmacro.h
│       │       └── MemMang/        ← heap_1.c
│       └── Debug/
│           └── FreeRTOS_LED.hex    ← Compiled HEX file
├── Proteus/
│   └── free RTOS.pdsprj            ← Proteus simulation
├── Report.pdf                      ← Project report
├── .gitignore
└── README.md
```

## Hardware Requirements

- ATmega32 microcontroller
- 3x LEDs (Green, Yellow, Red)
- 3x 330Ω resistors
- 8MHz crystal oscillator
- 2x 22pF capacitors
- 5V power supply

## Circuit Connections

```
PA0 (Pin 40) → 330Ω → Green LED  → GND
PA1 (Pin 39) → 330Ω → Yellow LED → GND
PA2 (Pin 38) → 330Ω → Red LED    → GND

XTAL1 (Pin 13) → 8MHz Crystal → XTAL2 (Pin 12)
Crystal pins   → 22pF caps    → GND
AVCC (Pin 30)  → VCC
RESET (Pin 9)  → VCC
```

## Software Requirements

- [Microchip Studio 7](https://www.microchip.com/mplab/microchip-studio)
- [Proteus 8](https://www.labcenter.com)
- [FreeRTOS Kernel](https://www.freertos.org)

## How to Build

1. Open Microchip Studio
2. Open `FreeRTOS_LED/FreeRTOS_LED.atsln`
3. Press `F7` to build
4. HEX file generated in `FreeRTOS_LED/FreeRTOS_LED/Debug/`

## How to Simulate

1. Open Proteus
2. Open `Proteus/free RTOS.pdsprj`
3. Double-click ATmega32 → load `FreeRTOS_LED.hex`
4. Set clock to `8MHz`
5. Press Play ▶

## Expected Behavior

All 3 LEDs blink simultaneously at different rates:
- Green blinks fastest — 500ms (highest priority)
- Yellow blinks medium — 1000ms
- Red blinks slowest — 2000ms (lowest priority)

## FreeRTOS Configuration

Key settings in `FreeRTOSConfig.h`:

```c
#define configCPU_CLOCK_HZ      8000000   // 8MHz crystal
#define configTICK_RATE_HZ      1000      // 1ms tick
#define configTOTAL_HEAP_SIZE   1500      // 2KB SRAM limit
#define configUSE_PREEMPTION    1         // Preemptive scheduling
#define configUSE_16_BIT_TICKS  1         // Required for AVR
```

## Author

**[Your Name]**
[Your University]
[Your Course]
[Date]
