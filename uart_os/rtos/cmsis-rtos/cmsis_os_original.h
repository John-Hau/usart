/* ----------------------------------------------------------------------
 * $Date:        5. February 2013
 * $Revision:    V1.02
 *
 * Project:      CMSIS-RTOS API
 * Title:        cmsis_os.h template header file
 *
 * Version 0.02
 *    Initial Proposal Phase
 * Version 0.03
 *    osKernelStart added, optional feature: main started as thread
 *    osSemaphores have standard behavior
 *    osTimerCreate does not start the timer, added osTimerStart
 *    osThreadPass is renamed to osThreadYield
 * Version 1.01
 *    Support for C++ interface
 *     - const attribute removed from the osXxxxDef_t typedef's
 *     - const attribute added to the osXxxxDef macros
 *    Added: osTimerDelete, osMutexDelete, osSemaphoreDelete
 *    Added: osKernelInitialize
 * Version 1.02
 *    Control functions for short timeouts in microsecond resolution:
 *    Added: osKernelSysTick, osKernelSysTickFrequency, osKernelSysTickMicroSec
 *    Removed: osSignalGet 
 *----------------------------------------------------------------------------
 *
 * Copyright (c) 2013 ARM LIMITED
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  - Neither the name of ARM  nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *---------------------------------------------------------------------------*/

 
#ifndef _CMSIS_OS_H
#define _CMSIS_OS_H


#ifndef _CMSIS_OS_HONEYCOMB_H_
  #error "Never include cmsis_os_original.h directly. Please include cmsis_os.h instead."
#endif

/* #001 23.09.2015 WSC: Added ATTRIBUTES_<function name> to all functions.
 *
 * #002 23.09.2015 WSC: Skip typedef of IDs in case that there is a macro with the type
 * name. This is to redirect the ID type definitions as required for the underlying
 * RTOS. According to the CMSIS standard, this is allowed, because those typedefs are
 * tagged with "CAN BE CHANGED"
 *
 * #003 23.09.2015 WSC: Added name to struct os_thread_def.
 * The tag "CAN BE CHANGED" allows modifications of os_thread_def.
 *
 * #004 09.10.2015 WSC: Adapted osKernelSysTickFrequency
 *
 * #005 13.10.2015 WSC: Removed memory pointer in os_pool_def, because we
 * always allocate pool memory from the global heap.
 *
 * #006 16.10.2015 WSC: Added timer name to os_timer_def structure, to utilize
 * ThreadX's timer name.
 *
 * #007 18.10.2015 WSC: Added mutex name to os_mutex_def structure, to utilize
 * ThreadX's mutex name.
 *
 * #008 18.10.2015 WSC: Added semaphore name to os_semaphore_def structure, to utilize
 * ThreadX's semaphore name.
 *
 * #009 18.10.2015 WSC: Added pool name to os_pool_def structure, to utilize
 * ThreadX's pool name.
 *
 * #010 19.10.2015 WSC: Added message queue name to os_messageQ_def structure, to utilize
 * ThreadX's queue name.
 *
 * #011 19.10.2015 WSC: Removed pool pointer in os_messageQ_def, because we
 * always allocate pool memory from the global heap.
 *
 * #012 19.10.2015 WSC: Removed pool pointer in os_mailQ_def, because we
 * always allocate pool memory from the global heap.
 *
 * #013 19.10.2015 WSC: Added message queue name to os_mailQ_def structure, to utilize
 * ThreadX's queue name.
 *
 * #014 27.10.2015 WSC: Added API extension section.
 *
 * #015 27.10.2015 WSC: Define OS features only if not already defined. This lets
 * OS features to be controlled from the target RTOS header file.
 *
 * #016 27.10.2015 WSC: Added documentation whether a function can be called within and
 * ISR context.
 *
 * #017 27.10.2015 WSC: Added some declarations to the API extension to doxygen group
 * cmsis_os
 *
 * #018 27.10.2015 WSC: Added doxygen \brief tag to all function descriptions.
 *
 * #018 28.10.2015 WSC: Added implemetation details about osSignalWait(..).
 *
 * #019 19.12.2015 WSC: Added osPoolDestroy_suppl().
 *
 * #020 10.01.2016 WSC: Changed osKernelSystemId to be derived TARGET_RTOS_NAME TARGET_RTOS_NAME
 *
 * #021 14.01.2016 WSC: Integrated full documentation from CMSIS-RTOS Specification
 *
 * #022 28.06.2016 WSC: Skip thread functions in a non RTOS environment.
 *
 * #023 13.09.2016 WSC: Introduced supplementary function osTimerIsActive_suppl(..).
 *
 * #024 13.09.2016 WSC: Introduced osGetThreadState_suppl(..).
 *
 * #025 29.09.2017 WSC: Introduced osThreadSuspend(..).
 *
 * #026 07.10.2017 WSC: Introduced osThreadGetName_suppl(..).
 *
 */

/* #017 Start */
/** @addtogroup cmsis_os_kernel */
/**@{*/
/* #017 End */

/**
 * Version information of the CMSIS-RTOS API whereby major version is in bits
 * [31:16] and sub version in bits [15:0]. The value 0x10000 represents
 * version 1.00.
 * \note MUST REMAIN UNCHANGED: \b osCMSIS identifies the CMSIS-RTOS API
 *   version.
 */
#define osCMSIS           0x10002
 
/**
 * Identifies the underlying RTOS kernel and version number. The actual name of
 * that define depends on the RTOS Kernel used in the implementation. For
 * example, osCMSIS_FreeRTOS identifies the FreeRTOS kernel and the value
 * indicates the version number of that kernel whereby the major version is in
 * bits [31:16] and sub version in bits [15:0]. The value 0x10000 represents
 * version 1.00.
 * \note CAN BE CHANGED: \b osCMSIS_KERNEL identifies the underlying RTOS kernel
 *   and version number.
 */
#define osCMSIS_KERNEL    0x10000
 
/**
 * Defines a string that identifies the underlying RTOS Kernel and provides
 * version information. The length of that string is limited to 21 bytes. A
 * valid identification string is for example, "FreeRTOS V1.00".
 * \note MUST REMAIN UNCHANGED: \b osKernelSystemId shall be consistent in every
 *   CMSIS-RTOS.
 */
#define osKernelSystemId (TARGET_RTOS_NAME TARGET_RTOS_VERSION) /* #020 */


/**
 * \name Kernel Feature Macros
 * \note MUST REMAIN UNCHANGED: \b osFeature_xxx shall be consistent in every CMSIS-RTOS.
 */
/** @{*/

/** \def osFeature_MainThread
 *  \brief main thread      1=main can be thread, 0=not available
 */
#ifndef osFeature_MainThread             /* #015 */
  #define osFeature_MainThread   1
#endif

/** \def osFeature_SysTick
 *  \brief osKernelSysTick functions: 1=available, 0=not available
 */
/** @}*/
#ifndef osFeature_SysTick                /* #015 */
  #define osFeature_SysTick      1
#endif

/* #017 Start */
/**@} cmsis_os_kernel */
/* #017 End */

/**
 * \def osFeature_Wait
 * \ingroup cmsis_os_generic_wait
 * \brief A CMSIS-RTOS implementation may support the generic wait function \ref osWait.
 *     - When _osFeature_Wait_ is 1 a generic wait function \ref osWait is available.
 *     - When _osFeature_Wait_ is 0 no generic wait function \ref osWait is available.
 */
#ifndef osFeature_Wait                   /* #015 */
  #define osFeature_Wait         0
#endif

/**
 * \def osFeature_Signals
 * \ingroup cmsis_os_signal
 * \brief Maximum number of Signal Flags available per thread
 */
#ifndef osFeature_Signals                /* #015 */
  #define osFeature_Signals     31
#endif

/**
 * \def osFeature_Semaphore
 * \ingroup cmsis_os_semaphore
 * \brief  maximum count for \ref osSemaphoreCreate function
 */
#ifndef osFeature_Semaphore              /* #015 */
  #define osFeature_Semaphore   30
#endif

/**
 * \def osFeature_Pool
 * \ingroup cmsis_os_memory_pool
 * \brief Memory Pools:    1=available, 0=not available
 */
#ifndef osFeature_Pool                   /* #015 */
  #define osFeature_Pool         1
#endif

/**
 * \def osFeature_MessageQ
 * \ingroup cmsis_os_message_queue
 * \brief A CMSIS-RTOS implementation may support message queues.
 *     - When _osFeature_MessageQ_ is 1 message queues are supported.
 *     - When _osFeature_MessageQ_ is 0 no message queues are supported.
 */
#ifndef osFeature_MessageQ               /* #015 */
  #define osFeature_MessageQ     1
#endif

/**
 * \def osFeature_MailQ
 * \ingroup cmsis_os_memory_mail_queues
 * \brief Mail Queues:     1=available, 0=not available
 */
#ifndef osFeature_MailQ                  /* #015 */
  #define osFeature_MailQ        1
#endif



#include <stdint.h>
#include <stddef.h>
 
