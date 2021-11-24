
#include "baseplate.h"
#include "boards/board-api/bapi_irq.h"
#include "non_freeable_heap.hpp"


typedef uint32_t ptr_size_t;

void *non_freeable_heap_malloc(uint8_t* theHeap_, size_t& nextFreeByte_, size_t adjustedHeapSize_, size_t size)
  {
  enum { BYTE_ALIGNMENT = SIZEOF_INT };
  enum { BYTE_ALIGNMENT_MASK = BYTE_ALIGNMENT-1};
  void *retval = nullptr;

  /* Ensure that blocks are aligned to the required number of bytes. */
#if BYTE_ALIGNMENT != 1
  if (size & BYTE_ALIGNMENT_MASK) {
    /* Byte alignment required. */
    size += (BYTE_ALIGNMENT - (size & BYTE_ALIGNMENT_MASK));
  }
#endif

  bapi_irq_enterCritical();

#if BYTE_ALIGNMENT != 1
  /* Ensure the heap starts on a correctly aligned boundary. */
  uint8_t* const pAlignedHeap = R_CAST(uint8_t *,
    (R_CAST(ptr_size_t, &theHeap_[ BYTE_ALIGNMENT ])) &
    (~(S_CAST(ptr_size_t, BYTE_ALIGNMENT_MASK)))
  );
#else
  uint8_t* const pAlignedHeap = theHeap_;
#endif

  /* Check there is enough room left for the allocation. */
  if (((nextFreeByte_ + size) < adjustedHeapSize_) &&
    ((nextFreeByte_ + size) > nextFreeByte_))/* Check for overflow. */
    {
    /* Return the next free byte then increment the index past this
     block. */
    retval = pAlignedHeap + nextFreeByte_;
    nextFreeByte_ += size;
  }

  bapi_irq_exitCritical();

  return retval;
}


