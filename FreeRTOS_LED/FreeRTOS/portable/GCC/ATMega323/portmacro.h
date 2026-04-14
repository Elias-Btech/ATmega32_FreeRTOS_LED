/*
 * FreeRTOS Kernel V10.4.3
 * portmacro.h - AVR ATmega323/ATmega32 port macros
 */

#ifndef PORTMACRO_H
#define PORTMACRO_H

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------
 * Port specific definitions.
 *----------------------------------------------------------*/

/* Type definitions */
#define portCHAR        char
#define portFLOAT       float
#define portDOUBLE      double
#define portLONG        long
#define portSHORT       int
#define portSTACK_TYPE  uint8_t
#define portBASE_TYPE   char

typedef portSTACK_TYPE  StackType_t;
typedef signed char     BaseType_t;
typedef unsigned char   UBaseType_t;

#if ( configUSE_16_BIT_TICKS == 1 )
    typedef uint16_t TickType_t;
    #define portMAX_DELAY ( TickType_t ) 0xffff
#else
    typedef uint32_t TickType_t;
    #define portMAX_DELAY ( TickType_t ) 0xffffffffUL
#endif

/* Critical section management */
#define portENTER_CRITICAL()        \
    asm volatile ( "in  __tmp_reg__, __SREG__" :: );  \
    asm volatile ( "cli" :: );                         \
    asm volatile ( "push __tmp_reg__" :: )

#define portEXIT_CRITICAL()         \
    asm volatile ( "pop  __tmp_reg__" :: );            \
    asm volatile ( "out  __SREG__, __tmp_reg__" :: )

#define portDISABLE_INTERRUPTS()    asm volatile ( "cli" :: )
#define portENABLE_INTERRUPTS()     asm volatile ( "sei" :: )

/* Architecture specifics */
#define portSTACK_GROWTH            ( -1 )
#define portBYTE_ALIGNMENT          1
#define portNOP()                   asm volatile ( "nop" )

/* Tick interrupt */
#define portTICK_PERIOD_MS  ( ( TickType_t ) 1000 / configTICK_RATE_HZ )

#ifndef portNOP
    #define portNOP()   asm volatile ( "nop" )
#endif
extern void vPortYield( void ) __attribute__ ( ( naked ) );
#define portYIELD()                 vPortYield()

extern void vPortYieldFromTick( void ) __attribute__ ( ( naked ) );

#define portSAVE_CONTEXT()                                  \
    asm volatile (  "push   r0                      \n\t"   \
                    "in     r0, __SREG__             \n\t"   \
                    "cli                             \n\t"   \
                    "push   r0                       \n\t"   \
                    "push   r1                       \n\t"   \
                    "clr    r1                       \n\t"   \
                    "push   r2                       \n\t"   \
                    "push   r3                       \n\t"   \
                    "push   r4                       \n\t"   \
                    "push   r5                       \n\t"   \
                    "push   r6                       \n\t"   \
                    "push   r7                       \n\t"   \
                    "push   r8                       \n\t"   \
                    "push   r9                       \n\t"   \
                    "push   r10                      \n\t"   \
                    "push   r11                      \n\t"   \
                    "push   r12                      \n\t"   \
                    "push   r13                      \n\t"   \
                    "push   r14                      \n\t"   \
                    "push   r15                      \n\t"   \
                    "push   r16                      \n\t"   \
                    "push   r17                      \n\t"   \
                    "push   r18                      \n\t"   \
                    "push   r19                      \n\t"   \
                    "push   r20                      \n\t"   \
                    "push   r21                      \n\t"   \
                    "push   r22                      \n\t"   \
                    "push   r23                      \n\t"   \
                    "push   r24                      \n\t"   \
                    "push   r25                      \n\t"   \
                    "push   r26                      \n\t"   \
                    "push   r27                      \n\t"   \
                    "push   r28                      \n\t"   \
                    "push   r29                      \n\t"   \
                    "push   r30                      \n\t"   \
                    "push   r31                      \n\t"   \
                    "lds    r26, pxCurrentTCB        \n\t"   \
                    "lds    r27, pxCurrentTCB + 1    \n\t"   \
                    "in     r0, 0x3d                 \n\t"   \
                    "st     x+, r0                   \n\t"   \
                    "in     r0, 0x3e                 \n\t"   \
                    "st     x+, r0                   \n\t"   \
                 );

#define portRESTORE_CONTEXT()                               \
    asm volatile (  "lds    r26, pxCurrentTCB        \n\t"  \
                    "lds    r27, pxCurrentTCB + 1    \n\t"  \
                    "ld     r28, x+                  \n\t"  \
                    "out    __SP_L__, r28             \n\t"  \
                    "ld     r29, x+                  \n\t"  \
                    "out    __SP_H__, r29             \n\t"  \
                    "pop    r31                      \n\t"   \
                    "pop    r30                      \n\t"   \
                    "pop    r29                      \n\t"   \
                    "pop    r28                      \n\t"   \
                    "pop    r27                      \n\t"   \
                    "pop    r26                      \n\t"   \
                    "pop    r25                      \n\t"   \
                    "pop    r24                      \n\t"   \
                    "pop    r23                      \n\t"   \
                    "pop    r22                      \n\t"   \
                    "pop    r21                      \n\t"   \
                    "pop    r20                      \n\t"   \
                    "pop    r19                      \n\t"   \
                    "pop    r18                      \n\t"   \
                    "pop    r17                      \n\t"   \
                    "pop    r16                      \n\t"   \
                    "pop    r15                      \n\t"   \
                    "pop    r14                      \n\t"   \
                    "pop    r13                      \n\t"   \
                    "pop    r12                      \n\t"   \
                    "pop    r11                      \n\t"   \
                    "pop    r10                      \n\t"   \
                    "pop    r9                       \n\t"   \
                    "pop    r8                       \n\t"   \
                    "pop    r7                       \n\t"   \
                    "pop    r6                       \n\t"   \
                    "pop    r5                       \n\t"   \
                    "pop    r4                       \n\t"   \
                    "pop    r3                       \n\t"   \
                    "pop    r2                       \n\t"   \
                    "pop    r1                       \n\t"   \
                    "pop    r0                       \n\t"   \
                    "out    __SREG__, r0             \n\t"   \
                    "pop    r0                       \n\t"   \
                 );

#ifdef __cplusplus
}
#endif

#endif /* PORTMACRO_H */
