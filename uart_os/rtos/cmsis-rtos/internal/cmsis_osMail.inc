/*
 *  $HeadURL: $
 *
 *  $Date: $
 *  $Author: $
 */

/** 
 * \file
 * \brief
 * Implements the cmsis RTOS mail queue API.
 */

#include "build-config.h" /* This must be the first include. It will pull the TARTGET_RTOS macro. */

#if TARGET_RTOS != RTOS_ThreadX /* For ThreadX we use a mail queue implementation that utilizes its own pool. */

uint32_t osMailBlockSizeGet_suppl(osMailQId queue_id) {
  osAssertHandle(queue_id);
  return queue_id->blockSize();
}

osMailQId osMailCreate (const osMailQDef_t *queue_def, osThreadId thread_id) {
  return os_mailQ_cb::create(queue_def->queue_sz, queue_def->item_sz);
}

void *osMailAlloc (osMailQId queue_id, uint32_t millisec) {
  osAssertHandle(queue_id);
  return queue_id->alloc(millisec);
}

void *osMailCAlloc (osMailQId queue_id, uint32_t millisec) {
  return queue_id->calloc(millisec);
}

osStatus osMailPut (osMailQId queue_id, void *mail) {
  osAssertHandle(queue_id);
  return (queue_id->pushBack(mail) == true) ? osOK : osErrorNoMemory;
}

osEvent osMailGet (osMailQId queue_id, uint32_t millisec) {
  osAssertHandle(queue_id);

  osEvent retval;
  retval.status = osEventMail;
  retval.def.mail_id = queue_id;

  bool abort = false;
  do {
    retval.value.p = queue_id->popFront(millisec, &abort);
  } while((millisec == osWaitForever) && (!retval.value.p) && (!abort));

  if(!retval.value.p) {
    retval.status = abort ? osErrorResource : osEventTimeout;
  }
  return retval;
}

osStatus osMailFree (osMailQId queue_id, void *mail) {
  osAssertHandle(queue_id);
  return queue_id->freeItem(mail);
}

osStatus osMailDestroy_suppl(osMailQId* queue_id_ptr, uint32_t millisec) {
  typedef _os::MailQueueDestructor<os_pool_cb, os_mailQ_cb> destructor;
  return destructor::mailDestroy(queue_id_ptr, millisec);
}

void osMailClear_suppl(osMailQId queue_id) {
  osEvent event = osMailGet(queue_id, 0);
  while(event.status == osEventMail) {
    osMailFree(queue_id, event.value.p);
    event = osMailGet(queue_id, 0);
  }
}

#endif /* #if TARGET_RTOS != RTOS_ThreadX */
