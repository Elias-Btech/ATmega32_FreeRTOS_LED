/*
 * FreeRTOS Kernel V10.4.3
 * tasks.c - Task management
 *
 * Handles: task creation, scheduler, delay, context switching
 * Trimmed for ATmega32 (minimal RAM usage)
 */

#include <stdlib.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"

/*-----------------------------------------------------------
 * MACROS AND DEFINITIONS
 *----------------------------------------------------------*/

#define tskIDLE_PRIORITY    ( ( UBaseType_t ) 0U )
#define tskBLOCKED_CHAR     ( 'B' )
#define tskREADY_CHAR       ( 'R' )
#define tskDELETED_CHAR     ( 'D' )
#define tskSUSPENDED_CHAR   ( 'S' )

/* Task states stored in TCB */
#define taskNOT_WAITING_NOTIFICATION    ( ( uint8_t ) 0 )

/*-----------------------------------------------------------
 * TASK CONTROL BLOCK (TCB)
 * This is the internal structure FreeRTOS uses to track each task
 *----------------------------------------------------------*/
typedef struct tskTaskControlBlock
{
    volatile StackType_t    *pxTopOfStack;  /* MUST be first member */

    ListItem_t              xStateListItem; /* Used for ready/blocked lists */
    ListItem_t              xEventListItem; /* Used for event lists */

    UBaseType_t             uxPriority;
    StackType_t             *pxStack;

    char                    pcTaskName[ configMAX_TASK_NAME_LEN ];

} tskTCB;

typedef tskTCB TCB_t;

/*-----------------------------------------------------------
 * GLOBAL VARIABLES
 *----------------------------------------------------------*/

/* Points to the TCB of the currently running task */
void * volatile pxCurrentTCB = NULL;

/* Ready list — one list per priority level */
static List_t pxReadyTasksLists[ configMAX_PRIORITIES ];

/* Delayed task lists */
static List_t xDelayedTaskList1;
static List_t xDelayedTaskList2;
static List_t * volatile pxDelayedTaskList;
static List_t * volatile pxOverflowDelayedTaskList;

/* Scheduler state */
static volatile UBaseType_t uxCurrentNumberOfTasks  = ( UBaseType_t ) 0U;
static volatile TickType_t  xTickCount              = ( TickType_t ) 0U;
static volatile UBaseType_t uxTopReadyPriority      = tskIDLE_PRIORITY;
static volatile BaseType_t  xSchedulerRunning       = pdFALSE;
static volatile UBaseType_t uxPendedTicks           = ( UBaseType_t ) 0U;
static volatile BaseType_t  xYieldPending           = pdFALSE;
static volatile BaseType_t  xNumOfOverflows         = ( BaseType_t ) 0;
static UBaseType_t          uxTaskNumber            = ( UBaseType_t ) 0U;
static TickType_t           xNextTaskUnblockTime    = portMAX_DELAY;

/*-----------------------------------------------------------
 * INTERNAL MACROS
 *----------------------------------------------------------*/

#define taskRECORD_READY_PRIORITY( uxPriority ) \
    if( ( uxPriority ) > uxTopReadyPriority )   \
    {                                            \
        uxTopReadyPriority = ( uxPriority );     \
    }

#define taskSELECT_HIGHEST_PRIORITY_TASK()                              \
{                                                                        \
    UBaseType_t uxTopPriority = uxTopReadyPriority;                     \
    while( listLIST_IS_EMPTY( &( pxReadyTasksLists[ uxTopPriority ] ) ) ) \
    {                                                                    \
        --uxTopPriority;                                                 \
    }                                                                    \
    listGET_OWNER_OF_NEXT_ENTRY( pxCurrentTCB,                          \
                                 &( pxReadyTasksLists[ uxTopPriority ] ) ); \
    uxTopReadyPriority = uxTopPriority;                                  \
}

#define taskADD_TO_READY_LIST( pxTCB )                                  \
    taskRECORD_READY_PRIORITY( ( pxTCB )->uxPriority );                 \
    vListInsertEnd( &( pxReadyTasksLists[ ( pxTCB )->uxPriority ] ),    \
                    &( ( pxTCB )->xStateListItem ) );

