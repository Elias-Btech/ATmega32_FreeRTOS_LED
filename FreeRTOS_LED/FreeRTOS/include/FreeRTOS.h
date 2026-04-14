/*
 * FreeRTOS Kernel V10.4.3
 * FreeRTOS.h - Main include file
 */

#ifndef FREERTOS_H
#define FREERTOS_H

#include <stddef.h>
#include <stdint.h>

/* Include the project-specific configuration first */
#include "FreeRTOSConfig.h"

/* Basic type definitions */
#include "projdefs.h"

/* Sanity check configUSE_16_BIT_TICKS */
#if ( configUSE_16_BIT_TICKS == 1 )
    typedef uint16_t TickType_t;
    #define portMAX_DELAY ( TickType_t ) 0xffff
#else
    typedef uint32_t TickType_t;
    #define portMAX_DELAY ( TickType_t ) 0xffffffffUL
#endif

typedef int8_t   BaseType_t;
typedef uint8_t  UBaseType_t;

/* Scheduler state */
#define taskSCHEDULER_SUSPENDED     ( ( BaseType_t ) 0 )
#define taskSCHEDULER_NOT_STARTED   ( ( BaseType_t ) 1 )
#define taskSCHEDULER_RUNNING       ( ( BaseType_t ) 2 )

/* NULL handle */
#define NULL_HANDLE  NULL

/* Include portable layer */
#include "portable.h"

/* Memory management */
void *pvPortMalloc( size_t xSize );
void  vPortFree( void *pv );
void  vPortInitialiseBlocks( void );
size_t xPortGetFreeHeapSize( void );
size_t xPortGetMinimumEverFreeHeapSize( void );

#endif /* FREERTOS_H */
