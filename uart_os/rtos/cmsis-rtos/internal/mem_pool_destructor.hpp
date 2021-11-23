/*
 *  $HeadURL: $
 *
 *  $Date: $
 *  $Author: $
 */

#ifndef mem_pool_destructor_HPP_
#define mem_pool_destructor_HPP_

#include "baseplate.h"

/**
 * \file
 * \brief
 * Implements the cmsis RTOS extension internal structure _os::MemoryPoolDestructor.
 */

namespace _os {

/**
 * \ingroup _cmsis_os
 * \brief This structure helps to use common memory pool destruction code for ThreadX, FreeRTOS, and NoRTOS
 *
 * It assumes that the memory pool has a member variable m_destroyingThread, and that
 * the allocate function of the memory pool does not allocate while the m_destroyingThread
 * is non-zero. The allocate function must use the function MemoryPoolDestructor::isDestructing()
 * to check this condition.
 *
 * Furthermore it is assumed, that the memory pool has a function destroy(MsecType millisec, bool bClose).
 * This function must wait at maximum for millisec milliseconds for the memory pool to become
 * empty (no block used). In any case, this destroy function must call the function
 * closeDestruction(PoolId pool_id, PoolId *const enclosure, bool success), if the bClose
 * parameter was true. The enclosure parameter must be set to zero, and the success parameter
 * must be set to true, in case that the pool successfully became empty, otherwise (on time out)
 * to false. The return value of destroy(MsecType millisec, bool bClose) must be the same as
 * this success parameter.
 * The destroy(MsecType millisec, bool bClose) function shall call the
 * assertDestroyingThread(const PoolId pool_id) to assert that the current thread is authorized
 * to call that function. Autorization must be obtained by the calling first the function
 * openDestruction(PoolId pool_id), which will set the m_destroyingThread variable if zero.
 * Otherwise, another thread is already performing the destruction, and the function
 * openDestruction(PoolId pool_id) will return with false.
 *
 * The function poolDestroy(PoolId *const pool_id_ptr, uint32_t millisec) does all the above an
 * can be used to destroy a memory pool.
 *
 */
template<typename MEMORY_POOL_TYPE> struct MemoryPoolDestructor {

  typedef MEMORY_POOL_TYPE* PoolId;
  typedef MEMORY_POOL_TYPE  pool_cb;

  /**
   * Must be called after the destroy method with the return value of the destroy method. In case
   * the destroy method failed this method will withdraw the reservation of the
   * destroy(MsecType millisec, bool bClose) call for a particular thread and will re-allow block allocation.
   */
  static void closeDestruction(PoolId pool_id, PoolId *const enclosure, bool success) {
    ASSERT(pool_id->m_destroyingThread == osThreadGetId());
    if(success) {
      if(enclosure) {
        *enclosure = 0;
      }
      free(pool_id);
    }
    else {
      pool_id->m_destroyingThread = 0;
    }
  }

  /**
   * Must be called before calling the destroy method, which will exclusively reserve a following
   * destroy(MsecType millisec, bool bClose) call for the currently running thread. It will also disable any
   * block allocation from the pool.
   *
   * \return true, if destroy(MsecType millisec) can be called. false, if another
   * thread is already performing the destruction.
   */
  static bool openDestruction(PoolId pool_id) {
    bool retval = false;
    bapi_irq_enterCritical();
    if(pool_id) {
      if(!pool_id->m_destroyingThread) {
        pool_id->m_destroyingThread = osThreadGetId();
        retval = true;
      }
    }
    bapi_irq_exitCritical();
    return retval;
  }

  /**
   * Assert that the currently cunning thread is the thread which
   * has opened the destruction vio openDestruction.
   */
  static bool assertDestroyingThread(const PoolId pool_id) {
    bool retval = false;
    bapi_irq_enterCritical();
    retval = pool_id && (pool_id->m_destroyingThread == osThreadGetId());
    bapi_irq_exitCritical();
    return retval;
  }

  /**
   * \Retrieve whether a memory pool is in destruction mode.
   *
   * Destruction mode means, that there is the destroying thread currently waiting
   * for the memory pool to become empty, or that the memory pool has already been
   * destroyed and de-allocated.
   *
   * \return true, if the memory pool is in destruction mode, otherwise false.
   */
  static bool isDestructing(const PoolId pool_id) {
    bool retval = false;
    bapi_irq_enterCritical();
    if(!pool_id || pool_id->m_destroyingThread) {
      retval = true;
    }
    bapi_irq_exitCritical();
    return retval;
  }

  /**
   * \copybrief osPoolDestroy_suppl
   * \copydoc osPoolDestroy_suppl
   */
  static inline osStatus poolDestroy(
    PoolId *const pool_id_ptr,
    uint32_t millisec)
  {
    osStatus retval = osErrorResource;
    osAssertHandle(pool_id_ptr);
    if(openDestruction(*pool_id_ptr)) {
      /* Destroy without closing the destruction. */
      if((*pool_id_ptr)->destroy(millisec, false)) {
        /* Close the destruction manually. */
        closeDestruction(*pool_id_ptr, pool_id_ptr, true);
        /* pool_id is invalid now ! */
        retval = osOK;
      } else {
        retval = osErrorTimeoutResource;
      }
    }
    return retval;
  }
};

} /* namespace _os */

#endif /* mem_pool_destructor_HPP_ */
