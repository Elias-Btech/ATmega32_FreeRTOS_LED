/*
 * FreeRTOS Kernel V10.4.3
 * projdefs.h - General definitions
 */

#ifndef PROJDEFS_H
#define PROJDEFS_H

typedef void (*TaskFunction_t)( void * );

#define pdFALSE         ( ( BaseType_t ) 0 )
#define pdTRUE          ( ( BaseType_t ) 1 )
#define pdPASS          ( pdTRUE )
#define pdFAIL          ( pdFALSE )
#define errQUEUE_EMPTY  ( ( BaseType_t ) 0 )
#define errQUEUE_FULL   ( ( BaseType_t ) 0 )

#define pdMS_TO_TICKS( xTimeInMs ) \
    ( ( TickType_t ) ( ( ( TickType_t ) ( xTimeInMs ) * \
      ( TickType_t ) configTICK_RATE_HZ ) / ( TickType_t ) 1000U ) )

#define pdTICKS_TO_MS( xTimeInTicks ) \
    ( ( uint32_t ) ( ( uint32_t ) ( xTimeInTicks ) * 1000U ) / \
      ( uint32_t ) configTICK_RATE_HZ )

/* portNOP is defined in portmacro.h — do not redefine here */

#endif /* PROJDEFS_H */