/*-----------------------------------------------------------
 * STATIC FUNCTION PROTOTYPES
 *----------------------------------------------------------*/
static void prvInitialiseNewTask( TaskFunction_t  pxTaskCode,
                                  const char *    pcName,
                                  uint16_t        usStackDepth,
                                  void *          pvParameters,
                                  UBaseType_t     uxPriority,
                                  TaskHandle_t *  pxCreatedTask,
                                  TCB_t *         pxNewTCB );

static void prvAddNewTaskToReadyList( TCB_t *pxNewTCB );
static void prvInitialiseTaskLists( void );
static void prvIdleTask( void *pvParameters );
static void prvAddCurrentTaskToDelayedList( TickType_t xTicksToWait );
static TCB_t *prvAllocateTCBAndStack( uint16_t usStackDepth );

/*-----------------------------------------------------------
 * TASK CREATION
 *----------------------------------------------------------*/

BaseType_t xTaskCreate( TaskFunction_t  pvTaskCode,
                        const char *    pcName,
                        uint16_t        usStackDepth,
                        void *          pvParameters,
                        UBaseType_t     uxPriority,
                        TaskHandle_t *  pxCreatedTask )
{
    TCB_t *pxNewTCB;
    BaseType_t xReturn;

    pxNewTCB = prvAllocateTCBAndStack( usStackDepth );

    if( pxNewTCB != NULL )
    {
        prvInitialiseNewTask( pvTaskCode, pcName, usStackDepth,
                              pvParameters, uxPriority,
                              pxCreatedTask, pxNewTCB );
        prvAddNewTaskToReadyList( pxNewTCB );
        xReturn = pdPASS;
    }
    else
    {
        xReturn = errQUEUE_EMPTY;
    }

    return xReturn;
}

/*-----------------------------------------------------------*/

static TCB_t *prvAllocateTCBAndStack( uint16_t usStackDepth )
{
    TCB_t *pxNewTCB;

    pxNewTCB = ( TCB_t * ) pvPortMalloc( sizeof( TCB_t ) );

    if( pxNewTCB != NULL )
    {
        pxNewTCB->pxStack = ( StackType_t * )
            pvPortMalloc( ( ( size_t ) usStackDepth ) * sizeof( StackType_t ) );

        if( pxNewTCB->pxStack == NULL )
        {
            pxNewTCB = NULL;
        }
    }

    return pxNewTCB;
}

/*-----------------------------------------------------------*/

static void prvInitialiseNewTask( TaskFunction_t  pxTaskCode,
                                  const char *    pcName,
                                  uint16_t        usStackDepth,
                                  void *          pvParameters,
                                  UBaseType_t     uxPriority,
                                  TaskHandle_t *  pxCreatedTask,
                                  TCB_t *         pxNewTCB )
{
    StackType_t *pxTopOfStack;
    UBaseType_t x;

    /* Calculate the top of stack address */
    pxTopOfStack = pxNewTCB->pxStack + ( usStackDepth - ( uint16_t ) 1 );

    /* Check alignment */
    pxTopOfStack = ( StackType_t * )
        ( ( ( uint16_t ) pxTopOfStack ) &
          ( ~( ( uint16_t ) portBYTE_ALIGNMENT ) ) );

    /* Store the task name */
    for( x = ( UBaseType_t ) 0; x < ( UBaseType_t ) configMAX_TASK_NAME_LEN; x++ )
    {
        pxNewTCB->pcTaskName[ x ] = pcName[ x ];
        if( pcName[ x ] == 0x00 )
        {
            break;
        }
    }
    pxNewTCB->pcTaskName[ configMAX_TASK_NAME_LEN - 1 ] = '\0';

    /* Clamp priority */
    if( uxPriority >= ( UBaseType_t ) configMAX_PRIORITIES )
    {
        uxPriority = ( UBaseType_t ) configMAX_PRIORITIES - 1U;
    }
    pxNewTCB->uxPriority = uxPriority;

    /* Initialise list items */
    vListInitialiseItem( &( pxNewTCB->xStateListItem ) );
    vListInitialiseItem( &( pxNewTCB->xEventListItem ) );

    listSET_LIST_ITEM_OWNER( &( pxNewTCB->xStateListItem ), pxNewTCB );
    listSET_LIST_ITEM_VALUE( &( pxNewTCB->xEventListItem ),
        ( TickType_t ) configMAX_PRIORITIES - ( TickType_t ) uxPriority );
    listSET_LIST_ITEM_OWNER( &( pxNewTCB->xEventListItem ), pxNewTCB );

    /* Initialise the stack */
    pxNewTCB->pxTopOfStack = pxPortInitialiseStack( pxTopOfStack,
                                                     pxTaskCode,
                                                     pvParameters );

    if( pxCreatedTask != NULL )
    {
        *pxCreatedTask = ( TaskHandle_t ) pxNewTCB;
    }
}