#ifdef  __cplusplus
extern "C"
{
#endif
 
/* ==== Enumeration, structures, defines ==== */
/** @addtogroup cmsis_os_generic_types */
/** @{*/

/**
 * \brief Priority used for thread control.
 * \note MUST REMAIN UNCHANGED: \b osPriority shall be consistent in every CMSIS-RTOS.
 */
typedef enum osPriority_ {
#if TARGET_RTOS == RTOS_ThreadX
  osPriorityOff           = -4,          ///< #025 Never assign this to tha thread. This is a Thread_X workaround for suspending a thread.
#endif
  osPriorityIdle          = -3,          ///< priority: idle (lowest)
  osPriorityLow           = -2,          ///< priority: low
  osPriorityBelowNormal   = -1,          ///< priority: below normal
  osPriorityNormal        =  0,          ///< priority: normal (default)
  osPriorityAboveNormal   = +1,          ///< priority: above normal
  osPriorityHigh          = +2,          ///< priority: high
  osPriorityRealtime      = +3,          ///< priority: realtime (highest)
  osPriorityError         =  0x84        ///< system cannot determine priority or thread has illegal priority
} osPriority;
 
/**
 * \brief Timeout value.
 * \note MUST REMAIN UNCHANGED: \b osWaitForever shall be consistent in every CMSIS-RTOS.
 */
#define osWaitForever     0xFFFFFFFF     ///< wait forever timeout value
/** @} */


/**
 * \ingroup cmsis_os_status_codes
 * \brief Status code values returned by CMSIS-RTOS functions.
 * \note MUST REMAIN UNCHANGED: \b osStatus shall be consistent in every CMSIS-RTOS.
 */
typedef enum osStatus_ {
  osOK                    =     0,       ///< function completed; no error or event occurred.
  osEventSignal           =  0x08,       ///< function completed; signal event occurred.
  osEventMessage          =  0x10,       ///< function completed; message event occurred.
  osEventMail             =  0x20,       ///< function completed; mail event occurred.
  osEventTimeout          =  0x40,       ///< function completed; timeout occurred.
  osErrorParameter        =  0x80,       ///< parameter error: a mandatory parameter was missing or specified an incorrect object.
  osErrorResource         =  0x81,       ///< resource not available: a specified resource was not available.
  osErrorTimeoutResource  =  0xC1,       ///< resource not available within given time: a specified resource was not available within the timeout period.
  osErrorISR              =  0x82,       ///< not allowed in ISR context: the function cannot be called from interrupt service routines.
  osErrorISRRecursive     =  0x83,       ///< function called multiple times from ISR with same object.
  osErrorPriority         =  0x84,       ///< system cannot determine priority or thread has illegal priority.
  osErrorNoMemory         =  0x85,       ///< system is out of memory: it was impossible to allocate or reserve memory for the operation.
  osErrorValue            =  0x86,       ///< value of a parameter is out of range.
  osErrorOS               =  0xFF,       ///< unspecified RTOS error: run-time error but no other error message fits.
  os_status_reserved      =  0x7FFFFFFF  ///< prevent from enum down-size compiler optimization.
} osStatus;
 

/* >>> the following data type definitions may shall adapted towards a specific RTOS */
/** @addtogroup cmsis_os_generic_types */
/** @{*/

/** \name CMSIS-RTOS-Objects Identifiers */
/** @{*/

#ifndef osThreadId /* #002 */
/**
 * \brief Thread ID identifies the thread (pointer to a thread control block).
 * \note CAN BE CHANGED: \b os_thread_cb is implementation specific in every CMSIS-RTOS.
 */
typedef struct os_thread_cb *osThreadId;
#else
/**
 * \def osThreadId
 * \brief Thread ID identifies the thread (pointer to a thread control block).
 * \note CAN BE CHANGED: \b os_thread_cb is implementation specific in every CMSIS-RTOS.
 */
#endif

#ifndef osTimerId /* #002 */
/**
 * \brief Timer ID identifies the timer (pointer to a timer control block).
 * \note CAN BE CHANGED: \b os_timer_cb is implementation specific in every CMSIS-RTOS.
 */
typedef struct os_timer_cb *osTimerId;
#else
/**
 * \def osTimerId
 * \brief Timer ID identifies the timer (pointer to a timer control block).
 * \note CAN BE CHANGED: \b os_timer_cb is implementation specific in every CMSIS-RTOS.
 */
#endif


#ifndef osMutexId /* #002 */
/**
 * \brief Mutex ID identifies the mutex (pointer to a mutex control block).
 * \note CAN BE CHANGED: \b os_mutex_cb is implementation specific in every CMSIS-RTOS.
 */
typedef struct os_mutex_cb *osMutexId;
#else
/**
 * \def osMutexId
 * \brief Mutex ID identifies the mutex (pointer to a mutex control block).
 * \note CAN BE CHANGED: \b os_mutex_cb is implementation specific in every CMSIS-RTOS.
 */
#endif
 
#ifndef osSemaphoreId /* #002 */
/**
 * \brief Semaphore ID identifies the semaphore (pointer to a semaphore control block).
 * \note CAN BE CHANGED: \b os_semaphore_cb is implementation specific in every CMSIS-RTOS.
 */
typedef struct os_semaphore_cb *osSemaphoreId;
#else
/**
 * \def osSemaphoreId
 * \brief Semaphore ID identifies the semaphore (pointer to a semaphore control block).
 * \note CAN BE CHANGED: \b os_semaphore_cb is implementation specific in every CMSIS-RTOS.
 */
#endif
 
#ifndef osPoolId /* #002 */
/**
 * \brief Pool ID identifies the memory pool (pointer to a memory pool control block).
 * \note CAN BE CHANGED: \b os_pool_cb is implementation specific in every CMSIS-RTOS.
 */
typedef struct os_pool_cb *osPoolId;
#else
/**
 * \def osPoolId
 * \brief Pool ID identifies the memory pool (pointer to a memory pool control block).
 * \note CAN BE CHANGED: \b os_pool_cb is implementation specific in every CMSIS-RTOS.
 */
#endif
 
#ifndef osMessageQId /* #002 */
/**
 * \brief Message ID identifies the message queue (pointer to a message queue control block).
 * \note CAN BE CHANGED: \b os_messageQ_cb is implementation specific in every CMSIS-RTOS.
 */
typedef struct os_messageQ_cb *osMessageQId;
#else
/**
 * \def osMessageQId
 * \brief Message ID identifies the message queue (pointer to a message queue control block).
 * \note CAN BE CHANGED: \b os_messageQ_cb is implementation specific in every CMSIS-RTOS.
 */
#endif
 
#ifndef osMailQId /* #002 */
/**
 * \brief Mail ID identifies the mail queue (pointer to a mail queue control block).
 * \note CAN BE CHANGED: \b os_mailQ_cb is implementation specific in every CMSIS-RTOS.
 */
typedef struct os_mailQ_cb *osMailQId;
#else
/**
 * \def osMailQId
 * \brief Mail ID identifies the mail queue (pointer to a mail queue control block).
 * \note CAN BE CHANGED: \b os_mailQ_cb is implementation specific in every CMSIS-RTOS.
 */
#endif
/** @}*/
/** @} cmsis_os_generic_types */


/* #017 Start */
/** @addtogroup cmsis_os_generic_types */
/** @{*/
/* #017 End */
 
/**
 *   \brief Event structure contains detailed information about an event.
 *  \note MUST REMAIN UNCHANGED: \b os_event shall be consistent in every CMSIS-RTOS.
 *        However the struct may be extended at the end.
 */
typedef struct  {
  osStatus                 status;     ///< status code: event or error information
  union  {
    uint32_t                    v;     ///< message as 32-bit value
    void                       *p;     ///< message or mail as void pointer
    int32_t               signals;     ///< signal flags
  } value;                             ///< event value
  union  {
    osMailQId             mail_id;     ///< mail id obtained by \ref osMailCreate
    osMessageQId       message_id;     ///< message id obtained by \ref osMessageCreate
  } def;                               ///< event definition
} osEvent;

/* #017 Start */
/* @} */
/* #017 End */

/* ==== Kernel Control Functions ==== */
/* #001 */
#ifndef ATTRIBUTES_osKernelInitialize
#define ATTRIBUTES_osKernelInitialize
#endif
#ifndef ATTRIBUTES_osKernelStart
#define ATTRIBUTES_osKernelStart
#endif
#ifndef ATTRIBUTES_osKernelRunning
#define ATTRIBUTES_osKernelRunning
#endif
#ifndef ATTRIBUTES_osKernelSysTick
#define ATTRIBUTES_osKernelSysTick
#endif
#ifndef ATTRIBUTES_osKernelMilliSecSysTick_suppl
#define ATTRIBUTES_osKernelMilliSecSysTick_suppl
#endif
/* #026 Start */
#ifndef ATTRIBUTES_osThreadGetName_suppl
#define ATTRIBUTES_osThreadGetName_suppl
#endif
/* #026 End */

/* #017 Start */
/** @addtogroup cmsis_os_kernel */
/** @{*/
/* #017 End */

/* #016 */
/**
 * \brief Initialize the RTOS Kernel.
 * \warning Cannot be called from ISR.
 * \return status code \ref osStatus that indicates the execution status of the function.
 * \note MUST REMAIN UNCHANGED: \b osKernelInitialize shall be consistent in every CMSIS-RTOS.
 */
ATTRIBUTES_osKernelInitialize osStatus osKernelInitialize (void); /* #001 */

#if TARGET_RTOS != RTOS_NoRTOS /* #022 */

/**
 * \brief Start the RTOS Kernel. See \ref cmsis_os_kernel_start_examples "example code".
 * \warning Cannot be called from ISR.
 *
 * @return osErrorISR: osKernelStart cannot be called from interrupt service routines.
 *   Otherwise, this function does never return.
 * Start the RTOS Kernel, call osApplicationDefine_suppl and then begin thread switching.
 *
 * \note MUST REMAIN UNCHANGED: \b osKernelStart shall be consistent in every CMSIS-RTOS.
 *
 */
ATTRIBUTES_osKernelStart osStatus osKernelStart (void); /* #001 */
 
#endif

/* #016 */
/**
 *\brief Check if the RTOS kernel is already started. Can be called from ISR.
 * Identifies if the RTOS kernel is started. For systems with the option to start the main
 * function as a thread this allows you to identify that the RTOS kernel is already running.
 * \return 0 RTOS is not started, 1 RTOS is started.
 * \note MUST REMAIN UNCHANGED: \b osKernelRunning shall be consistent in every CMSIS-RTOS.
 *
 * __Code Example:__
 ~~~~~~~~{.c}
 #include "cmsis_os.h"

 int main(void) {
   if (!osKernelRunning()){
     // if kernel is not running, initialize the kernel
     if (osKernelInitialize () != osOK){
       // check osStatus for other possible valid values
       // exit with an error message
     }
   }
 }
 ~~~~~~~~
 */
ATTRIBUTES_osKernelRunning int32_t osKernelRunning(void); /* #001 */
 
#if (defined (osFeature_SysTick)  &&  (osFeature_SysTick != 0))     // System Timer available

/**
 * \brief Get the RTOS kernel system timer counter. Can be called from ISR.
 * \return RTOS kernel system timer as 32-bit value
 *
 * Get the value of the Kernel SysTick timer for time comparison. The value is a rolling
 * 32-bit counter that is typically composed of the kernel system interrupt timer value
 * and an counter that counts these interrupts.
 *
 * This function allows the implementation of timeout checks. These are for example required
 * when checking for a busy status in a device or peripheral initialization routine.
 *
 * \note MUST REMAIN UNCHANGED: \b osKernelSysTick shall be consistent in every CMSIS-RTOS.
 *
 * __Code Example:__
 ~~~~~~~~{.c}
 #include "cmsis_os.h"
 void SetupDevice(void) {
   uint32_t tick;
   tick = osKernelSysTick(); // get start value of the Kernel system tick
   Device.Setup();
   // initialize a device or peripheral
   do { // poll device busy status for 100 microseconds
     if (!Device.Busy)
       break; // check if device is correctly initialized
   } while ( (osKernelSysTick() - tick) < osKernelTickMicroSec(100) );
   if (Device.Busy) {
     // in case device still busy, signal error
                    :
   }
   // start interacting with device}
                    :
 }
 ~~~~~~~~
 */
ATTRIBUTES_osKernelSysTick uint32_t osKernelSysTick (void); /* #001 */

/**
 * \brief Initialize the RTOS Kernel for creating objects.
 *
 * \note This functions has to be implemented individually by each application using the
 *   CMSIS-RTOS. The demand to have this function for RTOS object creation is a CMSIS-RTOS
 *   supplementary concept. That concept was needed in order to have an unmodified ThreadX
 *   as underlying RTOS.
 *
 * Initialize of the RTOS Kernel to allow peripheral setup and creation of other RTOS objects.
 * Creation of RTOS objects can be done with the functions:
 *   - osThreadCreate : \copybrief osThreadCreate
 *   - osTimerCreate : \copybrief osTimerCreate
 *   - osMutexCreate : \copybrief osMutexCreate
 *   - osSemaphoreCreate : \copybrief osSemaphoreCreate
 *   - osPoolCreate : \copybrief osPoolCreate
 *   - osMessageCreate : \copybrief osMessageCreate
 *   - osMailCreate : \copybrief osMailCreate
 *
 * The RTOS kernel starts thread switching after the function returned.
 * For examples see osKernelStart(void)
 *
 */
C_FUNC void osApplicationDefine_suppl(void);

#ifndef ATTRIBUTES_osKernelMilliSecSysTick_suppl
#define ATTRIBUTES_osKernelMilliSecSysTick_suppl
#endif

/**
 * \brief Convert milliseconds to system ticks.
 * \warning Cannot be called from ISR.
 *
 * @param msec Time value in milliseconds.
 * \return Time Value normalized to osKernelSysTickFrequency.
 *
 * \note The System Tick may be have a lower resolution than 1 one millisecond. I that case,
 * the tick value is always rounded __up__ according to the algorithm shown in the following
 * example:
 *
 * __Example 1:__ TICK RATE in Hertz = 100 -> Milliseconds per TICK = 10
 *
 * msec     |    macro result
 * ---------|-------------------
 *   0      |    0 (ticks)
 *   1..10  |    1 (tick)
 *  11..20  |    2 (ticks)
 *  21..30  |    3 (ticks)
 *
 * __Example 2:__ TICK RATE in Hertz = 50 -> Milliseconds per TICK = 20
 *
 * msec     |    result
 * ---------|-------------------
 *   0      |    0 (ticks)
 *   1..20  |    1 (tick)
 *  21..40  |    2 (ticks)
 *  41..60  |    3 (ticks)
 */
ATTRIBUTES_osKernelMilliSecSysTick_suppl bapi_SystemTick_t osKernelMilliSecSysTick_suppl( MsecType msec );


/** \name SysTick Macros */
/** @{*/

/**
 * \brief Specifies the frequency of the Kernel SysTick timer in Hz. The value is typically
 *   used to scale a time value and is for example used in osKernelSysTickMicroSec.
 * \sa osKernelSysTick(void)
 * \note Reflects the system timer setting and is typically defined in a configuration file.
 */
#define osKernelSysTickFrequency BAPI_RQ_SYSTEM_TICK_FREQUENCY_HZ /* #004 */
 
/**
 * \brief Allows you to scale a microsecond value to the frequency of the Kernel SysTick timer.
 *   This macro is typically used to check for short timeouts in polling loops.
 *
 * @param microsec Time value in microseconds.
 * @return Time value normalized to the \ref osKernelSysTickFrequency
 */
#define osKernelSysTickMicroSec(microsec) (((uint64_t)microsec * (osKernelSysTickFrequency)) / 1000000)
/** @}*/
 

#endif    // System Timer available

/* #017 Start */
/** @} cmsis_os_kernel*/
/* #017 End */

//  ==== Thread Management ====
/* #001 */
#ifndef ATTRIBUTES_osThreadCreate
#define ATTRIBUTES_osThreadCreate
#endif
#ifndef ATTRIBUTES_osThreadGetId
#define ATTRIBUTES_osThreadGetId
#endif
#ifndef ATTRIBUTES_osThreadTerminate
#define ATTRIBUTES_osThreadTerminate
#endif
#ifndef ATTRIBUTES_osThreadYield
#define ATTRIBUTES_osThreadYield
#endif
#ifndef ATTRIBUTES_osThreadSetPriority
#define ATTRIBUTES_osThreadSetPriority
#endif
#ifndef ATTRIBUTES_osThreadGetPriority
#define ATTRIBUTES_osThreadGetPriority
#endif
#ifndef ATTRIBUTES_osGetThreadState_suppl
#define ATTRIBUTES_osGetThreadState_suppl
#endif

/* #017 Start */
/** @addtogroup cmsis_os_thread */
/** @{*/
/* #017 End */

/**
 * \brief Entry point of a thread.
 * \note MUST REMAIN UNCHANGED: \b os_pthread shall be consistent in every CMSIS-RTOS.
 */
typedef void (*os_pthread) (void const *argument);


#if TARGET_RTOS != RTOS_NoRTOS /* #022 */

/// \brief Thread Definition structure contains startup information of a thread.
/// \note CAN BE CHANGED: \b os_thread_def is implementation specific in every CMSIS-RTOS.
typedef struct os_thread_def  {
  os_pthread             pthread;      ///< start address of thread function
  const char*            name;         ///< A descriptive name for the task. #003
  osPriority             tpriority;    ///< initial thread priority
  uint32_t               instances;    ///< maximum number of instances of that thread function
  uint32_t               stacksize;    ///< stack size requirements in bytes; 0 is default stack size
} osThreadDef_t;

/**
 * \brief Create a Thread Definition with function, priority, and stack requirements.
 *
 * @param name Name of the thread function.
 * @param priority Initial priority of the thread function.
 * @param instances Number of possible thread instances.
 * @param stacksz Stack size (in bytes) requirements for the thread function.
 *
 * \note CAN BE CHANGED: The parameters to \b osThreadDef shall be consistent but the
 *       macro body is implementation specific in every CMSIS-RTOS.
 */
#if defined (osObjectsExternal)  // object is external
#define osThreadDef(name, priority, instances, stacksz)  \
  extern const osThreadDef_t os_thread_def_##name
#else                            // define the object
#define osThreadDef(name, priority, instances, stacksz)  \
  const osThreadDef_t os_thread_def_##name = \
    { (name), #name, (priority), (instances), (stacksz)  }
#endif
 
/**
 * \brief Access to the thread definition for the function \ref osThreadCreate.
 * @param name Name of the thread definition object.
 * \note CAN BE CHANGED: The parameter to \b osThread shall be consistent but the
 *    macro body is implementation specific in every CMSIS-RTOS.
 *
 *
 * __Code Example:__
 ~~~~~~~~{.c}
#include "baseplate.h"
#include "rtos/cmsis-rtos/cmsis_os.h"

STATIC void MainThread(void* argument) { // define main thread function
  // Do your main thread work here.
}

// Define main thread with 0x380 bytes stack size
osThreadDef (MainThread, osPriorityNormal, 1, 0x380);

C_FUNC void osApplicationDefine_suppl (void) {
       :
  osThreadCreate( osThread(MainThread) // !!! Use osThread() to create the thread !!!
                                       // !!! definition for osThreadCreate()     !!!
    , NULL);
       :
}
 ~~~~~~~~
 */
#define osThread(name) \
  &os_thread_def_##name

/* #016 */
/**
 * \brief Create a thread and add it to Active Threads and set it to state READY.
 *  \warning Cannot be called from ISR.
 *
 *  @param[in] thread_def Thread definition referenced with \ref osThread.
 *  @param[in] argument Pointer that is passed to the thread function as start argument.
 *
 *  @return thread ID for reference by other functions or NULL in case of error.
 *
 *  Start a thread function by adding it to the Active Threads list and set it to state READY.
 *   The thread function receives the argument pointer as function argument when the
 *  function is started. When the priority of the created thread function is higher than the
 *  current RUNNING thread, the created thread function starts instantly and becomes the new
 *  RUNNING thread.
 *
 * __Code Example:__
 ~~~~~~~~{.c}
 #include "cmsis_os.h"

 void Thread_1 (void const *arg); // function prototype for Thread_1os
 ThreadDef (Thread_1, osPriorityNormal, 1, 0);    // define Thread_1

 void ThreadCreate_example (void) {
   osThreadId id =
     osThreadCreate (osThread (Thread_1), NULL);  // create the thread
   if (id == NULL) { // handle thread creation
     // Failed to create a thread
              :
   }
              :
   osThreadTerminate (id);                        // stop the thread
 }
 ~~~~~~~~
 *  \note MUST REMAIN UNCHANGED: \b osThreadCreate shall be consistent in every CMSIS-RTOS.
 */
ATTRIBUTES_osThreadCreate osThreadId osThreadCreate (const osThreadDef_t *thread_def, void *argument); /* #001 */
 
#endif /* #if TARGET_RTOS != RTOS_NoRTOS #022 */


/* #016 */
/**
 *  \brief Return the thread ID of the current running thread. Can be called from ISR.
 *  Will return null if called from ISR.
 *  @return thread ID for reference by other functions or _NULL_
 *     if called from non-kernel context respectively from ISR context.
 *
 *
 * __Code Example 1:__
 ~~~~~~~~{.c}
 void ThreadGetId_example(void) {
    osThreadId id = osThreadGetId (); // id for the currently running thread
    if (id == NULL) {
      // Failed to get the id; not in a thread, but in an ISR context.
               :
    }
               :
  }
 ~~~~~~~~
 *
 * __Code Example 2:__
 ~~~~~~~~{.c}
 int main (void) {
   ...
   osThreadId id = osThreadGetId (); // id will be NULL; not in a thread
   osKernelStart();                  // Start thread execution
 }
 ~~~~~~~~
 *  \note MUST REMAIN UNCHANGED: \b osThreadGetId shall be consistent in every CMSIS-RTOS.
 */
ATTRIBUTES_osThreadGetId osThreadId osThreadGetId (void); /* #001 */
 
/* #018 */
/**
 * \brief Terminate execution of a thread and remove it from Active Threads.
 *
 * \warning Cannot be called from ISR.
 * \warning You should be careful with terminating threads, because the memory
 *   that a thread may have allocated at the time of termination is not
 *   automatically freed.
 * @param [in] thread_id Thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
 * @return Status code that indicates the execution status
 *   of the function.
 *
 * Remove the thread function from the active thread list. If the thread is currently
 * RUNNING the execution will stop.
 * \note In case that _osThreadTerminate_ terminates the currently running task, the function
 *   never returns and other threads that are in the READY state are started.
 *
 * \ref osStatus_ "Status and Error Codes"
 *  - osOK: The specified thread has been successfully terminated.
 *  - osErrorParameter: thread_id is incorrect.
 *  - osErrorResource: thread_id refers to a thread that is not an active thread.
 *  - osErrorISR: _osThreadTerminate_ cannot be called from interrupt service routines.
 *
 * __Code Example:__
 ~~~~~~~~{.c}
 #include "cmsis_os.h"

 void Thread_1 (void const *arg);                // Function prototype for Thread_1

 osThreadDef (Thread_1, osPriorityNormal, 1, 0); // define Thread_1

 void ThreadTerminate_example(void) {
   osThreadId id =
     osThreadCreate (osThread (Thread_1), NULL); // Create the thread
   osStatus status = osThreadTerminate (id);     // Stop the thread
   if (status == osOK){
     // Thread was terminated successfully
                 :
   } else {
     // Failed to terminate a thread
                 :
   }
                 :
 }
 ~~~~~~~~
 *
 *
 * \note MUST REMAIN UNCHANGED: \b osThreadTerminate shall be consistent in every CMSIS-RTOS.
 *
 */
ATTRIBUTES_osThreadTerminate osStatus osThreadTerminate (osThreadId thread_id); /* #001 */
 
/**
 * \brief Change priority of an active thread.
 * \warning Cannot be called from ISR.
 *  @param[in] thread_id Thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
 *  @param[in] priority New priority value for the thread function.
 *  @return Status code that indicates the execution status of the function.
 *
 * \ref osStatus_ "Status and Error Codes"
 *  - _osOK_: the priority of the specified thread has been successfully changed.
 *  - _osErrorParameter_: thread_id is incorrect.
 *  - _osErrorValue_: incorrect priority value.
 *  - _osErrorResource_: thread_id refers to a thread that is not an active thread.
 *  - _osErrorISR_: _osThreadSetPriority_ cannot be called from interrupt service routines.
 *
 * __Code Example:__
 ~~~~~~~~{.c}
 #include "cmsis_os.h"

 void Thread_1(void const *arg) {    // Thread function
   osThreadId id = osThreadGetId(); // Obtain ID of current running thread
   if (id != NULL) {
     osStatus status = osThreadSetPriority(id, osPriorityBelowNormal);
     if (status == osOK){
       // Thread priority changed to BelowNormal
     } else {
       // Failed to set the priority
               :
     }
   } else {
       // Failed to get the id
        *      :
   }
               :
 }
 ~~~~~~~~
 *
 *  \note MUST REMAIN UNCHANGED: \b osThreadSetPriority shall be consistent in every CMSIS-RTOS.
 */
ATTRIBUTES_osThreadSetPriority osStatus osThreadSetPriority (osThreadId thread_id, osPriority priority); /* #001 */

/**
 *  \brief Get current priority of an active thread.
 *  @param [in] thread_id Thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
 *  @return current priority value of the thread function. In case of a failure the value
 *    _osPriorityError_ is returned.
 *
 * __Code Example:__
 ~~~~~~~~{.c}
 #include "cmsis_os.h"

 void Thread_1(void const *arg){    // Thread function
   osThreadId id = osThreadGetId(); // Obtain ID of current running thread
   if (id != NULL) {
     osPriority priority = osThreadGetPriority(id);
             :
   } else {
     // Failed to get the id
             :
   }
             :
 }
 ~~~~~~~~
 *
 *  \note MUST REMAIN UNCHANGED: \b osThreadGetPriority shall be consistent in every CMSIS-RTOS.
 */
ATTRIBUTES_osThreadGetPriority osPriority osThreadGetPriority (osThreadId thread_id); /* #001 */

/**
 * \brief Pass control to next thread that is in state __READY__.
 * \warning Cannot be called from ISR.
 * @param [in] thread_id Thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
 * @return status code that indicates the execution status of the function.
 *
 *  Pass control to the next thread that is in state __READY__. If there is no other thread
 *    in the state __READY__, the current thread continues execution and no thread switching
 *    occurs.
 *
 * \ref osStatus_ "Status and Error Codes"
 *   - _osOK_: The function has been correctly executed.
 *   - _osErrorISR_: osThreadYield cannot be called from interrupt service routines.
 *
 * __Code Example:__
 ~~~~~~~~{.c}
 #include "cmsis_os.h"
 void Thread_1(void const *arg) {  // Thread function
  while (1)
  {
    osStatus status = osThreadYield();
    if (status != osOK)  {
      // thread switch not occurred, not in a thread function
                   :
    }
   }
 }
 ~~~~~~~~
 *
 * \note MUST REMAIN UNCHANGED: \b osThreadYield shall be consistent in every CMSIS-RTOS.
 */
ATTRIBUTES_osThreadYield osStatus osThreadYield (void); /* #001 */


/* #024 Start */

/**
 * \brief Thread state values for the osGetThreadState function.
 * \note MUST REMAIN UNCHANGED: \b osThreadState shall be consistent in every CMSIS-RTOS.
 */

typedef enum osThreadState_ {
  osThreadReady,          // Thread is running or ready to run as the processor is available.
  osThreadSuspended,      // Thread is suspended from execution because it has nothing to do.
  osThreadBlocked,        // Thread is blocked, waiting for a resource.
  osThreadDeleted,        // Thread has been deleted.
  osThreadErrorGetState,  // Thread state is unavailable or a parameter error has occurred.
} osThreadState;


/**
 * \brief Get the state of a thread.
 *  @param [in] thread_id Thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
 *  @return current state of the thread. In case of a failure the value
 *    osThreadErrorGetState is returned.
 *
 *  \note MUST REMAIN UNCHANGED: \b osGetThreadState shall be consistent in every CMSIS-RTOS.
 */
ATTRIBUTES_osGetThreadState_suppl osThreadState osGetThreadState_suppl (osThreadId thread_id);

/* #024 End */

/* #025 Start */
#ifndef ATTRIBUTES_osThreadSuspend_suppl
#define ATTRIBUTES_osThreadSuspend_suppl
#endif
#ifndef ATTRIBUTES_osThreadResume_suppl
#define ATTRIBUTES_osThreadResume_suppl
#endif
/* #025 End */

#ifndef ATTRIBUTES_osThreadSuspendAll_suppl
#define ATTRIBUTES_osThreadSuspendAll_suppl
#endif
#ifndef ATTRIBUTES_osThreadResumeAll_suppl
#define ATTRIBUTES_osThreadResumeAll_suppl
#endif
/**
 * \brief Suspends a thread.
 *
 * \note Calls to osThreadSuspend_suppl are not accumulative - i.e. calling osThreadSuspend_suppl()
 * twice on the same task still only requires one call to osThreadResume_suppl() to ready
 * the suspended task.
 *
 * \warning Cannot be called from ISR.
 */
ATTRIBUTES_osThreadSuspend_suppl osStatus osThreadSuspend_suppl(osThreadId thread_id);

/**
 * \brief Resumes a suspended thread.
 *
 * \note A task that has been suspended by one or more calls
 * to osThreadSuspend_suppl() will be made available for running again by a single call
 * to osThreadResume_suppl().
 */
ATTRIBUTES_osThreadResume_suppl osStatus osThreadResume_suppl(osThreadId thread_id);

/**
 * \brief Suspends the scheduler without disabling interrupts.
 * \warning Cannot be called from ISR.
 */
ATTRIBUTES_osThreadSuspendAll_suppl void osThreadSuspendAll_suppl();

/**
 * \warning Cannot be called from ISR.
 * \brief Resumes scheduler activity after it was suspended by a call to osThreadSuspendAll_suppl().
 */
ATTRIBUTES_osThreadResumeAll_suppl void osThreadResumeAll_suppl();

/* #026 Start */
/**
 * \brief Suspends the scheduler without disabling interrupts.
 * \warning Cannot be called from ISR.
 */
ATTRIBUTES_osThreadGetName_suppl const char* osThreadGetName_suppl(osThreadId thread_id);
/* #026 End */

/* #017 Start */
/** @} */
/* #017 End */


/* ==== Generic Wait Functions ==== */
/* #001 */
#ifndef ATTRIBUTES_osDelay
#define ATTRIBUTES_osDelay
#endif
#ifndef ATTRIBUTES_osWait
#define ATTRIBUTES_osWait
#endif

/* #017 Start */
/** @addtogroup cmsis_os_generic_wait */
/** @{*/
/* #017 End */

#if (defined (osFeature_Wait)  &&  (osFeature_Wait != 0))     // Generic Wait available

/**
 * \brief Wait for Signal, Message, Mail, or Timeout.
 * \warning Cannot be called from ISR.
 * @param [in] millisec Timeout value or 0 in case of no time-out
 * @return event that contains signal, message, or mail information or error code.
 *
 * The osWait function puts a thread into the state WAITING and waits for any of the following events:
 *   - A __signal__ sent to that thread explicitly
 *   - A __message__ from a message object that is registered to that thread
 *   - A __mail__ from a mail object that is registered to that thread
 *
 * \note This function is optional and may not be provided by all CMSIS-RTOS implementations.
 *
 * \ref osStatus_ "Status and Error Codes"
 *  - osEventSignal_: a signal event occurred and is returned.
 *  - _osEventMessage_: a message event occurred and is returned.
 *  - _osEventMail_: a mail event occurred and is returned.
 *  - _osEventTimeout_: the time delay is expired.
 *  - _osErrorISR_: osDelay cannot be called from interrupt service routines.
 *
 * __Code Example:__
 ~~~~~~~~{.c}
 #include "cmsis_os.h"

 void Thread_1 (void const *arg)  {   // Thread function

   uint32_t waitTime = osWaitForever; // special "wait" value

   osEvent Event = osWait(waitTime);  // wait forever and until an event occurred
   switch (Event.status)
   {
   case osEventSignal:              // Signal arrived
       :                            // Event.value.signals contains the signal flags
     break;
   case osEventMessage:             // Message arrived
       :                            // Event.value.p contains the message pointer
       :                            // Event.def.message_id contains the message Id
     break;
   case osEventMail:                // Mail arrived
       :                            // Event.value.p contains the mail pointer
       :                            // Event.def.mail_id contains the mail Id
     break;
   case osEventTimeout:             // Timeout occurred
     break;
   default:                         // Error occurred
     break;
   }
       :
       :
 }
 ~~~~~~~~
 *
 * \note MUST REMAIN UNCHANGED: \b osWait shall be consistent in every CMSIS-RTOS.
 */
ATTRIBUTES_osWait osEvent osWait (uint32_t millisec); /* #001 */
 
#endif  // Generic Wait available

/**
 * \brief Wait for a specified time period in millisec (Time Delay).
 * \warning Cannot be called from ISR.
 * @param [in] millisec Time delay value
 * @return status code that indicates the execution status of the function.
 *
 * \ref osStatus_ "Status and Error Codes"
 *   - _osEventTimeout_: the time delay is executed.
 *   - _osErrorISR_: osDelay cannot be called from interrupt service routines.
 *
 * __Code Example:__
 ~~~~~~~~{.c}
 #include "cmsis_os.h"

 void Thread_1(void const *arg) { // Thread function
   uint32_t delayTime = 1000;             // Delay 1 second
          :
   osStatus status = osDelay (delayTime); // Suspend thread execution
   // handle error code
          :
 }
 ~~~~~~~~
 */
ATTRIBUTES_osDelay osStatus osDelay (uint32_t millisec); /* #001 */


/* #017 Start */
/** @} */
/* #017 End */
 
/* ==== Timer Management Functions ==== */
/* #001 */
#ifndef ATTRIBUTES_osTimerCreate
#define ATTRIBUTES_osTimerCreate
#endif
#ifndef ATTRIBUTES_osTimerStart
#define ATTRIBUTES_osTimerStart
#endif
#ifndef ATTRIBUTES_osTimerStop
#define ATTRIBUTES_osTimerStop
#endif
#ifndef ATTRIBUTES_osTimerDelete
#define ATTRIBUTES_osTimerDelete
#endif
/* #023 Start */
#ifndef ATTRIBUTES_osTimerIsActive_suppl
#define ATTRIBUTES_osTimerIsActive_suppl
#endif
/* #023 End */

/* #017 Start */
/** @addtogroup cmsis_os_timer */
/** @{*/
/* #017 End */

/**
 * \brief Entry point of a timer call back function.
 * \note MUST REMAIN UNCHANGED: \b os_ptimer shall be consistent in every CMSIS-RTOS.
 */
typedef void (*os_ptimer) (void const *argument);


/**
 * \brief Timer type value for the timer definition.
 * \note MUST REMAIN UNCHANGED: \b os_timer_type shall be consistent in every CMSIS-RTOS.
 */
typedef enum  {
  osTimerOnce             =     0,       /**< one-shot timer */
  osTimerPeriodic         =     1        /**< repeating timer */
} os_timer_type;

/**
 *  \brief Timer Definition structure contains timer parameters.
 *  \note CAN BE CHANGED: \b os_timer_def is implementation specific in every CMSIS-RTOS.
 */
typedef struct os_timer_def  {
  os_ptimer                 ptimer;    ///< start address of a timer function
  const char*               name;      ///< A descriptive name of the timer #006
} osTimerDef_t;

/**
 *  \brief Define a Timer object.
 *
 *  Define the attributes of a timer.
 *
 *  @param name Name of the timer object.
 *  @param function Name of the timer call back function.
 *  \note CAN BE CHANGED: The parameter to \b osTimerDef shall be consistent but the
 *        macro body is implementation specific in every CMSIS-RTOS.
 */
#if defined (osObjectsExternal)  // object is external
#define osTimerDef(name, function)  \
  extern const osTimerDef_t os_timer_def_##name
#else                            // define the object
#define osTimerDef(name, function)  \
  const osTimerDef_t os_timer_def_##name = \
    { (function), #name /* #006 */ }
#endif

/**
 *  \brief Access a Timer definition.
 *
 *  Access to the timer definition for the function \ref osTimerCreate.
 *
 *  @param  name Name of the timer object.
 *  \note CAN BE CHANGED: The parameter to \b osTimer shall be consistent but the
 *        macro body is implementation specific in every CMSIS-RTOS.
 */
#define osTimer(name) \
&os_timer_def_##name

/**
 * \brief Create a timer.
 * @param [in] timer_def Timer object referenced with \ref osTimer.
 * @param [in] type _osTimerOnce_ for one-shot or _osTimerPeriodic_ for periodic behavior.
 * @param [in] argument Argument to the timer call back function.
 * @return timer ID for reference by other functions or NULL in case of error.
 *
 * Create a one-shot or periodic timer and associate it with a callback function argument.
 * The timer is in stopped until it is started with \ref osTimerStart.
 *
 * \note MUST REMAIN UNCHANGED: \b osTimerCreate shall be consistent in every CMSIS-RTOS.
 *
 * __Code Example:__
 ~~~~~~~~{.c}
 #include "cmsis_os.h"

 void Timer1_Callback(void const *arg); // prototypes for timer callback function
 void Timer2_Callback  (void const *arg);

 osTimerDef (Timer1, Timer1_Callback);  // define timers
 osTimerDef (Timer2, Timer2_Callback);

 uint32_t  exec1 = 1;                   // argument for the timer call back function
 uint32_t  exec2 = 2;                   // argument for the timer call back function

 void TimerCreate_example(void) {

   osTimerId id1 = 0;                   // timer id
   osTimerId id2 = 0;                   // timer id

   // Create one-shoot timer
   id1 = osTimerCreate(osTimer(Timer1), osTimerOnce, &exec1);
   if (id1 != NULL) { // One-shoot timer created
            :
   }

   // Create periodic timer
   id2 = osTimerCreate(osTimer(Timer2), osTimerPeriodic, &exec2);
   if (id2 != NULL) { // Periodic timer created
            :
   }
 }
 ~~~~~~~~
 */
ATTRIBUTES_osTimerCreate osTimerId osTimerCreate (const osTimerDef_t *timer_def, os_timer_type type, void *argument); /* #001 */


/**
 * \brief Start or restart a timer.
 * \warning Cannot be called from ISR.
 * @param [in] timer_id Timer ID obtained by \ref osTimerCreate.
 * @param [in] millisec Time delay value of the timer.
 * @return status code that indicates the execution status of the function.
 *
 * Start or restart the timer.
 *
 * \ref osStatus_ "Status and Error Codes"
 *   - _osOK_: the specified timer has been started or restarted.
 *   - _osErrorISR_: osTimerStart cannot be called from interrupt service routines.
 *   - _osErrorParameter_: timer_id is incorrect.
 *
 * __Code Example:__
 ~~~~~~~~{.c}
 void TimerStart_example (void)  {
   // Create periodic timer
   osTimerId id = osTimerCreate (osTimer(Timer), osTimerPeriodic, &exec);
   if(id) {
     uint32_t timerDelay = 1000;
     osStatus status = osTimerStart (id, timerDelay); // start timer
     if(status != osOK) {
       // Timer could not be started
             :
     }
   }
             :
 }
 ~~~~~~~~
 *
 * \note MUST REMAIN UNCHANGED: \b osTimerStart shall be consistent in every CMSIS-RTOS.
 */
ATTRIBUTES_osTimerStart osStatus osTimerStart (osTimerId timer_id, uint32_t millisec); /* #001 */
 
/**
 * \brief Stop the timer.
 * \warning Cannot be called from ISR.
 * \param [in] timer_id Timer ID obtained by \ref osTimerCreate.
 * \return status code that indicates the execution status of the function.
 *
 * \ref osStatus_ "Status and Error Codes"
 *     - _osOK_: the specified timer has been stopped.
 *     - _osErrorISR_: osTimerStop cannot be called from interrupt service routines.
 *     - _osErrorParameter_: timer_id is incorrect.
 *     - _osErrorResource_: the timer is not started.
 *
 * __Code Example:__
 ~~~~~~~~{.c}
 #include "cmsis_os.h"

 void Timer_Callback  (void const *arg); // Prototype for timer callback function
 osTimerDef(Timer, Timer_Callback);  // Define timer

 void TimerStop_example(void) {
   osTimerId id =
     osTimerCreate (osTimer(Timer2), osTimerPeriodic, NULL);
   osTimerStart (id, 1000);           // Start timer
            :
   osStatus status = osTimerStop (id);// Stop timer

   if (status != osOK)  {
     // Timer could not be stopped
          :
   }
   osTimerStart (id, 1000);           // Start timer again
            :
 }
 ~~~~~~~~
 * \note MUST REMAIN UNCHANGED: \b osTimerStop shall be consistent in every CMSIS-RTOS.
 */
ATTRIBUTES_osTimerStop osStatus osTimerStop (osTimerId timer_id); /* #001 */
 
/**
 * \brief Delete a timer that was created by \ref osTimerCreate.
 * \warning Cannot be called from ISR.
 *
 * Delete the timer Object
 *
 * @param [in] timer_id Timer ID obtained by \ref osTimerCreate.
 * @return status code that indicates the execution status of the function.
 *
 * \ref osStatus_ "Status and Error Codes"
 *   - _osOK_: the specified timer has been deleted.
 *   - _osErrorISR_: osTimerDelete cannot be called from interrupt service routines.
 *   - _osErrorParameter_: timer_id is incorrect.
 *
 * __Code Example:__
 ~~~~~~~~{.c}
 #include "cmsis_os.h"

 void Timer_Callback  (void const *arg); // Prototype for timer callback function
 osTimerDef(Timer, Timer_Callback);      // Define timer

 void TimerDelete_example(void) {
   osTimerId id =
     osTimerCreate (osTimer(Timer2), osTimerPeriodic, NULL);
   osTimerStart (id, 1000);              // Start timer
            :
   osStatus status = osTimerDelete(id);  // Stop and delete timer

   if (status != osOK) {
     // Timer could not be deleted
            :
   }
 }
 ~~~~~~~~
 * \note MUST REMAIN UNCHANGED: \b osTimerDelete shall be consistent in every CMSIS-RTOS.
 */
ATTRIBUTES_osTimerDelete osStatus osTimerDelete (osTimerId timer_id); /* #001 */

/* #023 Start */

/**
 * \brief Get the state of a timer.
 *  @param [in] timer_id Timer ID obtained by \ref osTimerCreate
 * \return current state of the timer. 1 if timer is active, 0 if stopped or never started. If an error occurs, 
 *    0 is returned.
 *
 *  \note MUST REMAIN UNCHANGED: \b osTimerIsActive_suppl shall be consistent in every CMSIS-RTOS.
 */
ATTRIBUTES_osTimerIsActive_suppl uint32_t osTimerIsActive_suppl (osTimerId timer_id);

/* #023 End */

 
/** #017 Start */
/** @} */
/** #017 End */
 
/* ==== Signal Management ==== */
/* #001 */
#ifndef ATTRIBUTES_osSignalSet
#define ATTRIBUTES_osSignalSet
#endif
#ifndef ATTRIBUTES_osSignalClear
#define ATTRIBUTES_osSignalClear
#endif
#ifndef ATTRIBUTES_osSignalWait
#define ATTRIBUTES_osSignalWait
#endif

/* #017 Start */
/** @addtogroup cmsis_os_signal */
/** @{*/
/* #017 End */

/* #016 */
/**
 *  @brief Set the specified Signal Flags of an active thread.
 *    Can be called from ISR.
 *
 *  @param [in] thread_id Thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
 *  @param [in] signals Specifies the signal flags of the thread that should be set.
 *
 *  @return previous signal flags of the specified thread or 0x80000000 in case of incorrect
 *    parameters.
 *
 *  Set the signal flags of an active thread. This function may be used also within
 *  interrupt service routines.
 *
 * __Code Example:__
 ~~~~~~~~{.c}
 void Thread_1(void const *arg);               // Prototype of the thread function
 osThreadDef(Thread_1, osPriorityHigh, 1, 0);  // Define thread

 STATIC void Signal_example (void) {
   osThreadId id =
     osThreadCreate (osThread(Thread_1), NULL);// Create the thread

   if (id == NULL) {
     // Failed to create a thread.
   } else {
     int32_t signals =
       osSignalSet (id, 0x00000005);          // Send signals 0x05 to the created thread
   }
 }
 ~~~~~~~~
 *  \note MUST REMAIN UNCHANGED: \b osSignalSet shall be consistent in every CMSIS-RTOS.
 */
ATTRIBUTES_osSignalSet int32_t osSignalSet (osThreadId thread_id, int32_t signals); /* #001 */
 
/**
 * \brief Clear the specified Signal Flags of an active thread.
 * \warning Is not supported by Honeycomb MBSP, because need for it seems strange.
 *   Refer to the following thoughts:
 *   - A thread that is waiting for a signal must not clear the signals, because
 *      that will cause a race condition with another thread that sets the
 *      same signal. Hence when the \ref osSignalWait returns, the signals that
 *      the thread was waiting for must have been cleared already.
 *   - If another thread would clear a signal, the result would somehow be random
 *      depending on whether the thread has already processed the signal or not.
 *      So resetting the signal will not guarantee that the signal will not be
 *      processed by the thread.
 *
 * @param [in] thread_id Thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
 * @param [in] signals Specifies the signal flags of the thread that shall be cleared.
 * @return previous signal flags of the specified thread or 0x80000000 in case of
 *   incorrect parameters or call from ISR.
 * \note MUST REMAIN UNCHANGED: \b osSignalClear shall be consistent in every CMSIS-RTOS.
 */
ATTRIBUTES_osSignalClear int32_t osSignalClear (osThreadId thread_id, int32_t signals); /* #001 */
 
/* #019 */
/**
 * \brief Wait for one or more Signal Flags to become signaled for the current \b RUNNING thread.
 * \warning Cannot be called from ISR.
 *
 *  Suspend the execution of the current __RUNNING__ thread until
 *    __all specified signal flags__ with the parameter signals are set.
 *  When the parameter signals is __0__ the current __RUNNING__ thread is suspended until
 *    __any signal flag__  is set.
 *
 *  When these signal flags are already set, the function returns instantly. Otherwise the
 *  thread is put into the state WAITING. Signal flags that are reported as event are
 *  automatically cleared.
 *
 *  \note Implementation details for Honeycomb MBSP: CMSIS-RTOS specification is unclear
 *    about the signal clearance and the signals to be returned. __Honeycomb MBSP
 *    implementation__ consumes (clears) just the requested signals upon exit, but
 *    returns all notified signals (even those that the caller has not requested).
 *    That means that all non requested signal stay pending, even if they are returned
 *    as notified signals. That means that another \ref osSignalWait call that requests
 *    the still pending signals will return instantly.
 *
 *  @param [in] signals Wait until all specified signal flags set or 0 for any single signal flag.
 *  @param [in] millisec Timeout value or 0 in case of no time-out.
 *  @return event flag information or error code.
 *
 *  \note MUST REMAIN UNCHANGED: \b osSignalWait shall be consistent in every CMSIS-RTOS.
 */
ATTRIBUTES_osSignalWait osEvent osSignalWait (int32_t signals, uint32_t millisec); /* #001 */
 
/* #017 Start */
/** @} */
/* #017 End */
 
/* ==== Mutex Management ==== */
#ifndef ATTRIBUTES_osMutexCreate
#define ATTRIBUTES_osMutexCreate
#endif
#ifndef ATTRIBUTES_osMutexWait
#define ATTRIBUTES_osMutexWait
#endif
#ifndef ATTRIBUTES_osMutexRelease
#define ATTRIBUTES_osMutexRelease
#endif
#ifndef ATTRIBUTES_osMutexDelete
#define ATTRIBUTES_osMutexDelete
#endif

/* #017 Start */
/** @addtogroup cmsis_os_mutex */
/** @{*/
/* #017 End */

/**
 *  \brief Mutex Definition structure contains setup information for a mutex.
 *  \note CAN BE CHANGED: \b os_mutex_def is implementation specific in every CMSIS-RTOS.
 */
typedef struct os_mutex_def  {
/*  uint32_t                   dummy;   ///< dummy value. #007 */
  const char*                  name;    ///< A descriptive name of the mutex #007.
} osMutexDef_t;


/**
 * \brief Define a Mutex Object that is referenced by #osMutex(name).
 * @param name Name of the mutex object.
 * \note CAN BE CHANGED: The parameter to \b osMutexDef shall be consistent but the
 *    macro body is implementation specific in every CMSIS-RTOS.
 */
#if defined (osObjectsExternal)  // object is external
#define osMutexDef(name)  \
   extern const osMutexDef_t os_mutex_def_##name
#else                            // define the object
#define osMutexDef(name)  \
  const osMutexDef_t os_mutex_def_##name = \
    { #name /* #007 */ }
#endif
 
/**
 * \brief Access to mutex object for the functions \ref osMutexCreate.
 * @param name Name of the mutex object.
 * \note CAN BE CHANGED: The parameter to \b osMutex shall be consistent but the
 *    macro body is implementation specific in every CMSIS-RTOS.
 */
#define osMutex(name)  \
&os_mutex_def_##name
 
/**
 * \brief Create and Initialize a Mutex object.
 * @param [in] mutex_def Mutex definition referenced with \ref osMutex.
 * @return Mutex ID for reference by other functions or NULL in case of error.
 *
 * __Code Example:__
 ~~~~~~~~{.c}
 #include "cmsis_os.h"

 osMutexDef(Mutex_1); // Mutex definition
 osMutexId mutex_1_id = NULL;

 void CreateMutex_1(void) {
   if (mutex_1_id != NULL) {
     // Mutex not already created
     mutex_1_id =
       osMutexCreate(osMutex(Mutex_1));
     if (mutex_1_id != NULL)  {
       // Mutex object created
             :
     }
   }
 }
 ~~~~~~~~
 * \note MUST REMAIN UNCHANGED: \b osMutexCreate shall be consistent in every CMSIS-RTOS.
 */
ATTRIBUTES_osMutexCreate osMutexId osMutexCreate (const osMutexDef_t *mutex_def); /* #001 */
 
/**
 * \brief Wait until a Mutex becomes available.
 * \warning Cannot be called from ISR.
 * @param[in] mutex_id Mutex ID obtained by \ref osMutexCreate.
 * @param[in] millisec Timeout value or 0 in case of no time-out.
 * @return status code that indicates the execution status of the function.
 *
 * Wait until a Mutex becomes available. If no other thread has obtained the Mutex,
 * the function instantly returns and blocks the mutex object.
 *
 * The argument millisec specifies how long the system waits for a mutex. While the
 * system waits the thread that is calling this function is put into the state
 * WAITING. The millisec timeout can have the following values:
 *   - When _millisec_ is 0, the function returns instantly.
 *   - When _millisec_ is set to \ref #osWaitForever the function will wait for an infinite
 *       time until the mutex becomes available.
 *   - all other values specify a time in millisecond for a timeout.
 *
 * \ref osStatus_ "Status and Error Codes"
 *   - _osOK_: the mutex has been obtain.
 *   - _osErrorTimeoutResource_: the mutex could not be obtained in the given time.
 *   - _osErrorResource_: the mutex could not be obtained when no timeout was specified.
 *   - _osErrorParameter_: the parameter mutex_id is incorrect.
 *   - _osErrorISR_: \ref osMutexWait cannot be called from interrupt service routines.
 *
 * __Code Example:__
 ~~~~~~~~{.c}
 #include "cmsis_os.h"

 osMutexDef(Mutex_1); // Mutex definition
 osMutexId mutex_1_id = NULL;

 void CreateMutex_1(void); // Function prototype that creates the Mutex

 void WaitMutex_1() {
   if (mutex_1_id != NULL) {
     osStatus status =
       osMutexWait(mutex_1_id, 0);
     if (status != osOK) {
         // handle failure code
              :
     }
   }
 }
 ~~~~~~~~
 *
 * \note MUST REMAIN UNCHANGED: \b osMutexWait shall be consistent in every CMSIS-RTOS.
 */
ATTRIBUTES_osMutexWait osStatus osMutexWait (osMutexId mutex_id, uint32_t millisec); /* #001 */
 
/**
 *
 * \brief Release a Mutex that was obtained by \ref osMutexWait.
 * \warning Cannot be called from ISR.
 * @param [in] mutex_id mutex ID obtained by \ref osMutexCreate.
 * @return status code that indicates the execution status of the function.
 *
 * Release a Mutex that was obtained with osMutexWait. Other threads that currently
 * wait for the same mutex will be now put into the state __READY__.
 *
 * \ref osStatus_ "Status and Error Codes"
 *   - _osOK_: The mutex has been correctly released.
 *   - _osErrorResource_: The mutex was not obtained before.
 *   - _osErrorParameter_: The parameter mutex_id is incorrect.
 *   - _osErrorISR_: osMutexRelease cannot be called from interrupt service routines.
 *
 * __Code Example:__
 ~~~~~~~~{.c}
 #include "cmsis_os.h"

 osMutexDef(Mutex_1); // Mutex definition
 osMutexId mutex_1_id = NULL;

 void CreateMutex_1(void); // Function prototype that creates the Mutex

 void void ReleaseMutex_1 () {
   if (mutex_1_id != NULL) {
     osStatus status = osMutexRelease(mutex_1_id);
     if (status != osOK) {
       // handle failure code
            :
     }
   }
 }
 ~~~~~~~~
 *
 *  \note MUST REMAIN UNCHANGED: \b osMutexRelease shall be consistent in every CMSIS-RTOS.
 */
ATTRIBUTES_osMutexRelease osStatus osMutexRelease (osMutexId mutex_id); /* #001 */
 
/**
 * \brief Delete a Mutex that was created by \ref osMutexCreate.
 * \warning Cannot be called from ISR.
 * @param [in] mutex_id Mutex ID obtained by \ref osMutexCreate.
 * @return status code that indicates the execution status of the function.
 *
 * Delete a Mutex object. The function releases internal memory obtained for Mutex
 * handling. After this call the mutex_id is no longer valid and cannot be used. The
 * Mutex may be created again using the function \ref osMutexCreate.
 *
 * \ref osStatus_ "Status and Error Codes"
 *   - _osOK_: the mutex object has been deleted.
 *   - _osErrorISR_: osMutexDelete cannot be called from interrupt service routines.
 *   - _osErrorResource_: all tokens have already been released.
 *   - _osErrorParameter_: the parameter mutex_id is incorrect.
 *
 * __Code Example:__
 ~~~~~~~~{.c}
 #include "cmsis_os.h"

 osMutexDef(Mutex_1); // Mutex definition
 osMutexId mutex_1_id = NULL;

 void CreateMutex_1(void); // Function prototype that creates the Mutex

 void DeleteMutex_1() {
   if (mutex_1_id != NULL) {
     osStatus status = osMutexDelete(mutex_1_id);
     if (status == osOK) {
       mutex_1_id = NULL;
     } else {
       // handle failure code
            :
     }
   }
 }
 ~~~~~~~~
 *
 *  \note MUST REMAIN UNCHANGED: \b osMutexDelete shall be consistent in every CMSIS-RTOS.
 */
ATTRIBUTES_osMutexDelete osStatus osMutexDelete (osMutexId mutex_id); /* #001 */
 
/* #017 Start */
/** @} */
/* #017 End */
 
/* ==== Semaphore Management Functions ==== */
/* #001 */
#ifndef ATTRIBUTES_osSemaphoreCreate
#define ATTRIBUTES_osSemaphoreCreate
#endif
#ifndef ATTRIBUTES_osSemaphoreWait
#define ATTRIBUTES_osSemaphoreWait
#endif
#ifndef ATTRIBUTES_osSemaphoreRelease
#define ATTRIBUTES_osSemaphoreRelease
#endif
#ifndef ATTRIBUTES_osSemaphoreDelete
#define ATTRIBUTES_osSemaphoreDelete
#endif

#if (defined (osFeature_Semaphore)  &&  (osFeature_Semaphore != 0))     // Semaphore available

/* #017 Start */
/** @addtogroup cmsis_os_semaphore */
/** @{*/
/* #017 End */

/**
 *   \brief Semaphore Definition structure contains setup information for a semaphore.
 *  \ingroup cmsis_os_semaphore
 *  \note CAN BE CHANGED: \b os_semaphore_def is implementation specific in every CMSIS-RTOS.
 */
typedef struct os_semaphore_def  {
/*  uint32_t                   dummy;   ///< dummy value. #008 */
  const char*                  name;    ///< A descriptive name of the semaphore #008.
} osSemaphoreDef_t;

/**
 * \brief Define a semaphore object that is referenced by \ref osSemaphore.
 * @param name Name of the semaphore object.
 * \note CAN BE CHANGED: The parameter to \b osSemaphoreDef shall be consistent but the
 *    macro body is implementation specific in every CMSIS-RTOS.
 */
#if defined (osObjectsExternal)  // object is external
#define osSemaphoreDef(name)  \
  extern const osSemaphoreDef_t os_semaphore_def_##name
#else                            // define the object
#define osSemaphoreDef(name)  \
  const osSemaphoreDef_t os_semaphore_def_##name = \
    { #name /* #008 */ }
#endif
 
/**
 * \brief Access to semaphore object for the functions \ref osSemaphoreCreate.
 * @param name Name of the semaphore object.
 * \note CAN BE CHANGED: The parameter to \b osSemaphore shall be consistent but the
 *    macro body is implementation specific in every CMSIS-RTOS.
 */
#define osSemaphore(name)  \
&os_semaphore_def_##name
 
/**
 * \brief Create and Initialize a Semaphore object used for managing resources.
 * @param[in] semaphore_def Semaphore definition referenced with \ref osSemaphore.
 * @param[in] count         Number of available resources.
 *
 * @return semaphore ID for reference by other functions or NULL in case of error.
 *
 *  Create and initialize a Semaphore object that is used to manage access to
 *  shared resources. The parameter count specifies the number of available
 *  resources. The count value 1 creates a binary semaphore.
 *
 * __Code Example:__
 * \include CMSIS_RTOS_create_semaphore_example.c
 *
 * \note MUST REMAIN UNCHANGED: \b osSemaphoreCreate shall be consistent in every CMSIS-RTOS.
 */
ATTRIBUTES_osSemaphoreCreate osSemaphoreId osSemaphoreCreate (const osSemaphoreDef_t *semaphore_def, int32_t count); /* #001 */
 
/**
 * \brief Wait until a Semaphore token becomes available.
 * @param [in] semaphore_id Semaphore object referenced with \ref osSemaphoreCreate.
 * @param [in] millisec Timeout value or 0 in case of no time-out.
 * @return number of available tokens, or -1 in case of incorrect parameters.
 *
 * When no Semaphore token is available, the function waits for the time specified with
 * the parameter millisec. The argument millisec specifies how long the system waits
 * for a Semaphore token to become available. While the system waits the thread that is
 * calling this function is put into the state __WAITING__. The millisec timeout can
 * have the following values:
 *
 *  - When _millisec_ is 0, the function returns instantly.
 *  - When _millisec_ is set to \ref #osWaitForever the function will wait for an infinite
 *      time until the Semaphore token becomes available.
 *  - All other values specify a time in millisecond for a timeout.
 *
 * \note MUST REMAIN UNCHANGED: \b osSemaphoreWait shall be consistent in every CMSIS-RTOS.
 */
ATTRIBUTES_osSemaphoreWait int32_t osSemaphoreWait (osSemaphoreId semaphore_id, uint32_t millisec); /* #001 */

/* #016 */
/**
 * \brief Release a Semaphore token.
 * @param [in] semaphore_id  Semaphore object referenced with \ref osSemaphoreCreate.
 * @return status code that indicates the execution status of the function.
 *
 * This increments the count of available semaphore tokens.
 *
 * \ref osStatus_ "Status and Error Codes"
 *   - _osOK_: The semaphore has been released.
 *   - _osErrorResource_: All tokens have already been released.
 *   - _osErrorParameter_: The parameter semaphore_id is incorrect.
 *
 * \note MUST REMAIN UNCHANGED: \b osSemaphoreRelease shall be consistent in every CMSIS-RTOS.
 */
ATTRIBUTES_osSemaphoreRelease osStatus osSemaphoreRelease (osSemaphoreId semaphore_id); /* #001 */
 
/**
 * \brief Delete a Semaphore that was created by \ref osSemaphoreCreate.
 * \warning Cannot be called from ISR.
 * \param[in] semaphore_id  Semaphore object referenced with \ref osSemaphoreCreate.
 * \return status code that indicates the execution status of the function.
 *
 *  Delete a Semaphore object. The function releases internal memory obtained for Semaphore
 *  handling. After this call the semaphore_id is no longer valid and cannot be used. The
 *  Semaphore may be created again using the function \ref osSemaphoreCreate.
 *
 * \ref osStatus_ "Status and Error Codes"
 *   - _osOK_: the semaphore object has been deleted.
 *   - _osErrorISR_: osSemaphoreDelete cannot be called from interrupt service routines.
 *   - _osErrorResource_: the semaphore object could not be deleted.
 *   - _osErrorParameter_: the parameter semaphore_id is incorrect.
 *
 * \note MUST REMAIN UNCHANGED: \b osSemaphoreDelete shall be consistent in every CMSIS-RTOS.
 */
ATTRIBUTES_osSemaphoreDelete osStatus osSemaphoreDelete (osSemaphoreId semaphore_id); /* #001 */
 
/* #017 Start */
/* @} */
/* #017 End */
 
#endif     // Semaphore available

/* ==== Memory Pool Management Functions ==== */
/* #001 */
#ifndef ATTRIBUTES_osPoolCreate
#define ATTRIBUTES_osPoolCreate
#endif
#ifndef ATTRIBUTES_osPoolAlloc
#define ATTRIBUTES_osPoolAlloc
#endif
#ifndef ATTRIBUTES_osPoolCAlloc
#define ATTRIBUTES_osPoolCAlloc
#endif
#ifndef ATTRIBUTES_osPoolFree
#define ATTRIBUTES_osPoolFree
#endif

#if (defined (osFeature_Pool)  &&  (osFeature_Pool != 0))  // Memory Pool Management available

/* #017 Start */
/** @addtogroup cmsis_os_memory_pool */
/** @{*/
/* #017 End */

/**
 *  \brief Definition structure for memory block allocation.
 *  \note CAN BE CHANGED: \b os_pool_def is implementation specific in every CMSIS-RTOS.
 */
typedef struct os_pool_def  {
  uint32_t                 pool_sz;    ///< number of items (elements) in the pool
  uint32_t                 item_sz;    ///< size of an item
/* #005
 * void                       *pool;    ///< pointer to memory for pool
 */
  const char*                  name;    ///< A descriptive name of the pool #009.
} osPoolDef_t;

/**
 * \brief Define a memory pool that is referenced by \ref osPool.
 * @param name Name of the memory pool.
 * @param no Maximum number of blocks (objects) in the memory pool.
 * @param type Data type of a single block (object).
 * \note CAN BE CHANGED: The parameter to \b osPoolDef shall be consistent but the
 *    macro body is implementation specific in every CMSIS-RTOS.
 */
#if defined (osObjectsExternal)  // object is external
#define osPoolDef(name, no, type)   \
  extern const osPoolDef_t os_pool_def_##name
#else                            // define the object
#define osPoolDef(name, no, type)   \
  const osPoolDef_t os_pool_def_##name = \
    { (no), sizeof(type), #name /* #005, #009 */}
#endif
 
/**
 * \brief Access a memory pool for the functions \ref osPoolCreate.
 * @param name Name of the memory pool
 * \note CAN BE CHANGED: The parameter to \b osPool shall be consistent but the
 *    macro body is implementation specific in every CMSIS-RTOS.
 */
#define osPool(name) \
&os_pool_def_##name
 
/**
 * \brief Create and Initialize a memory pool.
 * @param [in] pool_def memory pool definition referenced with \ref osPool.
 * @return memory pool ID for reference by other functions or NULL in case of error.
 *
 * __Code Example:__
 ~~~~~~~~{.c}
 #include "cmsis_os.h"

 struct MemBlock{                            // Define memory block structure
   uint8_t Buf[32];
   uint8_t Idx;
 };

 osPoolDef (MemPool_1, 8,  struct MemBlock); // Define Memory Pool with 8 entries of
                                             // the size of a MemBlock structure
 osPoolId MemPool_1_ID = NULL;

 void CreateMemPool_example (void) {
   MemPool_1_ID = osPoolCreate(osPool(MemPool_1));
   if (MemPool_1_ID != NULL) {             // memory pool created
           :
   }
 }
 ~~~~~~~~
 * \note MUST REMAIN UNCHANGED: \b osPoolCreate shall be consistent in every CMSIS-RTOS.
 */
ATTRIBUTES_osPoolCreate osPoolId osPoolCreate (const osPoolDef_t *pool_def); /* #001 */

/* #016 */
/**
 * \brief Allocate a memory block from a memory pool.
 *   Can be called from ISR.
 * @param [in] pool_id Memory pool ID obtain referenced with \ref osPoolCreate.
 * @return address of the allocated memory block or NULL in case of no memory available.
 *
 * __Code Example:__
 ~~~~~~~~{.c}
 #include "cmsis_os.h"

 osPoolDef (MemPool_1, 8,  struct MemBlock); // Define Memory Pool with 8 entries
                                             // of the size of a MemBlock structure
 osPoolId MemPool_1_ID = NULL;

 void AllocMemoryPoolBlock_example(void) {
   MemPool_1_ID =
     osPoolCreate (osPool (MemPool));
   if (MemPool_1_ID != NULL) {
            :
     // Allocate a memory block
     struct MemBlock *addr =
       (struct MemBlock *)osPoolAlloc(MemPool_1_ID);
     if (addr != NULL) {
       // memory block was allocated
                :
     }
   }
 }
 ~~~~~~~~
 *
 * \note MUST REMAIN UNCHANGED: \b osPoolAlloc shall be consistent in every CMSIS-RTOS.
 */
ATTRIBUTES_osPoolAlloc void *osPoolAlloc (osPoolId pool_id); /* #001 */
 
/* #016 */
/**
 * \brief Allocate a memory block from a memory pool and set memory block to zero.
 *    Can be called from ISR.
 * @param[in] pool_id Memory pool ID obtain referenced with \ref osPoolCreate.
 * @return address of the allocated memory block or NULL in case of no memory available.
 *
 * __Code Example:__
 ~~~~~~~~{.c}
 #include "cmsis_os.h"

 osPoolDef (MemPool_1, 8,  struct MemBlock); // Define Memory Pool with 8 entries
                                             // of the size of a MemBlock structure
 osPoolId MemPool_1_ID = NULL;

 void CAllocMemoryPoolBlock_example(void) {
   MemPool_1_ID =
     osPoolCreate (osPool (MemPool));
   if (MemPool_1_ID != NULL) {
            :
     // Allocate a memory block initialized with zero
     struct MemBlock *addr =
       (struct MemBlock *)osPoolCAlloc(MemPool_1_ID);
     if (addr != NULL) {
       // memory block was allocated
                :
     }
   }
 }
 ~~~~~~~~
 *
 *  \note MUST REMAIN UNCHANGED: \b osPoolCAlloc shall be consistent in every CMSIS-RTOS.
 */
ATTRIBUTES_osPoolCAlloc void *osPoolCAlloc (osPoolId pool_id); /* #001 */
 
/* #016 */
/**
 * \brief Return an allocated memory block back to a specific memory pool.
 *    Can be called from ISR.
 * @param [in] pool_id Memory pool ID obtain referenced with \ref osPoolCreate.
 * @param [in] block Address of the allocated memory block that is returned to the memory pool.
 * @return status code that indicates the execution status of the function.
 *
 * __Code Example:__
 ~~~~~~~~{.c}
 #include "cmsis_os.h"

 struct MemBlock {
    uint8_t Buf[32];
    uint8_t Idx;
    };

 osPoolDef (MemPool_1, 8,  struct MemBlock); // Define Memory Pool with 8 entries of
                                             // the size of a MemBlock structure
 osPoolId MemPool_1_ID = NULL;

 void FreeMemoryPoolBlock_example(void) {
   MemPool_1_ID = osPoolCreate(osPool (MemPool_1));
   if (MemPool_1_ID != NULL) {
     struct MemBlock *addr =
       (struct MemBlock *)osPoolCAlloc (MemPool_1_ID);
     if (addr != NULL) {
            :
       // Return a memory block back to pool
       osStatus status = osPoolFree (MemPool_1_ID, addr);
       if (status==osOK) {
         // Handle status code
       }
     }
   }
 }
 ~~~~~~~~
 *
 * \ref osStatus_ "Status and Error Codes"
 * osOK: the memory block is released.
 *   - _osErrorValue_: block does not belong to the memory pool.
 *   - _osErrorParameter_: a parameter is invalid or outside of a permitted range.
 *
 * \note MUST REMAIN UNCHANGED: \b osPoolFree shall be consistent in every CMSIS-RTOS.
 */
ATTRIBUTES_osPoolFree osStatus osPoolFree (osPoolId pool_id, void *block); /* #001 */


#ifndef ATTRIBUTES_osPoolDestroy_suppl
#define ATTRIBUTES_osPoolDestroy_suppl
#endif
#ifndef ATTRIBUTES_osPoolBlockSizeGet_suppl
#define ATTRIBUTES_osPoolBlockSizeGet_suppl
#endif

/**
 * \brief Delete a memory pool
 *
 * There are 2 case when a memory pool cannot be deleted:
 *
 * - When the memory pool of this mail queue has still memory blocks in use after the timeout
 *     time.
 * - When another thread has already started the destruction of this mail queue.
 *
 * \warning A pointer to the pool id is passed as argument to that function. When the function
 * returns successfully, the pool id that this pointer is pointing to, is set to zero. Otherwise
 * it stays untouched. This ensures that, that the pool id is invalidated, when the pool became
 * invalid. However, there might exist multiple clones of the pool id, that still refer to the
 * invalid destroyed pool. Those clones must be reseted to zero manually.
 *
 * \return
 *  - osOK in case of success
 *  - osErrorResource, if an other thread is already is already destroying the memory pool
 *  - osErrorTimeoutResource, if there is still a memory block in use after the timeout period.
 */
ATTRIBUTES_osPoolDestroy_suppl osStatus osPoolDestroy_suppl(
   osPoolId* pool_id_ptr   /**< [in] [out] Pointer to the memory pool to be deleted */
  ,uint32_t millisec       /**< timeout. */
  );

/**
 * \brief Retrieve the block size of a memory pool.
 * \return The memory block size in bytes.
 */
ATTRIBUTES_osPoolBlockSizeGet_suppl uint32_t osPoolBlockSizeGet_suppl(osPoolId pool_id);


/* #017 Start */
/** @} */
/* #017 End */

#endif   // Memory Pool Management available

/*  ==== Message Queue Management Functions ==== */
/* #001 */
#ifndef ATTRIBUTES_osMessageCreate
#define ATTRIBUTES_osMessageCreate
#endif
#ifndef ATTRIBUTES_osMessagePut
#define ATTRIBUTES_osMessagePut
#endif
#ifndef ATTRIBUTES_osMessageGet
#define ATTRIBUTES_osMessageGet
#endif

#if (defined (osFeature_MessageQ)  &&  (osFeature_MessageQ != 0))     // Message Queues available

/* #017 Start */
/** @addtogroup cmsis_os_message_queue */
/** @{*/
/* #017 End */

/**
 *   \brief Definition structure for message queue.
 *  \ingroup cmsis_os_message_queue
 *  \note CAN BE CHANGED: \b os_messageQ_def is implementation specific in every CMSIS-RTOS.
 */
typedef struct os_messageQ_def  {
  uint32_t                queue_sz;    ///< number of elements in the queue
  uint32_t                 item_sz;    ///< size of an item
/* #011
 * void                       *pool;    ///< memory array for messages
 */
  const char*                  name;    ///< A descriptive name of the pool #010.
} osMessageQDef_t;

/**
 * \brief Define the attributes of a message queue created by the function \ref osMessageCreate using \ref osMessageQ.
 * @param name Name of the queue.
 * @param queue_sz Maximum number of messages in the queue.
 * @param type Data type of a single message element (for debugger).
 *
 * \note CAN BE CHANGED: The parameter to \b osMessageQDef shall be consistent but the
 *    macro body is implementation specific in every CMSIS-RTOS.
 */
#if defined (osObjectsExternal)  // object is external
#define osMessageQDef(name, queue_sz, type)   \
  extern const osMessageQDef_t os_messageQ_def_##name
#else                            // define the object
#define osMessageQDef(name, queue_sz, type)   \
  const osMessageQDef_t os_messageQ_def_##name = \
    { (queue_sz), sizeof (type), #name /* #010, #011 */  }
#endif
 
/**
 * \brief Access to the message queue definition for the function \ref osMessageCreate..
 * @param name Name of the queue
 *  \note CAN BE CHANGED: The parameter to \b osMessageQ shall be consistent but the
 *     macro body is implementation specific in every CMSIS-RTOS.
 */
#define osMessageQ(name) \
&os_messageQ_def_##name

/**
 * \brief Create and Initialize a Message Queue.
 * @param [in] queue_def Queue definition referenced with \ref osMessageQ.
 * @param [in] thread_id Thread ID (obtained by \ref osThreadCreate or \ref osThreadGetId) or NULL.
 * @return message Queue ID for reference by other functions or NULL in case of error.
 *
 * __Code Example:__
 * \include CMSIS_RTOS_create_message_queue_example.c
 *
 * \note MUST REMAIN UNCHANGED: \b osMessageCreate shall be consistent in every CMSIS-RTOS.
 */
ATTRIBUTES_osMessageCreate osMessageQId osMessageCreate (const osMessageQDef_t *queue_def, osThreadId thread_id); /* #001 */
 
/* #016 */
/**
 * \brief Put the message info in a message queue specified by queue_id.
 *    Can be called from ISR.
 * @param[in] queue_id Message queue ID obtained with \ref osMessageCreate.
 * @param[in] info Message information.
 * @param[in] millisec Timeout value or 0 in case of no time-out.
 * @return status code that indicates the execution status of the function.
 *
 * When the message queue is full, the system retries for a specified time with
 * millisec. While the system retries the thread that is calling this function
 * is put into the state WAITING. The millisec timeout can have the following values:
 *   - When _millisec_ is 0, the function returns instantly.
 *   - When _millisec_ is set to \ref #osWaitForever the function will wait for an
 *       infinite time until a message queue slot becomes available.
 *   - All other values specify a time in millisecond for a timeout.
 *
 *  \note MUST REMAIN UNCHANGED: \b osMessagePut shall be consistent in every CMSIS-RTOS.
 */
ATTRIBUTES_osMessagePut osStatus osMessagePut (osMessageQId queue_id, uint32_t info, uint32_t millisec); /* #001 */
 
/* #016 */
/**
 * \brief Get a Message or Wait for a Message from a Queue.
 *    Can be called from ISR.
 * @param[in]     queue_id      message queue ID obtained with \ref osMessageCreate.
 * @param[in]     millisec      timeout value or 0 in case of no time-out.
 * @return event information that includes status code.
 *
 * Suspend the execution of the current RUNNING thread until a message arrives.
 * When a message is already in the queue, the function returns instantly with
 * the message information.
 *
 * The argument millisec specifies how long the system waits for a message to
 * become available. While the system waits the thread that is calling this
 * function is put into the state WAITING. The millisec timeout value can have
 * the following values:
 *
 *   - When _millisec_ is 0, the function returns instantly.
 *   - When _millisec_ is set to \ref #osWaitForever the function will wait for an infinite
 *      time until a message arrives.
 *   - All other values specify a time in millisecond for a timeout.
 *
 * \ref osStatus_ "Status and Error Codes"
 *   - _osOK_: no message is available in the queue and no timeout was specified.
 *   - _osEventTimeout_: no message has arrived during the given timeout period.
 *   - _osEventMessage_: message received, value.p contains the pointer to message.
 *   - _osErrorParameter_: a parameter is invalid or outside of a permitted range.
 *
 *  \note MUST REMAIN UNCHANGED: \b osMessageGet shall be consistent in every CMSIS-RTOS.
 */
ATTRIBUTES_osMessageGet osEvent osMessageGet (osMessageQId queue_id, uint32_t millisec); /* #001 */
 
/* #017 Start */
/** @} */
/* #017 End */

#endif     // Message Queues available

 
/*  ==== Mail Queue Management Functions ==== */
/* #001 */
#ifndef ATTRIBUTES_osMailCreate
#define ATTRIBUTES_osMailCreate
#endif
#ifndef ATTRIBUTES_osMailAlloc
#define ATTRIBUTES_osMailAlloc
#endif
#ifndef ATTRIBUTES_osMailCAlloc
#define ATTRIBUTES_osMailCAlloc
#endif
#ifndef ATTRIBUTES_osMailPut
#define ATTRIBUTES_osMailPut
#endif
#ifndef ATTRIBUTES_osMailGet
#define ATTRIBUTES_osMailGet
#endif
#ifndef ATTRIBUTES_osMailFree
#define ATTRIBUTES_osMailFree
#endif
 
#if (defined (osFeature_MailQ)  &&  (osFeature_MailQ != 0))     // Mail Queues available

/* #017 Start */
/** @addtogroup cmsis_os_mail_queue */
/** @{*/
/* #017 End */

/**
 *  \brief Definition structure for mail queue.
 *  \note CAN BE CHANGED: \b os_mailQ_def is implementation specific in every CMSIS-RTOS.
 */
typedef struct os_mailQ_def  {
  uint32_t                queue_sz;    ///< number of elements in the queue
  uint32_t                 item_sz;    ///< size of an item
/* #012
 * void                       *pool;    ///< memory array for mail
 */
  const char*                  name;    ///< A descriptive name of the mail queue #013.
} osMailQDef_t;


/**
 * \brief Create a Mail Queue Definition.
 * @param name Name of the queue
 * @param queue_sz Maximum number of mails in queue
 * @param type Data type of a single message element
 *
 * \note CAN BE CHANGED: The parameter to \b osMailQDef shall be consistent but the
 *     macro body is implementation specific in every CMSIS-RTOS.
 */
#if defined (osObjectsExternal)  // object is external
#define osMailQDef(name, queue_sz, type) \
  extern const osMailQDef_t os_mailQ_def_##name
#else                            // define the object
#define osMailQDef(name, queue_sz, type) \
  const osMailQDef_t os_mailQ_def_##name =  \
    { (queue_sz), sizeof (type), #name /* #012, #013*/ }
#endif
 
/**
 * \brief Access to the mail queue definition for the function \ref osMailCreate..
 * @param name Name of the queue
 * \note CAN BE CHANGED: The parameter to \b osMailQ shall be consistent but the
 *   macro body is implementation specific in every CMSIS-RTOS.
 */
#define osMailQ(name)  \
&os_mailQ_def_##name
 
/**
 * \brief Create and Initialize mail queue.
 * \param [in] queue_def Reference to the mail queue definition obtain with \ref osMailQ
 * \param [in] thread_id Thread ID (obtained by \ref osThreadCreate or \ref osThreadGetId) or NULL.
 * \return Mail queue ID for reference by other functions or NULL in case of error.
 *
 * __Code Example:__
 * \include CMSIS_RTOS_create_mail_queue_example.c
 *
 *  \note MUST REMAIN UNCHANGED: \b osMailCreate shall be consistent in every CMSIS-RTOS.
 */
ATTRIBUTES_osMailCreate osMailQId osMailCreate (const osMailQDef_t *queue_def, osThreadId thread_id); /* #001 */

/* #016 */
/**
 * \brief Allocate a memory block from the mail queue that is filled with the mail information.
 *   The memory block returned contains __random data__.
 *   Can be called from ISR.
 * @param [in] queue_id Mail queue ID obtained with \ref osMailCreate.
 * @param [in] millisec Timeout value or 0 in case of no time-out
 * @return pointer to memory block that can be filled with mail or NULL in case of error.
 *
 * The argument _queue_id_ specifies a mail queue identifier that is obtain with \ref osMailCreate.
 * The argument _millisec_ specifies how long the system waits for a mail slot to become
 * available. While the system waits the tread calling this function is put into the state
 * __WAITING__.
 *
 * The _millisec_ timeout can have the following values:
 *  - When _millisec_ is 0, the function returns instantly.
 *  - When _millisec_ is set to \ref #osWaitForever the function will wait for an infinite time until
 *      a mail slot can be allocated.
 *  - All other values specify a time in millisecond for a timeout.
 *
 * \note The parameter millisec must be 0 for using this function in an ISR.
 *
 * A NULL pointer is returned when no memory slot can be obtained or queue specifies an illegal
 * parameter.
 *
 *  \note MUST REMAIN UNCHANGED: \b osMailAlloc shall be consistent in every CMSIS-RTOS.
 */
ATTRIBUTES_osMailAlloc void *osMailAlloc (osMailQId queue_id, uint32_t millisec); /* #001 */
 
/* #016 */
/**
 * \brief Allocate a memory block from the mail queue that is filled with the mail information.
 *   The memory block returned is __initialized with zeroes__.
 *   Can be called from ISR.
 *
 * \copydetails osMailAlloc (osMailQId queue_id, uint32_t millisec)
 */
ATTRIBUTES_osMailCAlloc void *osMailCAlloc (osMailQId queue_id, uint32_t millisec); /* #001 */
 
/* #016 */
/**
 * \brief Put the memory block specified with mail into the mail queue specified by queue.
 *    Can be called from ISR.
 * @param [in] queue_id Mail queue ID obtained with \ref osMailCreate.
 * @param [in] mail Memory block previously allocated with \ref osMailAlloc or \ref osMailCAlloc.
 *
 * @return status code that indicates the execution status of the function.
 *
 * \ref osStatus_ "Status and Error Codes"
 *    - _osOK_: The message is put into the queue.
 *    - _osErrorValue_: Mail was previously not allocated as memory slot.
 *    - _osErrorParameter_: a parameter is invalid or outside of a permitted range.
 *
 *  \note MUST REMAIN UNCHANGED: \b osMailPut shall be consistent in every CMSIS-RTOS.
 */
ATTRIBUTES_osMailPut osStatus osMailPut (osMailQId queue_id, void *mail); /* #001 */
 
/* #016 */
/**
 * \brief Get a mail from a queue.
 *   Can be called from ISR.
 * @param[in]     queue_id      mail queue ID obtained with \ref osMailCreate.
 * @param[in]     millisec      timeout value or 0 in case of no time-out
 * @return event that contains mail information or error code.
 *
 * Suspend the execution of the current __RUNNING__ thread until a mail arrives. When a mail is
 * already in the queue, the function returns instantly with the mail information.
 *
 * The argument _millisec_ specifies how long the system waits for a mail to arrive. While the
 * system waits the thread that is calling this function is put into the state __WAITING__. The
 * _millisec_ timeout can have the following values:
 *   - When _millisec_ is 0, the function returns instantly.
 *   - When _millisec_ is set to \ref #osWaitForever the function will wait for an infinite time until a mail arrives.
 *   - All other values specify a time in millisecond for a timeout.
 *
 * \ref osStatus_ "Status and Error Codes"
 *    -_osOK_: No mail is available in the queue and no timeout was specified.
 *    -_osEventTimeout_: No mail has arrived during the given timeout period.
 *    -_osEventMail_: Mail received, value.p contains the pointer to mail content.
 *    -_osErrorParameter_: A parameter is invalid or outside of a permitted range.
 *
 * \note MUST REMAIN UNCHANGED: \b osMailGet shall be consistent in every CMSIS-RTOS.
 */
ATTRIBUTES_osMailGet osEvent osMailGet (osMailQId queue_id, uint32_t millisec); /* #001 */
 
/* #016 */
/**
 * \brief Free the memory block specified by mail and return it to the mail queue.
 *    Can be called from ISR.
 * @param[in]     queue_id      mail queue ID obtained with \ref osMailCreate.
 * @param[in]     mail          pointer to the memory block that was obtained with \ref osMailGet.
 *
 * \return status code that indicates the execution status of the function.
 *
 * \ref osStatus_ "Status and Error Codes"
 *   - _osOK_: The mail block is released.
 *   - _osErrorValue_: Mail block does not belong to the mail queue pool.
 *   - _osErrorParameter_: The value to the parameter queue_id is incorrect.
 *
 * \note MUST REMAIN UNCHANGED: \b osMailFree shall be consistent in every CMSIS-RTOS.
 */
ATTRIBUTES_osMailFree osStatus osMailFree (osMailQId queue_id, void *mail); /* #001 */

#ifndef ATTRIBUTES_osMailDestroy_suppl
#define ATTRIBUTES_osMailDestroy_suppl
#endif
#ifndef ATTRIBUTES_osMailBlockSizeGet_suppl
#define ATTRIBUTES_osMailBlockSizeGet_suppl
#endif

/**
 * \brief Delete a mail queue.
 *
 * There are 2 case when a mail queue cannot be deleted:
 *
 * - When the memory pool of this mail queue has still memory blocks in use after the timeout time.
 * - When another thread has already started the destruction of this mail queue.
 *
 * \note All threads that are waiting for the deleted mail queue will be woken with an
 * osErrorResource status. That means their osMailGet call will return with an
 * osErrorResource status.
 *
 * \warning A pointer to the queue id is passed as argument to that function. When the function
 * returns successfully, the queue id that this pointer is pointing to, is set to zero. Otherwise
 * it stays untouched. This ensures that, that the queue id is invalidated, when the queue became
 * invalid. However, there might exist multiple clones of the queu id, that still refer to the
 * invalid destroyed queue. Those clones must be reseted to zero manually.
 *
 * \return
 *  - osOK in case of success
 *  - osErrorResource, if an other thread is already is already destroying the mail queue
 *  - osErrorTimeoutResource, if there is still a memory block in use after the timeout period.
 */
ATTRIBUTES_osMailDestroy_suppl osStatus osMailDestroy_suppl(
   osMailQId* queue_id_ptr /**< [in] [out] Pointer to the mail queue to be deleted. */
  ,uint32_t millisec       /**< timeout. */
  );


/**
 * \brief Remove and free all mails that are currently in the mail queue.
 * @param[in]     queue_id      mail queue ID obtained with \ref osMailCreate.
 */
C_FUNC void osMailClear_suppl(osMailQId queue_id);


/**
 * \brief Retrieve the item size of a mail queue.
 */
ATTRIBUTES_osMailBlockSizeGet_suppl uint32_t osMailBlockSizeGet_suppl(osMailQId queue_id);

/* #017 Start */
/* @} */
/* #017 End */

#endif  // Mail Queues available


#ifdef  __cplusplus
}
#endif

/*************************** Additional extension APIs ***********************/

/* #019 Start */
#if (defined (osFeature_Pool)  &&  (osFeature_Pool != 0))  // Memory Pool Management available

/* #019 End */


#endif /* #if (defined (osFeature_Pool)  &&  (osFeature_Pool != 0))  */


#endif  // _CMSIS_OS_H
