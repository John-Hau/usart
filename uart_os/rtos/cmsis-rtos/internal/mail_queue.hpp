/*
 *  $HeadURL: $
 *
 *  $Date: $
 *  $Author: $
 */


#include "build-config.h" /* This must be the first include. It will pull the TARTGET_RTOS macro. */

#ifndef mail_queue_H_
#define mail_queue_H_

#if TARGET_RTOS != RTOS_ThreadX /* For ThreadX use a separate implementation, that utilizes the ThreadX pool. */

#include "baseplate.h"
#ifdef __IAR_SYSTEMS_LIB__
  #include <stdlib.h>
#else
  #include <malloc.h>
#endif
#include <new>

#include "boards/board-api/bapi_irq.h"
#include "boards/board-api/bapi_atomic.h"

#include "mem_pool.hpp"
#include "mail_queue_destructor.hpp"

/**
 * \file
 * \brief
 * Implements the cmsis RTOS extension internal structure _os::MailQueue.
 */


namespace _os {

/**
 * \ingroup _cmsis_os
 * \brief
 * A template class that implements Mail Queues as per CMSIS RTOS.
 * As an extension to the CMSIS RTOS API, a graceful destruction feature is supported either.
 *
 * This class is a container for the queue of mails as well as the memory pool that is associated with the
 * Mail Queue concept of CMSIS RTOS. The queue consists of memory block indexes of the memory pool.
 * This template class can be used by any OS that wants to implement the CMSIS RTOS mail queues and does
 * not come along with an own queue and memory pool artifact.
 * In order to utilize this class, a queue type with a same thread/ISR safe - interface
 * like NoRTOS::UntypedAtomicQueue must be provided as template parameter.
 *
 * \note C++ standard container classes are not used for implementation, because of the required
 * determinism and thread/ISR safety.
 *
 * TODO: Optimize pool management for performance?
 *
*/
template<typename WAIT_TICK_PROVIDER, typename queue_t, typename MEMORY_POOL_TYPE> struct MailQueue {

  template<typename, typename> friend struct MailQueueDestructor;
  typedef MEMORY_POOL_TYPE pool_type;
  typedef typename queue_t::index_type index_type;
  typedef MailQueueDestructor<pool_type, MailQueue> destructor;

private:

  typedef struct MemoryPoolDestructor<pool_type> memPoolDestructor;

  /* queue_t must be thread/ISR safe ! */
  queue_t m_atomicQueue;

  pool_type* m_pool;

  inline index_type size()const {
    return m_atomicQueue.size();
  }

  inline void sendAbortMessage() {
    index_type abortItemindex = m_pool->maxBlocks();
    bool result = m_atomicQueue.pushBack(&abortItemindex, osWaitForever);

    /* Assert thet there was space for the abort message, because
     * we create the queue one entry bigger than the pool. */
    ASSERT(result);
  }

  /**
   * No destructor call allowed. Call openDestruction() and
   * destroy(MsecType millisec, bool bClose) instead.
   */
  ~MailQueue();

  /**
   * \brief
   * The constructor is private, because it must be ensured that MailQueue objects
   * only exit on the heap. This is required to support destruction of a mail
   * queue. closeDestruction(mail_queue_id *const enclosure, bool success) will call
   * ::free() in case of a destroy(MsecType millisec, bool bClose).
   * Hence the static create(index_type maxItems, uint32_t itemSize) function must
   * be used instead or a direct construction via new. */
  MailQueue(index_type maxItems, size_t itemSize) : m_pool(0) {
    /* Create the queue with the indexes to items in the pool.
     * Create one additional queue item than requested, in
     * order to have space in the queue for an abort message.
     */
    if(m_atomicQueue.create(maxItems + 1, sizeof(index_type))) {
      m_pool = pool_type::create(maxItems, itemSize);
      if(!m_pool) {
        m_atomicQueue.destroy();
      }
    }
  }

  inline void mailQDelete() {
    m_atomicQueue.destroy();
  }

public:
  static MailQueue* create(index_type maxItems, size_t itemSize) {
    void* place = ::malloc(sizeof(MailQueue));
    if(place) {
      MailQueue* retval = new (place) MailQueue(maxItems, itemSize);
      if(!retval->m_pool) {
        free(place);
        retval = 0;
      }
      return retval;
    }
    return 0;
  }

  inline size_t blockSize()const {
    /* Note that m_pool can be null, because pool_type::blockSize checks if this is null. */
    return m_pool->blockSize();
  }

//  /**
//   * Must be called before calling the destroy method, which will exclusively reserve a following
//   * destroy(MsecType millisec, bool bClose) call for the currently running thread. It will also disable any
//   * block allocation from the pool.
//   * \return true, if destroy(MsecType millisec) can be called. false, if another
//   * thread is already performing the destruction.
//   */
//  bool openDestruction() {
//    bool retval = false;
//    bapi_irq_enterCritical();
//    if(this) {
//      retval = destructor::openDestruction(m_pool);
//      /* From here, no new item will be pushed into the queue anymore, because
//       * the alloc function of the memory pools skips when openDestruction()
//       * was called. There might be one allocated block on the way already, but
//       * this doesn't matter, because we will be waiting anyhow until the
//       * pool becomes empty. */
//    }
//    bapi_irq_exitCritical();
//    return retval;
//  }

//  bool isDestructing()const {
//    bool retval = false;
//    bapi_irq_enterCritical();
//    if(!this || memPoolDestructor::isDestructing(m_pool)) {
//      retval = true;
//    }
//    bapi_irq_exitCritical();
//    return retval;
//  }

//  /**
//   * Must be called after the destroy method with the return value of the destroy method. In case
//   * the destroy method failed this method will withdraw the reservation of the
//   * destroy(MsecType millisec) call for a particular thread and will re-allow block allocation.
//   */
//  void closeDestruction(MailQueue* *const enclosure, bool success) {
//    if(success) {
//      /* The pool was successfully destroyed, so now we destroy the queue as well. */
//      mailQDelete();
//      if(enclosure) {
//        *enclosure = 0;
//      }
//      memPoolDestructor::closeDestruction(m_pool, &m_pool, success);
//      free(this);
//    } else {
//      memPoolDestructor::closeDestruction(m_pool, &m_pool, success);
//    }
//  }

  bool destroy(MsecType millisec, bool bClose) {

    memPoolDestructor::assertDestroyingThread(m_pool);

    osThreadId threadId = osThreadGetId();

    /* Send an abort message with the highest priority. */
    osPriority originalThreadPrio = osThreadGetPriority(threadId);
    osThreadSetPriority(threadId, osPriorityHigh);
    sendAbortMessage();

    /* Give thread(s) that are waiting for this queue a chance
     * to receive the abort mail. */
    osThreadSetPriority(threadId, osPriorityIdle);
    osThreadYield();

    /* Set back to original priority. */
    osThreadSetPriority(threadId, originalThreadPrio);

    /* Try destroying the pool and destroy the queue, if the pool could be
     * destroyed. */
    bool success = m_pool->destroy(millisec, false);

    if(bClose) {
      destructor::closeDestruction(this, 0, success);
    }

    return success;
  }

  inline void* alloc(MsecType msecBlockTime) {
    /* Note that m_pool is allowed to be null here, because
     * MemoryPool::alloc() skips when this is null. */
    return m_pool->alloc(msecBlockTime, size());
  }

  inline void* calloc(MsecType msecBlockTime) {
    /* Note that m_pool is allowed to be null here, because
     * MemoryPool::calloc() skips when this is null. */
    return m_pool->calloc(msecBlockTime, size());
  }

  inline osStatus freeItem(const void* pPoolBlock) {
    /* Assert that we are not using a destroyed pool. */
    ASSERT(m_pool);
    return m_pool->freeBlock(pPoolBlock);
  }

  inline void* popFront(MsecType msecBlockTime, bool* abort) {
    index_type poolBlockIndex = 0;

    /* Assert that we are not using a destroyed pool. */
    ASSERT(m_pool);

    if( m_atomicQueue.popFront(osKernelMilliSecSysTick_suppl(msecBlockTime), &poolBlockIndex) ) {

      /* The pool will not be destroyed while performing
       * the following code, because the block of the
       * received block index is still not yet freed. */
      ASSERT(poolBlockIndex <= m_pool->maxBlocks());

      if (poolBlockIndex < m_pool->maxBlocks()) {
        *abort = false;
        return m_pool->at(poolBlockIndex);
      } else {
        /* A pool index of max_items() indicates an abort. */
        *abort = true;
        return 0;
      }
    }

    /* return upon timeout */
    *abort = memPoolDestructor::isDestructing(m_pool);
    return 0;
  }

  inline bool pushBack(const void* pPoolItem) {
    /* Assert that we are not using a destroyed pool. */
    ASSERT(m_pool);

    /* When we arrive here, the pool item has already been
     * allocated, which will avoid a deletion of m_pool in
     * our destroy(MsecType millisec, bool bClose) function during
     * execution of the following code. */

    /* Assert that the item is really from this pool. */
    ASSERT(pPoolItem < m_pool->at(m_pool->maxBlocks()));
    ASSERT(pPoolItem >= m_pool->at(0));

    index_type poolItemIndex = m_pool->toIndex(pPoolItem);

    ASSERT(poolItemIndex < m_pool->maxBlocks());
    return m_atomicQueue.pushBack(&poolItemIndex, 0);
  }
};

} /* namespace _os */

#endif /* #if TARGET_RTOS != RTOS_ThreadX */
#endif /* #ifndef mail_queue_H_ */

