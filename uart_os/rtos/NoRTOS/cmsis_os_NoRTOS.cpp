/*
 *  $HeadURL: $
 *
 *  $Date: $
 *  $Author: $
 */

/**
 * \brief
 * This file implements the osMail cmsis API on NoRTOS
 */

#include "rtos//cmsis-rtos/cmsis_os.h"

/******************************************************************************
 *  Internal
 *****************************************************************************/

namespace _os {
/**
 * \ingroup _cmsis_os
 * \brief
 * A structure that provides a single system tick wait as a static function.
 * It used required to implement the memory pool alloc function that has a wait
 * option.
 *
 * This implementation doesn't really wait a single tick in this Non-
 * RTOS environment, but the memory pool implementation expects
 * that this structure does exist.
 */
struct WaitTickProvider {
  static MsecType waitSingleTick(MsecType UNUSED(msec)) {
    return 0; /* We cannot wait here because there is no RTOS. */
  }
};
}

/******************************************************************************
 *  Wait Function (Does a spin around)
 *****************************************************************************/
osStatus osDelay(uint32_t millisec) {
  uint32_t firstTick = osKernelSysTick();
  uint32_t delta = 0;

  /* Just spin around until time expired */
  do {
    delta = osKernelSysTick() - firstTick;
  } while((delta * 1000 / osKernelSysTickFrequency) < millisec);
  return osOK;
}

/******************************************************************************
 *  Memory Pool
 *****************************************************************************/

#if (defined (osFeature_Pool)  &&  (osFeature_Pool != 0))  // Memory Pool Management available

#include "rtos/cmsis-rtos/internal/mem_pool.hpp"
/**
 * \ingroup _cmsis_os
 *
 * \brief Define the os_mailQ_cb structure that is declared in the cmsis_os.h. It is an
 * instantiation of the MemoryPool helper.
*/
struct os_pool_cb : public _os::MemoryPool<_os::WaitTickProvider, MemPoolIndex_t> {
  typedef MemoryPool<_os::WaitTickProvider, MemPoolIndex_t> base_class_t;

  inline static os_pool_cb* create(index_type maxBlocks, uint32_t blockSize) {
    return S_CAST(os_pool_cb*, base_class_t::create(maxBlocks, blockSize));
  }

  void _closeDestruction(os_pool_cb* *const enclosure, bool success) {
    base_class_t::destructor::closeDestruction(this, R_CAST(base_class_t* *const, enclosure), success);
  }
};

#include "../cmsis-rtos/internal/cmsis_osPool.inc"

#endif /* #if (defined (osFeature_Pool)  &&  (osFeature_Pool != 0)) */



/******************************************************************************
 *  Mail Queue
 *****************************************************************************/

#if (defined (osFeature_MailQ)  &&  (osFeature_MailQ != 0))     // Mail Queues available

#include "rtos/NoRTOS/atomic_queue.hpp"

/**
 * \ingroup _cmsis_os
 *
 * The Queue type to be used for the helper class _osMailPool to implement os_mailQ_cb
 * */
typedef _os::UntypedAtomicQueue<MemPoolIndex_t> queue_t;

#include "rtos/cmsis-rtos/internal/mail_queue.hpp"
/**
 * \ingroup _cmsis_os
 *
 * Define os_mailQ_cb structure that is declared in the cmsis_os.h. It is an
 * instantiation of the _osMailPool helper with a NoRTOS::UntypedAtomicQueue as the
 * queue type.
*/
struct os_mailQ_cb : public _os::MailQueue<_os::WaitTickProvider, queue_t, os_pool_cb> {
  typedef _os::MailQueue<_os::WaitTickProvider, queue_t, os_pool_cb> base_class_t;

  inline static os_mailQ_cb* create(index_type maxItems, size_t itemSize) {
    return S_CAST(os_mailQ_cb*, base_class_t::create(maxItems, itemSize));
  }

  void _closeDestruction(os_mailQ_cb* *const enclosure, bool success) {
    base_class_t::destructor::closeDestruction(this, R_CAST(base_class_t* *const, enclosure), success);
  }
};

#include "rtos/cmsis-rtos/internal/cmsis_osMail.inc"

#endif /* #if (defined (osFeature_MailQ)  &&  (osFeature_MailQ != 0)) */
