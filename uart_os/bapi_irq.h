/*
 * bapi_irq.h
 *
 *  Created on: 07.04.2013
 *      Author: Wolfgang
 */

/**************************************************************************//**
 * \file
 * \brief This file declares board API interface functions that control
 * interrupt requests.
 *****************************************************************************/

#ifndef BAPI_IRQ_H_
#define BAPI_IRQ_H_

#include "baseplate.h"

#if MCU_VENDOR == MCU_VENDOR_FREESCALE || MCU_VENDOR == MCU_VENDOR_NXP


//
//#ifdef __GNUC__
//  #pragma GCC diagnostic push
//  #pragma GCC diagnostic ignored "-Wunused-parameter"
//#endif
//
//  #include "fsl_device_registers.h"
//
//#ifdef __GNUC__
//  #pragma GCC diagnostic pop
//#endif

/**
 * \ingroup bapi_irq
 * \brief Type of the enumeration for interrupts that are available on the board.
 */
//typedef enum IRQn bapi_IRQnType;
typedef unsigned bapi_IRQnType;

#endif

/*
 * In order to avoid the include of fsl_device_registers.h here in bapi_irq.h,
 * we define the CMSIS core macro __NVIC_PRIO_BITS ourselves here. The include
 * of fsl_device_registers.h here is causing problems. Since
 * fsl_device_registers.h and this file are both included in bapi_irq.c, we
 * will automatically have a compile time check whether our own definitions
 * are correct.
 */
#if   defined( CPU_MKL17Z128VLH4 )
  #define __NVIC_PRIO_BITS 2
#elif defined( CPU_MKL17Z256VLH4 )
  #define __NVIC_PRIO_BITS 2
#elif defined( CPU_MKL46Z256VLL4 )
  #define __NVIC_PRIO_BITS 2
#elif defined( CPU_MK24FN1M0VLQ12 )
  #define __NVIC_PRIO_BITS 4
#elif defined( CPU_MK64FN1M0VLL12 )
  #define __NVIC_PRIO_BITS 4
#elif defined( CPU_MK66FN2M0VMD18 )
  #define __NVIC_PRIO_BITS 4
#elif defined( CPU_MIMXRT1062CVL5A)
  #define __NVIC_PRIO_BITS 4
#elif defined( CPU_MIMXRT1051CVJ5B)
  #define __NVIC_PRIO_BITS 4
#else
  #error "__NVIC_PRIO_BITS not defined for the MCU, please add."
#endif


/**
 * \ingroup bapi_irq
 * \brief Upper and lower limits for the valid priorities that can be passed
 *    to \ref bapi_irq_setPrio.
 *
 * \warning This is not an ARM like approach, The bapi layer takes higher
 *   priorities as higher numbers. Depending on the MCU type (ARM or not)
 *   it will be translated accordingly.
 */
enum{
  bapi_E_IrqLowestPrio  = 0,
  bapi_E_IrqHighestPrio = (1 << __NVIC_PRIO_BITS) - 1,
  bapi_E_IrqPriorityCount = (bapi_E_IrqHighestPrio - bapi_E_IrqLowestPrio + 1)
};

/**
 * \ingroup bapi_irq
 * \brief A global boolean NMI flag is set to true, when the non maskable interrupt has appeared.
 *   This function clears the NMI flag.
 */
C_FUNC void bapi_irq_clearNmiFlag();

/**
 * \ingroup bapi_irq
 * \brief A global boolean NMI flag is set to true, when the non maskable interrupt has appeared.
 *   This function queries the NMI flag.
 *   @return The NMI flag;
 */
C_FUNC bool bapi_irq_getNmiFlag();

typedef void (*bapi_irq_NmiCallback_t)();
/**
 * \ingroup bapi_irq
 * \brief Install a callback that will be called (within the NMI context) when an NMI happens.
 * @return The old callback the was installed before.
 */
bapi_irq_NmiCallback_t bapi_irq_setNmiCallback(bapi_irq_NmiCallback_t callback);

/**
 * \ingroup bapi_irq
 * \brief This is the counter that counts the nesting level of bapi_irq_enterCritical() calls.
 * See also function bapi_irq_exitCritical().
 */
extern volatile int32_t  _bapi_disableIrqCounter;

/**
 * \ingroup bapi_irq
 * \brief Retrieve whether interrupts are enabled.
 * \return true, if interrupts are enabled, otherwise false
 */
C_FUNC bool bapi_irq_enabled();

/**
 * \ingroup bapi_irq
 * \brief
 * Ensures, that no interrupt will happen, after this function returns.
 * The purpose of this function is to allow execution of some
 * processing instructions atomically. Each bapi_irq_enterCritical() call
 * must have a corresponding bapi_irq_exitCritical() call. Otherwise
 * all interrupts stay disabled for ever. E.g. if bapi_irq_enterCritical() was
 * called 3 times, bapi_irq_exitCritical() has also to be called 3 times
 * in order to effectively re-enable interrupts.
 *
 * \return void
 * */
C_FUNC void bapi_irq_enterCritical();

/**
 * \ingroup bapi_irq
 * \brief
 * Withdraws the request for interrupt suppression that was initiated by
 * calling bapi_irq_enterCritical(). Note: Each bapi_irq_enterCritical() call
 * must have a corresponding call of bapi_irq_exitCritical(), before interrupts
 * will effectively be enabled.
 * */
C_FUNC void bapi_irq_exitCritical();

 /**
  * \ingroup bapi_irq
  * \brief Retrieves whether the current runtime context is an Interrupt Service Routine.
  * @return true, if the current runtime context is an ISR, otherwise false.
  */
C_FUNC bool bapi_irq_isInterruptContext();


/**
 * \ingroup bapi_irq
 * \brief Sets the priority for a particular interrupt.
 * @param irqNum The interrupt for which to set the priority.
 * @param prio The priority within the range of  [bapi_E_IrqLowestPrio..bapi_E_IrqHighestPrio].
 *    \note: _low numbers_ mean _low priority_, _high numbers_ mean _high priority_.
 * @return true, if successful, false if the priority is out of range.
 */
C_FUNC bool bapi_irq_setPrio(bapi_IRQnType irqNum, unsigned prio);


#endif /* #ifndef BAPI_IRQ_H_ */
