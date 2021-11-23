/*
 *  $HeadURL: $
 *
 *  $Date: $
 *  $Author: $
 */

#ifndef cmsis_osMailQueue_H_
#define cmsis_osMailQueue_H_

#if defined(__cplusplus)


#include "baseplate.h"
#include "rtos/cmsis-rtos/cmsis_os_redirect.h"
#include "boards/board-api/bapi_atomic.h"

/**
 * \file
 * \brief This file implements the class os::MailQueue.
 */


namespace os {
/**
 * \ingroup cmsis_os_cpp
 * \brief This class wraps the CMSIS osMail interface into a class with typed
 * items.
 *
 * \note The wrapper does not generate code overhead, because it uses inline
 * functions for wrapping.
 */
template< typename T > class MailQueue {
public:
  typedef T ItemType;

private:
  osMessageQueueId_t m_mailQId;

  static osStatus_t _getAndFree(osMessageQueueId_t mailQId, ItemType* pItem, MsecType msecBlockTime) {
    //osEvent event = osMailGet(mailQId, msecBlockTime);
    //if(event.status == osEventMail) {
    //  ASSERT(event.value.p);

      /* Using assignment instead of memcpy ensures that an eventually
       * overloaded assignment operator is called.*/
    //   *pItem = *static_cast<ItemType*>(event.value.p);
    //   osMailFree(mailQId, event.value.p);
    //}
    //return event;

    osStatus_t retStatus = osMessageQueueGet(mailQId, pItem, NULL, msecBlockTime);
    return retStatus;
  }

  static inline osStatus_t _get(osMessageQueueId_t mailQId, ItemType* pItem, MsecType msecBlockTime) {
    //osEvent event = osMailGet(mailQId, msecBlockTime);
    //if(event.status == osEventMail) {
      //return static_cast<ItemType*>(event.value.p);
    //}
    osStatus_t retStatus;
    retStatus = osMessageQueueGet(mailQId, pItem, NULL, msecBlockTime);

    return retStatus;
  }


public:

/*
 * IAR language provider workarounds. IAR has problems with size_t
 *
 * Note: The cmake macro _create_language_provider_ invokes the compiler for the indexer with -D_eclipse_LANGUAGE_PROVIDER__
 *
 */
#if defined(__IAR_SYSTEMS_ICC__) && defined(_eclipse_LANGUAGE_PROVIDER__)
  typedef unsigned size_t;
#endif

  /**
   * \brief Construct a Mail Queue object without creating the underlying osMailQ
   * object. */
  inline MailQueue() : m_mailQId(0) {
  }

  /**
   * Construct a MailQueue object and create the underlying osMailQ object.
   * */
  inline MailQueue(size_t queueSize, const char* name = 0) : m_mailQId(0) {
    create(queueSize, name);
  }

  /*
   * \brief create the underlying osMailQ object, if not already done at
   * construction.
   */
  inline osStatus_t create(size_t queueSize, const char* name = 0) {
    osStatus_t retval = osErrorResource;

    /* Ensure that no other thread takes over and creates a second
     * mail queue. That would lead to a memory leak, because one of
     * either created mail queues would not be saved in the m_mailQId
     * member and would be lost forever. */
    //osThreadSuspendAll_suppl();
    osKernelLock();

    if(!m_mailQId) {
      //const osMailQDef_t mailQDef = {queueSize, sizeof(ItemType), name};
      //m_mailQId = osMailCreate(&mailQDef, 0);
      const osMessageQueueAttr_t mssgQAtt = {name, 0, NULL, 0, NULL, 0};
      m_mailQId = osMessageQueueNew(queueSize, sizeof(ItemType), &mssgQAtt);
      if(m_mailQId) {
        retval = osOK;
      }
    }

    //osThreadResumeAll_suppl();
    osKernelUnlock();
    return retval;
  }

  bool isCreated()const {
    return m_mailQId != 0;
  }

  inline osStatus_t destroy(uint32_t millisec) {
    //return osMailDestroy_suppl(&m_mailQId, millisec);
    return osMessageQueueDelete(m_mailQId);
  }

  /* It is recommended to call the destroy(uint32_t millisec) method before
   * calling the destructor. That allows careful evaluation of the result
   * of the destruction try. */
  inline ~MailQueue() {
    ASSERT(!m_mailQId); /* Please call destroy(uint32_t millisec) first. */
    destroy(osWaitForever);
  }

  inline ItemType* allocate() {
    //return static_cast<ItemType*>(osMailCAlloc(m_mailQId, 0));
    return NULL;
  }

  inline osStatus_t freeItem(ItemType* mail) {
    //return osMailFree(m_mailQId, mail);
    return osOK;
  }

  inline osStatus_t put(const ItemType* mail) {
    //return osMailPut(m_mailQId, mail);
    //return osMessageQueuePut(m_mailQId, mail, 0, osWaitForever ); //TODO - is "osWaitForever" correct ?????
    return osMessageQueuePut(m_mailQId, mail, 0, 0 );
  }

  inline osStatus_t get(ItemType* pItem, MsecType msecBlockTime) {
    return _get(m_mailQId, pItem, msecBlockTime);
  }

  inline osStatus_t allocateAndPut(ItemType* pItemToQueue) {
    //ItemType* mail = static_cast<ItemType*>(osMailCAlloc(m_mailQId, 0));
    //if(mail) {
      /* Using assignment instead of memcpy ensures that an eventually
       * overloaded assignment operator is called.*/
    //  *mail = *pItemToQueue;
    //  return osMailPut(m_mailQId, mail);
    //}

    //return osErrorNoMemory;

    return put(pItemToQueue);
  }

  inline osStatus_t getAndFree(ItemType* pItem, MsecType msecBlockTime) {
    return _getAndFree(m_mailQId, pItem, msecBlockTime);
  }
};


} /* namespace osCppWrapper */

#endif /* #if defined(__cplusplus) */

#endif /* #ifndef cmsis_osMailQueue_H_ */

