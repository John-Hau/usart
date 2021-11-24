#ifndef UTILS_TypedQueue_H_
#define UTILS_TypedQueue_H_

#include "baseplate.h"

#ifdef __IAR_SYSTEMS_ICC__
  #include <stdlib.h>
#else
  #include <malloc.h>
#endif

#include "object_traits.hpp"

namespace utils
{


template<typename ITEM_TYPE, typename INDEX_TYPE = unsigned int> class TypedQueue;

/**
 * \brief A structure that allows specialization or partial specialization of some
 * CircularQueue functions for optimization on fundamental data types as ITEM_TYPE.
 *
 * This structure cannot be placed within the scope of CircularQueue, because partial
 * specialization is only allowed in a namespace scope. Hence we declare it outside,
 * and make it a friend.
 *
 */

template<
  typename ITEM_TYPE, typename INDEX_TYPE, bool is_fundamental=utils::type_traits<ITEM_TYPE>::is_fundamental
>
struct TypedQueueHelper {
  /**
   * \brief remove multiple items from the queue including a ITEM_TYPE destructor call.
   * \return the number of removed items.
   */
 static inline INDEX_TYPE popMultiple(
     TypedQueue<ITEM_TYPE, INDEX_TYPE>& queue   /** [in] The queue to operate on. */
    ,INDEX_TYPE maxCount                           /** [in] The maximum number of items to remove. */
  );
};

template<
  typename ITEM_TYPE, typename INDEX_TYPE
>
struct TypedQueueHelper<ITEM_TYPE, INDEX_TYPE, true> {
  /**
   * \brief remove multiple items from the queue without calling the ITEM_TYPE destructor.
   * \return the number of removed items.
   */
  static inline INDEX_TYPE popMultiple(
     TypedQueue<ITEM_TYPE, INDEX_TYPE>& queue   /** [in] The queue to operate on. */
    ,INDEX_TYPE maxCount                           /** [in] The maximum number of items to remove. */
  );
};

/**
 * TODO: This class is currently untested.
 */
template<typename ITEM_TYPE, typename INDEX_TYPE> class TypedQueue {

  friend struct TypedQueueHelper<ITEM_TYPE, INDEX_TYPE, utils::type_traits<ITEM_TYPE>::is_fundamental>;

public:
  typedef INDEX_TYPE index_type;
  typedef ITEM_TYPE value_type;
  typedef value_type& reference;
  typedef const value_type const_reference;

private:
  index_type maxItems;   /**< The size of the queue in item units. */
  index_type first;      /**< Memory array index of the first item the queue. */
  index_type end;        /**< Memory array index behind the last item the queue. */
  index_type validItems; /**< The number of items that are currently in the queue. */
  value_type* data;      /**< The memory on the heap where the items are stored. */


  index_type _pushMultiple(const value_type* items, index_type countDemand) {
    index_type retval = 0;
    if (available()) {
      /*
       * For the following case pictures:
       * m = maxItems, f=first, e= end, c = count
       */
      if (end < first) {
        /*
         * | 0 | 1 | 2 | ... | n-3 | n-2 | n-1 |
         *                ^     ^                ^
         *                f     e                m
         *                |<-c->|
         */
        retval = MIN(first - end, countDemand);
        utils::object<value_type, index_type>::copy(&data[end], items, retval);
        end = (end + retval) % maxItems;
        validItems += retval;
      }
      else {
        if (end + countDemand <= maxItems) {
          /*
           * | 0 | 1 | 2 | ... | n-3 | n-2 | n-1 |
           *               ^      ^                ^
           *               e      f                m
           *                      |<--- c --->|
           */
          retval = countDemand;
          utils::object<value_type, index_type>::copy(&data[end], items, retval);
          end = (end + retval) % maxItems;
          validItems += retval;
        }
        else {
          /*
           * | 0 | 1 | 2 | ... | n-3 | n-2 | n-1 |
           *               ^      ^                ^
           *               e      f                m
           *                      |<--- c ---...
           *  ...--- c --->|
           */
          retval = maxItems - end;
          utils::object<value_type, index_type>::copy(&data[end], items, retval);
          end = MIN(first, countDemand - retval);
          utils::object<value_type, index_type>::copy(&data[0], &items[retval], end);
          retval += end;
          validItems += retval;
        }
      }
    }
    return retval;
  }
  
  index_type _push(const_reference item) {
    if (validItems < maxItems) {
      ++validItems;
      data[end] = item;
      end = (end + 1) % maxItems;
      return 1;
    }
    return 0;
  }
  
public:
  TypedQueue()
      : maxItems(0), first(0), end(0), validItems(0), data(0) {
  }

  inline void flush() {
    while ( !empty() ) {
      popMultiple(validItems);
    }
  }
  
  /**
   * For debug purpose only.
   */
  inline index_type _first()const {
    return first;
  }

  /**
   * For debug purpose only.
   */
  inline index_type _end()const {
    return end;
  }

  /**
   * \brief Create the queue by allocating memory for it.
   */
  bool create(
    index_type max_items /** [in] The number of items that the queue shall be able to hold. */
  ) {
    ASSERT(max_items > 0);
    maxItems = max_items;
    data = S_CAST(value_type*, ::calloc(maxItems, sizeof(value_type)));
    return data != 0;
  }

  void destroy() {
    flush();
    maxItems = 0;
    free(data);
    data = 0;
  }

  virtual ~TypedQueue() {
    destroy();
  }

  /**
   * \return The number of available entries in the queue.
   */
  inline index_type available()const {
    return maxItems - validItems;
  }

  /**
   * \return The number of items that are currently in the queue.
   */
  inline index_type size()const {
    return validItems;
  }

  /**
   * \return true, if the queue is empty. Otherwise false.
   */
  inline bool empty()const {
    if (validItems == 0) {
      return true;
    }
    return false;
  }

  /**
   * \return The total size of the queue.
   */
  inline index_type max_items() const {
    return maxItems;
  }

  /**
   * \brief Push a new item into the queue, if there is space.
   * \return The number of pushed items, which is 1, on success.
   * Otherwise 0.
   * */
  inline index_type push(const_reference item) {
    ASSERT(data);
    return _push(item);
  }

  inline const_reference front() const {
    ASSERT(data);
    return data[first];
  }

  inline const value_type* pfront() const {
    ASSERT(data);
    return &data[first];
  }

//  inline value_type* pfront() {
//    ASSERT(data);
//    return &data[first];
//  }

  inline const value_type* pdata() const {
    ASSERT(data);
    return data;
  }


  /**
   * \brief remove a single item from the queue, if there is any.
   */
  void pop() {
    if (empty()) {
      return;
    }

    data[first].value_type::~value_type();
    first = (first + 1) % maxItems;
    --validItems;
  }

  /**
   * \brief remove multiple items from the queue.
   * \return the number of removed items.
   */
  inline index_type popMultiple(
      index_type maxCount /** [in] The maximum number of items to remove. */
    ) {
    return TypedQueueHelper<value_type, index_type>::popMultiple(*this, maxCount);
  }


  /**
   * \return the number items that are available consecutively in the queue beginning starting from front().
   */
  inline index_type consecutive() {
    if(validItems > 0) {
      if(end > first) {
        /* We don't have a wrap around. */
        /*
         * | 0 | 1 | 2 | ... | n-3 | n-2 | n-1 |
         *                ^           ^          ^
         *                f           e          m
         *                |<--- c --->|
         */
        return end - first;
      }
      /* We have a wrap around. */
      /*
       * | 0 | 1 | 2 | ... | n-3 | n-2 | n-1 |
       *               ^      ^                ^
       *               e      f                m
       *                      |<--- c ---...
       *  ...--- c --->|
       */
      return maxItems - first;
    }
    return 0;
  }

  /**
   * \return the number items that are available consecutively starting from index 0 of the queue.
   */
  inline index_type wrappedConsecutive() {
    if(validItems > 0) {
      if(end > first) {
        /* We don't have a wrap around. */
        /*
         * | 0 | 1 | 2 | ... | n-3 | n-2 | n-1 |
         *                ^           ^          ^
         *                f           e          m
         *                |<--- c --->|
         */
        return 0;
      }
      /* We have a wrap around. */
      /*
       * | 0 | 1 | 2 | ... | n-3 | n-2 | n-1 |
       *               ^      ^                ^
       *               e      f                m
       *                      |<--- c ---...
       *  ...--- c --->|
       */
      return end;
    }
    return 0;
  }

  /**
   * \brief push multiple items into the queue.
   * \return the number of pushed items. That might be less than demanded.
   */
  inline index_type pushMultiple(
      const value_type* items /** [in] Pointer to all items to push. */
    , index_type countDemand  /** [in] The demanded number of items to push. */
    ) {

    return (countDemand == 1 ? _push(*items) : _pushMultiple(items, countDemand));
  }
};


template<
  typename ITEM_TYPE, typename INDEX_TYPE, bool is_fundamental
>
INDEX_TYPE TypedQueueHelper<ITEM_TYPE, INDEX_TYPE, is_fundamental>::popMultiple(
     TypedQueue<ITEM_TYPE, INDEX_TYPE>& queue   /** [in] The queue to operate on. */
    ,INDEX_TYPE maxCount                           /** [in] The maximum number of items to remove. */
  ) {
  INDEX_TYPE retval = MIN(queue.size(), maxCount);
  maxCount = retval;
  /* Do one by one, which ensures that the item destructor is called. */
  while(maxCount) {
    queue.pop();
    --maxCount;
  }

  return retval;
}

template<
  typename ITEM_TYPE, typename INDEX_TYPE
>
INDEX_TYPE TypedQueueHelper<ITEM_TYPE, INDEX_TYPE, true>::popMultiple (
      TypedQueue<ITEM_TYPE, INDEX_TYPE>& queue   /** [in] The queue to operate on. */
    , INDEX_TYPE maxCount
  ) {
  /* Do all at once, because fundamental item types don't have a destructor. */
  INDEX_TYPE retval = MIN(queue.size(), maxCount);
  queue.first = (queue.first + retval) % queue.max_items();
  queue.validItems -= retval;
  return retval;
}


} // namespace utils

#endif // UTILS_TypedQueue_H_

