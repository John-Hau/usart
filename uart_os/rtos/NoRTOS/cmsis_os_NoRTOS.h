/*
 *  $HeadURL: $
 *
 *  $Date: $
 *  $Author: $
 */


/**
 * \file
 * \brief
 * This file declares the cmsis os API parts that are available under NoRTOs
 * Those are currently 
 */
 
#ifndef _CMSIS_OS_NORTOS_H
#define _CMSIS_OS_NORTOS_H

#include "baseplate.h"
#include "utils/utils.h"

#define osFeature_MainThread   0       ///< main thread      1=main can be thread, 0=not available
#define osFeature_Pool         1       ///< Memory Pools:    1=available, 0=not available
#define osFeature_MailQ        1       ///< Mail Queues:     1=available, 0=not available
#define osFeature_MessageQ     0       ///< Message Queues:  1=available, 0=not available
#define osFeature_Signals      0       ///< maximum number of Signal Flags available per thread
#define osFeature_Semaphore    0       ///< maximum count for \ref osSemaphoreCreate function
#define osFeature_Wait         1       ///< osWait function: 1=available, 0=not available
#define osFeature_SysTick      1       ///< osKernelSysTick functions: 1=available, 0=not available

#define ATTRIBUTES_osextApplicationDefine   C_FUNC
/* #025 Start */
#define ATTRIBUTES_osThreadSuspend_suppl       C_INLINE
#define ATTRIBUTES_osThreadResume_suppl        C_INLINE
/* #025 End */
#define ATTRIBUTES_osThreadSuspendAll_suppl    C_INLINE
#define ATTRIBUTES_osThreadResumeAll_suppl     C_INLINE
#define ATTRIBUTES_osKernelInitialize          C_INLINE
#define ATTRIBUTES_osKernelSysTick          C_INLINE
#define ATTRIBUTES_osKernelMilliSecSysTick_suppl          C_INLINE
#define ATTRIBUTES_osextMailDelete          C_FUNC
#define ATTRIBUTES_osextMailItemSizeGet     C_FUNC
#define ATTRIBUTES_osThreadGetId            C_INLINE
#define ATTRIBUTES_osThreadGetPriority      C_INLINE
#define ATTRIBUTES_osThreadSetPriority      C_INLINE
#define ATTRIBUTES_osThreadYield         C_INLINE
#define ATTRIBUTES_osPoolDestroy_suppl         C_FUNC
#define ATTRIBUTES_osPoolBlockSizeGet_suppl    C_FUNC
#define ATTRIBUTES_osDelay                     C_FUNC
#define ATTRIBUTES_osGetThreadState_suppl   C_INLINE

#define osThreadId int                  ///< map thread id to simple integer

#include "../cmsis-rtos/cmsis_os_original.h"
typedef uint16_t MemPoolIndex_t;


/******************************************************************************
 *  Internal
 *****************************************************************************/
#define osAssertHandle(expr) ASSERT(expr)

/******************************************************************************
 *  Thread
 *****************************************************************************/
osStatus osKernelInitialize() {
  /* Do nothing if we don't have an RTOS */
  return osOK;
}

void osThreadSuspendAll_suppl() {
  /* Do nothing if we don't have an RTOS */
  return;
}

void osThreadResumeAll_suppl() {
  /* Do nothing if we don't have an RTOS */
  return;
}

uint32_t osKernelSysTick () {
  return bapi_getSystemTick();
}

/**
 * \ingroup _cmsis_os
 * \brief Retrieve the system tick in milliseconds
 */
C_INLINE MsecType _osTickRateMs() {
  return S_CAST(MsecType, 1000) / osKernelSysTickFrequency;
}

osThreadId osThreadGetId() {
  return 1;
}

osPriority osThreadGetPriority(osThreadId UNUSED( thread_id )) {
  return osPriorityNormal;
}

osStatus osThreadSetPriority(osThreadId UNUSED( thread_id ), osPriority UNUSED( priority )) {
  return osOK;
}

osStatus osThreadYield() {
  return osOK;
}

osThreadState osGetThreadState_suppl (osThreadId UNUSED(thread_id) ) {
  return osThreadReady;
}	

/* #025 Start */
osStatus osThreadSuspend_suppl(osThreadId thread_id) {
  /* Stub implementation: Cannot suspend threads in a non rtos environment. */
  return osErrorOS;
}

osStatus osThreadResume_suppl(osThreadId thread_id) {
  /* Stub implementation: Cannot resume threads in a non rtos environment. */
  return osErrorOS;
}
/* #025 End */

/******************************************************************************
 *  Generic Wait
 *****************************************************************************/

/**
 * \ingroup cmsis_os_ext
 * \brief convert milliseconds to system ticks.
 *
 * Example: configTICK_RATE_HZ = 100. -> MSEC_PER_TICK = 10
 * msec         macro result
 *   0                0 (ticks)
 *   1..10            1 (tick)
 *  11..20            2 (ticks)
 *  21..30            3 (ticks)
 * */
bapi_SystemTick_t osKernelMilliSecSysTick_suppl( MsecType msec ) {
  return ((msec) == (osWaitForever) ?
    (BAPI_TICK_TYPE_MAX) : (((msec) + _osTickRateMs() - 1) / _osTickRateMs()));
}



#endif  // _CMSIS_OS_NORTOS_H
