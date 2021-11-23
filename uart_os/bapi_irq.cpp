/*
 * bapi_isr.c
 *
 *  Created on: 10.04.2013
 *      Author: e673505
 */

/** \file
 * \brief
 * This file implements the board API interface functions/variables that are declared
 * in bapi_irq.h and are common to all hardware boards. Board API interface
 * functions that are board specific, are implemented by bapi_irq_<board name>.c or .cpp.
 * */

#include "baseplate.h"
#include "boards/board-api/bapi_common.h"
#include "boards/board-api/bapi_irq.h"
#include "boards/board-api/bapi_atomic.h"

#include "utils/utils.h"

#if MCU_VENDOR == MCU_VENDOR_SILABS

  #include "em_device.h"

#elif (MCU_VENDOR == MCU_VENDOR_FREESCALE) || (MCU_VENDOR == MCU_VENDOR_NXP)

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

  #include "fsl_device_registers.h"

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif

#else
  #error "Fatal Error: MCU Vendor."
#endif



bool bapi_irq_setPrio(bapi_IRQnType irqNum, unsigned prio) {
  ASSERT(irqNum < NUMBER_OF_INT_VECTORS);

  if((irqNum < NUMBER_OF_INT_VECTORS) &&
    utils::Interval<unsigned, bapi_E_IrqLowestPrio, bapi_E_IrqHighestPrio>::isWithin(prio) ) {

    const unsigned nHighestPrio = S_CAST(unsigned, bapi_E_IrqHighestPrio);
    NVIC_SetPriority( S_CAST(IRQn_Type, irqNum), nHighestPrio - prio /* Translate to ARM - priority numbers. */ );

    return true;
  }
  return false;
}

volatile int32_t _bapi_disableIrqCounter = 0;


#define _BAPI_INTERRUPTS_ENABLED() \
  ((__get_PRIMASK() & 0x00000001) == 0)


bool bapi_irq_enabled() {
  return _BAPI_INTERRUPTS_ENABLED();
}

void bapi_irq_enterCritical() {
#ifdef _DEBUG
  bool interruptsEnabled = _BAPI_INTERRUPTS_ENABLED();
#endif
  __disable_irq();

  ASSERT(!_BAPI_INTERRUPTS_ENABLED());
  ASSERT(_bapi_disableIrqCounter >= 0);

  if(0 == _bapi_disableIrqCounter) {
    /* Assert, that there isn't any other code (e.g. the RTOS), that had
     * disabled interrupts by bypassing the bapi_disableIrqCounter.
     * bapi_irq_enterCritical() is the only function that is allowed
     * to call __disable_irq(). */
#ifdef _DEBUG
    ASSERT(interruptsEnabled == true);
#endif
  }

  ++_bapi_disableIrqCounter;
}


void bapi_irq_exitCritical() {
 ASSERT(_bapi_disableIrqCounter > 0);

#ifdef _DEBUG
 /* Assert, that there isn't any other code (e.g. the RTOS), that had
  * enabled interrupts by bypassing the bapi_disableIrqCounter.
  * bapi_irq_exitCritical() is the only function that is allowed to call
  * __enable_irq().
  */
 ASSERT(_BAPI_INTERRUPTS_ENABLED() == false);
#endif

 if(_bapi_disableIrqCounter == 1) {
   _bapi_disableIrqCounter = 0;
   __enable_irq();
   return;
 }
 --_bapi_disableIrqCounter;
}

bool bapi_irq_isInterruptContext() {
  return __get_IPSR () > 0;
}

STATIC bool _bapi_bNmiOccured = false;

STATIC void (*_bapi_nmiCallback)() = nullptr;

/* Catch NMI. */
C_FUNC void NMI_Handler() {
  _bapi_bNmiOccured = true;
  if(_bapi_nmiCallback) {
    _bapi_nmiCallback();
  }
}

void bapi_irq_clearNmiFlag() {
  _bapi_bNmiOccured = false;
}

bool bapi_irq_getNmiFlag() {
  return _bapi_bNmiOccured;
}

