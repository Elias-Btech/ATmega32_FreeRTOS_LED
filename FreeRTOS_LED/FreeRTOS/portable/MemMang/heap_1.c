/*
 * FreeRTOS Kernel V10.4.3
 * heap_1.c - Simplest memory allocator
 *
 * This allocator only allows memory to be allocated, never freed.
 * Suitable for ATmega32 where tasks are created once and never deleted.
 */

#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"

/* Ensure byte alignment */
#define portBYTE_ALIGNMENT_MASK  ( 0x0000 )

/* The heap is a static array */
static uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];

/* Index into the ucHeap array */
static size_t xNextFreeByte = ( size_t ) 0;

/*-----------------------------------------------------------*/

void *pvPortMalloc( size_t xWantedSize )
{
    void *pvReturn = NULL;

    /* Ensure byte alignment */
    #if portBYTE_ALIGNMENT != 1
        if( xWantedSize & portBYTE_ALIGNMENT_MASK )
        {
            xWantedSize += ( portBYTE_ALIGNMENT -
                           ( xWantedSize & portBYTE_ALIGNMENT_MASK ) );
        }
    #endif

    vTaskSuspendAll();
    {
        if( ( xNextFreeByte + xWantedSize ) <  configTOTAL_HEAP_SIZE )
        {
            pvReturn = &( ucHeap[ xNextFreeByte ] );
            xNextFreeByte += xWantedSize;
        }
    }
    xTaskResumeAll();

    return pvReturn;
}

/*-----------------------------------------------------------*/

void vPortFree( void *pv )
{
    /* heap_1 does not support freeing memory */
    ( void ) pv;
}

/*-----------------------------------------------------------*/

void vPortInitialiseBlocks( void )
{
    xNextFreeByte = ( size_t ) 0;
}

/*-----------------------------------------------------------*/

size_t xPortGetFreeHeapSize( void )
{
    return ( configTOTAL_HEAP_SIZE - xNextFreeByte );
}
