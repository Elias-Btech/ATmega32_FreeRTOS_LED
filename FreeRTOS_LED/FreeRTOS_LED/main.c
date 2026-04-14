/*
 * ================================================================
 *  Project  : FreeRTOS LED Priority Control
 *  MCU      : ATmega32
 *  Clock    : 8 MHz
 *  Compiler : Microchip Studio (avr-gcc)
 *
 *  Hardware Connections:
 *  Green  LED -> PA0 -> Priority 3 (Highest) -> 500ms blink
 *  Yellow LED -> PA1 -> Priority 2 (Medium)  -> 1000ms blink
 *  Red    LED -> PA2 -> Priority 1 (Lowest)  -> 2000ms blink
 *
 *  Each LED: MCU Pin -> 330 Ohm Resistor -> LED Anode -> GND
 * ================================================================
 */

#include <avr/io.h>
#include "FreeRTOS.h"
#include "task.h"

/* ================================================================
 *  PIN DEFINITIONS
 * ================================================================ */
#define GREEN_PIN    PA0
#define YELLOW_PIN   PA1
#define RED_PIN      PA2

/* ================================================================
 *  LED CONTROL MACROS
 * ================================================================ */
#define GREEN_ON()    (PORTA |=  (1 << GREEN_PIN))
#define GREEN_OFF()   (PORTA &= ~(1 << GREEN_PIN))
#define YELLOW_ON()   (PORTA |=  (1 << YELLOW_PIN))
#define YELLOW_OFF()  (PORTA &= ~(1 << YELLOW_PIN))
#define RED_ON()      (PORTA |=  (1 << RED_PIN))
#define RED_OFF()     (PORTA &= ~(1 << RED_PIN))

/* ================================================================
 *  HARDWARE INITIALIZATION
 *  Set PA0, PA1, PA2 as OUTPUT and turn all LEDs OFF
 * ================================================================ */
void hardware_init(void)
{
    /* DDRA: set bits 0,1,2 as OUTPUT */
    DDRA |= (1 << GREEN_PIN) | (1 << YELLOW_PIN) | (1 << RED_PIN);

    /* All LEDs OFF at startup */
    PORTA &= ~((1 << GREEN_PIN) | (1 << YELLOW_PIN) | (1 << RED_PIN));
}

/* ================================================================
 *  TASK 1: GREEN LED - HIGHEST PRIORITY (3)
 *  Blinks every 500ms
 *  This task preempts Yellow and Red whenever it is ready
 * ================================================================ */
void vGreenLEDTask(void *pvParameters)
{
    (void) pvParameters;

    for (;;)
    {
        GREEN_ON();
        vTaskDelay(pdMS_TO_TICKS(500));   /* Block for 500ms */

        GREEN_OFF();
        vTaskDelay(pdMS_TO_TICKS(500));   /* Block for 500ms */
    }
}

/* ================================================================
 *  TASK 2: YELLOW LED - MEDIUM PRIORITY (2)
 *  Blinks every 1000ms
 *  Runs when Green is blocked, preempts Red
 * ================================================================ */
void vYellowLEDTask(void *pvParameters)
{
    (void) pvParameters;

    for (;;)
    {
        YELLOW_ON();
        vTaskDelay(pdMS_TO_TICKS(1000));  /* Block for 1000ms */

        YELLOW_OFF();
        vTaskDelay(pdMS_TO_TICKS(1000));  /* Block for 1000ms */
    }
}

/* ================================================================
 *  TASK 3: RED LED - LOWEST PRIORITY (1)
 *  Blinks every 2000ms
 *  Only runs when both Green and Yellow are blocked
 * ================================================================ */
void vRedLEDTask(void *pvParameters)
{
    (void) pvParameters;

    for (;;)
    {
        RED_ON();
        vTaskDelay(pdMS_TO_TICKS(2000));  /* Block for 2000ms */

        RED_OFF();
        vTaskDelay(pdMS_TO_TICKS(2000));  /* Block for 2000ms */
    }
}

/* ================================================================
 *  MAIN FUNCTION
 * ================================================================ */
int main(void)
{
    /* Initialize hardware pins */
    hardware_init();

    /* Create Green LED task - Priority 3 (Highest) */
    xTaskCreate(vGreenLEDTask,  "GREEN",  100, NULL, 3, NULL);

    /* Create Yellow LED task - Priority 2 (Medium) */
    xTaskCreate(vYellowLEDTask, "YELLOW", 100, NULL, 2, NULL);

    /* Create Red LED task - Priority 1 (Lowest) */
    xTaskCreate(vRedLEDTask,    "RED",    100, NULL, 1, NULL);

    /* Start FreeRTOS scheduler - never returns */
    vTaskStartScheduler();

    /* Should never reach here */
    for (;;);

    return 0;
}
