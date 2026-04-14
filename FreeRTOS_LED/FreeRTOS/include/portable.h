/*
 * FreeRTOS Kernel V10.4.3
 * portable.h - Portable layer API
 */

#ifndef PORTABLE_H
#define PORTABLE_H

/* Include the portmacro.h file for the port being used */
#include "portmacro.h"

/* Setup the stack of a new task so it is ready to be placed under
 * the scheduler control. */
StackType_t *pxPortInitialiseStack( StackType_t *pxTopOfStack,
                                    TaskFunction_t pxCode,
                                    void *pvParameters );

/* Setup the hardware ready for the scheduler to take control. */
BaseType_t xPortStartScheduler( void );

/* Undo any hardware setup that was performed by xPortStartScheduler(). */
void vPortEndScheduler( void );

#endif /* PORTABLE_H */