/*-----------------------------------------------------------*/

static void prvAddNewTaskToReadyList( TCB_t *pxNewTCB )
{
    portENTER_CRITICAL();
    {
        uxCurrentNumberOfTasks++;

        if( pxCurrentTCB == NULL )
        {
            /* First task — no context switching yet */
            pxCurrentTCB = pxNewTCB;

            if( uxCurrentNumberOfTasks == ( UBaseType_t ) 1 )
            {
                prvInitialiseTaskLists();
            }
        }
        else
        {
            if( xSchedulerRunning == pdFALSE )
            {
                if( pxCurrentTCB->uxPriority <= pxNewTCB->uxPriority )
                {
                    pxCurrentTCB = pxNewTCB;
                }
            }
        }

        uxTaskNumber++;
        taskADD_TO_READY_LIST( pxNewTCB );
    }
    portEXIT_CRITICAL();
}

/*-----------------------------------------------------------*/

static void prvInitialiseTaskLists( void )
{
    UBaseType_t uxPriority;

    for( uxPriority = ( UBaseType_t ) 0U;
         uxPriority < ( UBaseType_t ) configMAX_PRIORITIES;
         uxPriority++ )
    {
        vListInitialise( &( pxReadyTasksLists[ uxPriority ] ) );
    }

    vListInitialise( &xDelayedTaskList1 );
    vListInitialise( &xDelayedTaskList2 );

    pxDelayedTaskList         = &xDelayedTaskList1;
    pxOverflowDelayedTaskList = &xDelayedTaskList2;
}

/*-----------------------------------------------------------*/

void vTaskDelete( TaskHandle_t xTaskToDelete )
{
    /* Minimal implementation — not used in this project */
    ( void ) xTaskToDelete;
}

/*-----------------------------------------------------------
 * SCHEDULER
 *----------------------------------------------------------*/

void vTaskStartScheduler( void )
{
    BaseType_t xReturn;

    /* Create the idle task */
    xReturn = xTaskCreate( prvIdleTask,
                           "IDLE",
                           configMINIMAL_STACK_SIZE,
                           ( void * ) NULL,
                           tskIDLE_PRIORITY,
                           NULL );

    if( xReturn == pdPASS )
    {
        /* Disable interrupts — port layer will re-enable them */
        portDISABLE_INTERRUPTS();

        xSchedulerRunning = pdTRUE;
        xTickCount        = ( TickType_t ) 0U;

        /* Start the first task — this call never returns */
        xPortStartScheduler();
    }

    /* Should never get here */
    for( ;; );
}

/*-----------------------------------------------------------*/

void vTaskEndScheduler( void )
{
    portDISABLE_INTERRUPTS();
    xSchedulerRunning = pdFALSE;
    vPortEndScheduler();
}

/*-----------------------------------------------------------*/

void vTaskSuspendAll( void )
{
    ++uxPendedTicks;
}

/*-----------------------------------------------------------*/

BaseType_t xTaskResumeAll( void )
{
    BaseType_t xAlreadyYielded = pdFALSE;

    if( uxPendedTicks > ( UBaseType_t ) 0U )
    {
        --uxPendedTicks;
    }

    return xAlreadyYielded;
}