bapi_irq_NmiCallback_t bapi_irq_setNmiCallback(bapi_irq_NmiCallback_t callback) {
  return atomic_PtrReplace(bapi_irq_NmiCallback_t, &_bapi_nmiCallback, callback);
}


/**
 * \ingroup _bapi_systick
 * \brief This is the Board API local system tick counter.
 */
STATIC atomic_uint32_t _bapi_systemTick = 0;


/**
 * \ingroup _bapi_systick
 * \brief This is the Board API system tick callback.
 */
STATIC bapi_systemTickCallback_t _bapi_systemTickCallback = 0;


bapi_systemTickCallback_t bapi_systemTickSetCallback(bapi_systemTickCallback_t systemTickCallback) {
  ASSERT(!bapi_irq_isInterruptContext());

  bapi_irq_enterCritical();

  bapi_systemTickCallback_t retval = _bapi_systemTickCallback;
  _bapi_systemTickCallback = systemTickCallback;

  bapi_irq_exitCritical();

  return retval;
}


C_FUNC bapi_SystemTick_t bapi_getSystemTick() {
  return atomic_Uint32Get(&_bapi_systemTick);
}

// #define _BAPI_TEST_ATOMIC_UINT16_
// #define _BAPI_TEST_ATOMIC_UINT8_

#ifdef _BAPI_TEST_ATOMIC_UINT16_
struct PACKED_ALIGNED(sizeof(uint16_t)) test16_ {
  atomic_uint16_t m_test0;
  atomic_uint16_t m_test1;
};
STATIC struct test16_ test16 = {0, 0};
#endif

#ifdef _BAPI_TEST_ATOMIC_UINT8_
struct PACKED_ALIGNED(sizeof(uint8_t)) test8_ {
  atomic_uint8_t m_test0;
  atomic_uint8_t m_test1;
};
STATIC struct test8_ test8 = {0, 0};
#endif

#if (TARGET_RTOS != RTOS_NoRTOS)
void bapi_SysTick_Handler() {
#else
/**
 * \brief Implementation of the System Tick Handler ISR
 */
C_FUNC void SysTick_Handler() {
#endif

#ifndef USE_ARM_LDREX_STREX
  if( MCU_CORE_BYTE_WIDTH >= sizeof(_bapi_systemTick)) {
    /* No other task or ISR will increment the system tick. So we can
     * use here a normal ++ statement if it is a MCU native atomic
     * operation.
     * */
    ++_bapi_systemTick;
  } else {
    /* The system tick type is bigger than the native MCU byte width.
     * So we must ensure, that on other interrupt reads the system
     * tick while it is only partly written to memory after increment.
     * */
    atomic_Uint32Add(&_bapi_systemTick, 1);
  }
#else
  /* We have ARM LDREX and STREX available that is the preferred
   * solution here. */
  atomic_Uint32Add(&_bapi_systemTick, 1);
#endif

#ifdef _BAPI_TEST_ATOMIC_UINT16_
  {
    uint16_t tmp = test16.m_test0;
    atomic_Uint16Add(&test16.m_test0, -1);
    ASSERT(tmp = test16.m_test0 - 1);

    tmp = test16.m_test1;
    atomic_Uint16Add(&test16.m_test1, 1);
    ASSERT(tmp = test16.m_test1 + 1);
  }
#endif

#ifdef _BAPI_TEST_ATOMIC_UINT8_
  {
    uint8_t tmp = test8.m_test0;
    atomic_Uint8Add(&test8.m_test0, -1);
    ASSERT(tmp = test8.m_test0 - 1);

    tmp = test8.m_test1;
    atomic_Uint8Add(&test8.m_test1, 1);
    ASSERT(tmp = test8.m_test1 + 1);
  }
#endif

//  bapi_systemTickIsrHook(_bapi_systemTick);

  /* Use a temporary variable for the callback, so that _bapi_systemTickCallback can be
   * replaced while this code is running. */
  bapi_systemTickCallback_t callback = _bapi_systemTickCallback;

  if(callback) {
    (*callback)(_bapi_systemTick);
  }
}

