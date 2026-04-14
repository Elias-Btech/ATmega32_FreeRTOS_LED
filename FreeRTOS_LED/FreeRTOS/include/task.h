/*
 * FreeRTOS Kernel V10.4.3
 * task.h - Task control API
 */

#ifndef TASK_H
#define TASK_H

#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Task handle type */
typedef void * TaskHandle_t;

/* Task states */
typedef enum
{
    eRunning = 0,
    eReady,
    eBlocked,
    eSuspended,
    eDeleted,
    eInvalid
} eTaskState;

/*-----------------------------------------------------------
 * TASK CREATION / DELETION
 *----------------------------------------------------------*/

BaseType_t xTaskCreate( TaskFunction_t  pvTaskCode,
                        const char *    pcName,
                        uint16_t        usStackDepth,
                        void *          pvParameters,
                        UBaseType_t     uxPriority,
                        TaskHandle_t *  pvCreatedTask );

void vTaskDelete( TaskHandle_t xTaskToDelete );

/*-----------------------------------------------------------
 * TASK CONTROL
 *----------------------------------------------------------*/

void vTaskDelay( const TickType_t xTicksToDelay );

void vTaskDelayUntil( TickType_t * const pxPreviousWakeTime,
                      const TickType_t xTimeIncrement );

UBaseType_t uxTaskPriorityGet( const TaskHandle_t xTask );
void        vTaskPrioritySet( TaskHandle_t xTask, UBaseType_t uxNewPriority );
void        vTaskSuspend( TaskHandle_t xTaskToSuspend );
void        vTaskResume( TaskHandle_t xTaskToResume );
BaseType_t  xTaskResumeFromISR( TaskHandle_t xTaskToResume );

/*-----------------------------------------------------------
 * SCHEDULER CONTROL
 *----------------------------------------------------------*/

void        vTaskStartScheduler( void );
void        vTaskEndScheduler( void );
void        vTaskSuspendAll( void );
BaseType_t  xTaskResumeAll( void );

/*-----------------------------------------------------------
 * TASK UTILITIES
 *----------------------------------------------------------*/

TickType_t  xTaskGetTickCount( void );
TickType_t  xTaskGetTickCountFromISR( void );
UBaseType_t uxTaskGetNumberOfTasks( void );
char *      pcTaskGetName( TaskHandle_t xTaskToQuery );

/*-----------------------------------------------------------
 * SCHEDULER INTERNALS
 *----------------------------------------------------------*/

void        vTaskIncrementTick( void );
void        vTaskSwitchContext( void );
BaseType_t  xTaskIncrementTick( void );

/* TCB pointer - used by port.c for context save/restore */
extern void * volatile pxCurrentTCB;

#ifdef __cplusplus
}
#endif

#endif /* TASK_H */
