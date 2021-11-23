/*
 *  $HeadURL: $
 *
 *  $Date: $
 *  $Author: $
 */

/**
 * \file
 * \brief
 * This file implements an ISR safe queue based on NoRTOS
 */

#ifndef atomic_queue_NoRTOS_H_
#define atomic_queue_NoRTOS_H_

#include "baseplate.h"
#include "boards/board-api/bapi_atomic.h"

#include <string.h>

namespace _os
{

/**
 * \ingroup _cmsis_os
 * \brief
 * An ISR safe queue using board api to achieve ISR safety.
 */
template<typename INDEX_TYPE = uint16_t> class UntypedAtomicQueue {

public:
  typedef INDEX_TYPE index_type;
  typedef uint8_t value_type;

protected:
  value_type* data;
  index_type maxItems;
  index_type first;
  index_type last;
  index_type validItems;
  size_t     itemSize;

  /** 
   * \brief Copies a queue item, assuming that it is a POD.
   */
  inline void assign(void* dst, const void* const src) {
    memcpy(dst, src, itemSize);
  }

  /** 
   * \brief Provide a pointer to the head item whithin the queue
   * It is assumed that the queue is currently not empty. Otherwise
   * the result is undefined.
   */
  inline void* front() const {
    ASSERT(data);
    return &data[first * itemSize];
  }

public:
  UntypedAtomicQueue()
      : data(0), maxItems(0), first(0), last(0), validItems(0), itemSize(0) {
  }

  bool create(index_type max_items, size_t item_size) {
    ASSERT(max_items > 0);
    data = new value_type[max_items * item_size];
    if(data != 0) {
      maxItems = max_items;
      itemSize = item_size;
    }
    return data != 0;
  }

  void destroy() {
    value_type* d = data;

    bapi_irq_enterCritical();

    if(data) {
      data = 0;
      bapi_irq_exitCritical();
      delete[] d;
      return;
    }

    bapi_irq_exitCritical();
  }

  inline ~UntypedAtomicQueue(){
    destroy();
  }

  inline index_type size()const {
    return atomic_Get(&validItems);
  }

  inline index_type max_items() const {
    /* maxItems will never change after creation. */
    return maxItems;
  }

  bool pushBack(const void* const item, bapi_SystemTick_t UNUSED(tickTimeToWait)) {
    bool retval = false;

    bapi_irq_enterCritical();

    /* data will never change after creation. */
    ASSERT(data);

    if (validItems < maxItems) {
      ++validItems;
      assign(&data[last * itemSize], item);
      last = (last + 1) % maxItems;

      retval = true;
    }

    bapi_irq_exitCritical();
    return retval;
  }

  bool popFront(bapi_SystemTick_t UNUSED(tickTimeToWait), void* const item) {
    bool retval = false;

    bapi_irq_enterCritical();

    /* data will never change after creation. */
    ASSERT(data);

    if (validItems > 0) {
      assign(item, front());
      first = (first + 1) % maxItems;
      --validItems;

      retval = true;
    }

    bapi_irq_exitCritical();

    return retval;
  }
};

} // namespace NoRTOS

#endif /* #ifndef atomic_queue_NoRTOS_H_ */
