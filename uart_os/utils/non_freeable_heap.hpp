/*
 * non_freeable_heap.hpp
 *
 *  Created on: 2.09.2017
 *      Author: Wolfgang
 */

#ifndef non_freeable_heap_HPP_
#define non_freeable_heap_HPP_


#include "baseplate.h"
#include <stdlib.h>

void *non_freeable_heap_malloc(uint8_t* theHeap_, size_t& nextFreeByte_, size_t adjustedHeapSize_, size_t size);

template<unsigned HEAP_SIZE> struct nonFreeableHeap {

private:
  enum { BYTE_ALIGNMENT = SIZEOF_INT };
#if BYTE_ALIGNMENT != 1
  enum { ADJUSTED_HEAP_SIZE	= HEAP_SIZE - BYTE_ALIGNMENT };
#else
  enum { ADJUSTED_HEAP_SIZE = HEAP_SIZE };
#endif
  uint8_t m_Heap[HEAP_SIZE];
  size_t  m_nextFreeByte;

public:
  nonFreeableHeap() : m_nextFreeByte(0) {
  }

  void *malloc(size_t size) {
    return non_freeable_heap_malloc(m_Heap,  m_nextFreeByte, ADJUSTED_HEAP_SIZE, size);
  }

  void free(void* mem) {
    ASSERT(false); /* intentionally not supported. */
  }

  int32_t usableSize(const void* mem) {
    return 0; /* currently not supported. */
  }

  size_t availableSize(void) {
    return (ADJUSTED_HEAP_SIZE -  m_nextFreeByte);
  }
};

#endif /* non_freeable_heap_HPP_ */
