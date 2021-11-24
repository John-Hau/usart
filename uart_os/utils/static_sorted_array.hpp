/*
 * osaUtils.h
 *
 *  Created on: 21.04.2013
 *      Author: Wolfgang
 */

#ifndef static_sorted_array_H_included_
#define static_sorted_array_H_included_

#include "baseplate.h"
#include "utils/binary_search.hpp"
#include <string.h>

namespace utils {

template<typename T, unsigned ENTRIES_CNT> class static_sorted_array {

protected:
  T m_Array[ENTRIES_CNT];
  utils::ssize_t m_NextFree;

public:
  inline utils::ssize_t nextFree()const {return m_NextFree;}

  static_sorted_array()
    : m_NextFree(0) {
  };

  utils::ssize_t registerItem(const T& item) {
    /* Assert that there is space for a new entry.
     * If not, enlarge ENTRIES_CNT. */
    ASSERT(m_NextFree < ENTRIES_CNT);

    ssize_t i = utils::binary_search(m_Array, m_NextFree, item);
    ASSERT(i < m_NextFree);

    if(i>=0) {
      if(m_Array[i] == item) {
        return(-1); /* mem already registered. */
      }
    }

    /* Goto the first entry that needs to be moved up. */
    i++;

    memmove( &m_Array[i+1], &m_Array[i], (m_NextFree - i) * sizeof(m_Array[0]) );
    m_Array[i] = item;
    m_NextFree++;

    return i; /* Return the index of the inserted item */
  }

  utils::ssize_t findItem(const T& item) {
    ssize_t i = utils::binary_search(m_Array, m_NextFree, item);
    ASSERT(i < m_NextFree);

    if(i<0) {
      return(-1); /* mem not found */
    }

    if( m_Array[i] != item) {
      return(-1); /* mem not found */
    }
    return i;
  }

  void unregisterItem(ssize_t i)
    {
    memmove(&m_Array[i], &m_Array[i + 1], (m_NextFree - (i + 1)) * sizeof(m_Array[0]));
    m_NextFree--;
    m_Array[m_NextFree] = 0;
  }

  utils::ssize_t unregisterItem(const T& item) {
    ssize_t i = findItem(item);
    unregisterItem(i);
    return i; /* Return the index of the removed item */
  }

};


}

#endif /* static_sorted_array_H_included_ */