/*-----------------------------------------------------------
 * TASK DELAY
 *----------------------------------------------------------*/

void vTaskDelay( const TickType_t xTicksToDelay )
{
    if( xTicksToDelay > ( TickType_t ) 0U )
    {
        portENTER_CRITICAL();
        {
            prvAddCurrentTaskToDelayedList( xTicksToDelay );
        }
        portEXIT_CRITICAL();

        /* Force a context switch */
        portYIELD();
    }
}

/*-----------------------------------------------------------*/

void vTaskDelayUntil( TickType_t * const pxPreviousWakeTime,
                      const TickType_t xTimeIncrement )
{
    TickType_t xTimeToWake;
    BaseType_t xShouldDelay = pdFALSE;

    portENTER_CRITICAL();
    {
        xTimeToWake = *pxPreviousWakeTime + xTimeIncrement;

        if( xTickCount < *pxPreviousWakeTime )
        {
            if( ( xTimeToWake < *pxPreviousWakeTime ) &&
                ( xTimeToWake > xTickCount ) )
            {
                xShouldDelay = pdTRUE;
            }
        }
        else
        {
            if( ( xTimeToWake < *pxPreviousWakeTime ) ||
                ( xTimeToWake > xTickCount ) )
            {
                xShouldDelay = pdTRUE;
            }
        }

        *pxPreviousWakeTime = xTimeToWake;

        if( xShouldDelay != pdFALSE )
        {
            prvAddCurrentTaskToDelayedList( xTimeToWake - xTickCount );
        }
    }
    portEXIT_CRITICAL();

    if( xShouldDelay != pdFALSE )
    {
        portYIELD();
    }
}

/*-----------------------------------------------------------*/

static void prvAddCurrentTaskToDelayedList( TickType_t xTicksToWait )
{
    TickType_t xTimeToWake;

    /* Remove from ready list */
    if( uxListRemove( &( ( ( TCB_t * ) pxCurrentTCB )->xStateListItem ) )
        == ( UBaseType_t ) 0 )
    {
        uxTopReadyPriority = ( ( TCB_t * ) pxCurrentTCB )->uxPriority;
    }

    xTimeToWake = xTickCount + xTicksToWait;

    listSET_LIST_ITEM_VALUE( &( ( ( TCB_t * ) pxCurrentTCB )->xStateListItem ),
                             xTimeToWake );

    if( xTimeToWake < xTickCount )
    {
        /* Overflow — add to overflow list */
        vListInsert( pxOverflowDelayedTaskList,
                     &( ( ( TCB_t * ) pxCurrentTCB )->xStateListItem ) );
    }
    else
    {
        vListInsert( pxDelayedTaskList,
                     &( ( ( TCB_t * ) pxCurrentTCB )->xStateListItem ) );

        if( xTimeToWake < xNextTaskUnblockTime )
        {
            xNextTaskUnblockTime = xTimeToWake;
        }
    }
}

/*-----------------------------------------------------------
 * TICK INCREMENT (called from Timer ISR)
 *----------------------------------------------------------*/

