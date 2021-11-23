/*
 *  $HeadURL: $
 *
 *  $Date: $
 *  $Author: $
 */

#ifndef mail_queue_destructor_HPP_
#define mail_queue_destructor_HPP_

#include "baseplate.h"


/**
 * \file
 * \brief
 * Implements the cmsis RTOS extension internal stucture _os::MailQueueDestructor.
 */

namespace _os {

//template<typename MEMORY_POOL_TYPE> struct MemoryPoolDestructor;

/**
 * \ingroup _cmsis_os
 * \brief This structure helps to use common mail queue destruction code for ThreadX, FreeRTOS and NoRTOS.
 *
 * The MailQueueDestructor assumes, that the mail queue's memory pool is compliant with the
 * _os::MemoryPoolDestructor. It first tries to destroy the mail queue's memory pool by utilizing
 * the _os::MemoryPoolDestructor. Upon success, it destroys the mail queue, assuming that the
 * mail queue provides a method mailQDelete(), that will be certainly successful, when the mail queue's
 * memory pool was successfully destructed up front.
 *
 * Each mail queue destruction must be started with a call of openDestruction(MailQId queue_id).
 * In case that this function call returns false, another thread is already performing
 * a destruction of this mail queue. Otherwise the next step is to call
 * _os::MailQueueDestructor::destroy(MailQId queue_id, MsecType millisec, bool bClose), either with bClose set to true,
 * if the closeDestruction(MailQId queue_id, MailQId *const enclosure, bool success) should be called
 * automatically before return, or set to true, if that call will be done separately after return
 * of that function. The MailQueueDestructor::destroy(MailQId queue_id, MsecType millisec, bool bClose)
 * will return true if successful or false if not. In any of both cases the
 * closeDestruction(MailQId queue_id, MailQId *const enclosure, bool success) __must__ be called, if
 * the bClose parameter of the destroy call was set to true.
 *
 * The function mailDestroy does all the above an can be used to destroy a mail queue.
 *
 */
template<typename MEMORY_POOL_TYPE, typename MAIL_QUEUE_TYPE> struct MailQueueDestructor {

  typedef MAIL_QUEUE_TYPE* MailQId;
  typedef MAIL_QUEUE_TYPE  mailQ_cb;
  typedef MEMORY_POOL_TYPE pool_cb;

  /**
   * Must be called before calling the destroy method, which will exclusively reserve a following
   * destroy(MsecType millisec, bool bClose) call for the currently running thread. It will also disable any
   * block allocation from the pool.
   * \return true, if destroy(MsecType millisec) can be called. false, if another
   * thread is already performing the destruction.
   */
  static bool openDestruction(MailQId queue_id) {
    bool retval = false;
    bapi_irq_enterCritical();
    if(queue_id) {
      retval = MemoryPoolDestructor<pool_cb>::openDestruction(queue_id->m_pool);
    }
    bapi_irq_exitCritical();
    return retval;
  }

  /**
   * \Retrieve whether a mail queue is in destruction mode.
   *
   * Destruction mode means, that there is the destroying thread currently waiting
   * for the mail queue's memory pool to become empty, or that the mail queue's
   * memory pool has already been destroyed and de-allocated, or that the complete
   * mail queue and its memory pool has already been destroyed and de-allocated.
   *
   * \return true, if the memory pool is in destruction mode, otherwise false.
   */
  static bool isDestructing(const MailQId queue_id) {
    bool retval = false;
    bapi_irq_enterCritical();
    if(!queue_id || MemoryPoolDestructor<pool_cb>::isDestructing(queue_id->m_pool)) {
      retval = true;
    }
    bapi_irq_exitCritical();
    return retval;
  }

  /**
   * Must be called after the destroy method with the return value of the destroy method. In case
   * the destroy method failed this method will withdraw the reservation of the
   * destroy(MsecType millisec) call for a particular thread and will re-allow block allocation.
   */
  static void closeDestruction(MailQId queue_id, MailQId *const enclosure, bool success) {
    if(success) {
      /* The pool was successfully destroyed, so now we destroy the queue as well. */
      queue_id->mailQDelete();

      if(enclosure) {
        *enclosure = 0;
      }
      MemoryPoolDestructor<pool_cb>::closeDestruction(queue_id->m_pool, &(queue_id->m_pool), success);
      free(queue_id);
    }
    else {
      MemoryPoolDestructor<pool_cb>::closeDestruction(queue_id->m_pool, &(queue_id->m_pool), success);
    }
  }

  static bool destroy(MailQId queue_id, MsecType millisec, bool bClose) {

    MemoryPoolDestructor<pool_cb>::assertDestroyingThread(queue_id->m_pool);

    /* Try destroying the pool and destroy the queue, if the pool could be
     * destroyed. */
    bool success = queue_id->m_pool->destroy( millisec, false);

    if(bClose) {
      closeDestruction(queue_id, 0, success);
    }

    return success;
  }

  /**
   * \copybrief osMailDestroy_suppl
   * \copydoc osMailDestroy_suppl
   */
  static inline osStatus mailDestroy(MailQId *const queue_id_ptr, uint32_t millisec) {
    osStatus retval = osErrorResource;
    osAssertHandle(queue_id_ptr);
    if(openDestruction(*queue_id_ptr)) {
      /* Destroy without closing the destruction. */
      if(destroy(*queue_id_ptr, millisec, false)) {
        /* Close the destruction manually. */
        closeDestruction(*queue_id_ptr, queue_id_ptr, true);
        /* queue_id is invalid now */
        retval = osOK;
      } else {
        retval = osErrorTimeoutResource;
      }
    }
    return retval;
  }
};

} /* namespace _os */

#endif /* mail_queue_destructor_HPP_ */