BaseType_t xTaskIncrementTick( void )
{
    TCB_t *pxTCB;
    TickType_t xItemValue;
    BaseType_t xSwitchRequired = pdFALSE;

    if( uxPendedTicks == ( UBaseType_t ) 0U )
    {
        xTickCount++;

        if( xTickCount == ( TickType_t ) 0U )
        {
            /* Tick overflow — swap delayed lists */
            List_t *pxTemp = pxDelayedTaskList;
            pxDelayedTaskList         = pxOverflowDelayedTaskList;
            pxOverflowDelayedTaskList = pxTemp;
            xNumOfOverflows++;

            if( listLIST_IS_EMPTY( pxDelayedTaskList ) != pdFALSE )
            {
                xNextTaskUnblockTime = portMAX_DELAY;
            }
        }

        /* Unblock tasks whose delay has expired */
        if( xTickCount >= xNextTaskUnblockTime )
        {
            for( ;; )
            {
                if( listLIST_IS_EMPTY( pxDelayedTaskList ) != pdFALSE )
                {
                    xNextTaskUnblockTime = portMAX_DELAY;
                    break;
                }
                else
                {
                    pxTCB = ( TCB_t * )
                        listGET_OWNER_OF_HEAD_ENTRY( pxDelayedTaskList );
                    xItemValue = listGET_ITEM_VALUE_OF_HEAD_ENTRY(
                                     pxDelayedTaskList );

                    if( xTickCount < xItemValue )
                    {
                        xNextTaskUnblockTime = xItemValue;
                        break;
                    }

                    /* Remove from delayed list */
                    ( void ) uxListRemove( &( pxTCB->xStateListItem ) );

                    /* Add to ready list */
                    taskADD_TO_READY_LIST( pxTCB );

                    #if ( configUSE_PREEMPTION == 1 )
                    {
                        if( pxTCB->uxPriority >=
                            ( ( TCB_t * ) pxCurrentTCB )->uxPriority )
                        {
                            xSwitchRequired = pdTRUE;
                        }
                    }
                    #endif
                }
            }
        }

        #if ( ( configUSE_PREEMPTION == 1 ) && ( configIDLE_SHOULD_YIELD == 1 ) )
        {
            if( ( ( TCB_t * ) pxCurrentTCB )->uxPriority == tskIDLE_PRIORITY )
            {
                if( listCURRENT_LIST_LENGTH(
                        &( pxReadyTasksLists[ tskIDLE_PRIORITY ] ) ) > 1U )
                {
                    xSwitchRequired = pdTRUE;
                }
            }
        }
        #endif

        #if ( configUSE_PREEMPTION == 1 )
        {
            if( xYieldPending != pdFALSE )
            {
                xSwitchRequired = pdTRUE;
            }
        }
        #endif
    }
    else
    {
        ++uxPendedTicks;
    }

    return xSwitchRequired;
}

/*-----------------------------------------------------------
 * CONTEXT SWITCH
 *----------------------------------------------------------*/

void vTaskSwitchContext( void )
{
    taskSELECT_HIGHEST_PRIORITY_TASK();
}

/*-----------------------------------------------------------
 * TASK UTILITIES
 *----------------------------------------------------------*/

TickType_t xTaskGetTickCount( void )
{
    return xTickCount;
}

TickType_t xTaskGetTickCountFromISR( void )
{
    return xTickCount;
}

UBaseType_t uxTaskGetNumberOfTasks( void )
{
    return uxCurrentNumberOfTasks;
}

char *pcTaskGetName( TaskHandle_t xTaskToQuery )
{
    TCB_t *pxTCB;
    pxTCB = ( xTaskToQuery == NULL ) ?
            ( TCB_t * ) pxCurrentTCB :
            ( TCB_t * ) xTaskToQuery;
    return &( pxTCB->pcTaskName[ 0 ] );
}

UBaseType_t uxTaskPriorityGet( const TaskHandle_t xTask )
{
    TCB_t *pxTCB;
    pxTCB = ( xTask == NULL ) ?
            ( TCB_t * ) pxCurrentTCB :
            ( TCB_t * ) xTask;
    return pxTCB->uxPriority;
}

void vTaskPrioritySet( TaskHandle_t xTask, UBaseType_t uxNewPriority )
{
    ( void ) xTask;
    ( void ) uxNewPriority;
}

void vTaskSuspend( TaskHandle_t xTaskToSuspend )
{
    ( void ) xTaskToSuspend;
}

void vTaskResume( TaskHandle_t xTaskToResume )
{
    ( void ) xTaskToResume;
}

BaseType_t xTaskResumeFromISR( TaskHandle_t xTaskToResume )
{
    ( void ) xTaskToResume;
    return pdFALSE;
}

void vTaskIncrementTick( void )
{
    xTaskIncrementTick();
}

/*-----------------------------------------------------------
 * IDLE TASK
 *----------------------------------------------------------*/

static void prvIdleTask( void *pvParameters )
{
    ( void ) pvParameters;

    for( ;; )
    {
        /* Idle — do nothing, just yield */
        portNOP();
    }
}
