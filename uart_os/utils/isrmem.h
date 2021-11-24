/*
 * isrmem.h
 *
 *  Created on: 21.04.2013
 *      Author: Wolfgang
 */

#ifndef UTILS_MALLOC_H_
#define UTILS_MALLOC_H_

#include <stdint.h>

#include "baseplate.h"
#if __IAR_SYSTEMS_LIB__
  #include <stdlib.h>
#else
  #include <malloc.h>
#endif

#include "utils/utils.h"
#include "boards/board-api/bapi_irq.h"


/*
 * IAR language provider workarounds. IAR has problems with size_t
 *
 * Note: The cmake macro _create_language_provider_ invokes the compiler for the indexer with -D_eclipse_LANGUAGE_PROVIDER__
 *
 */
#if defined(__IAR_SYSTEMS_ICC__) && defined(_eclipse_LANGUAGE_PROVIDER__)
  #define ISRMEM_size_t unsigned
#else
  #define ISRMEM_size_t size_t
#endif

/** This ISRMEM memory management allows to defer freeing up memory that needs
 * to be freed up by an ISR. It also allows freeing up memory in the
 * non-ISR context. The advantage is, that the ISR is executed quick,
 * interrupts don't need to be disabled while the heap is being cleared. */

#ifdef _DEBUG
  // The PREAMLE helps validating, whether a memory chunk that is going
  // to be deallocated by an ISRMEM_* function was definitely allocated by
  // the ISRMEM_malloc(..) function.
  // Comment out if you don't want validation via PREAMBLE
  #define ISRMEM_PREAMBLE_VALUE 0xA5A5B5B5
#endif

typedef struct ualloc_mem_ {
#ifdef ISRMEM_PREAMBLE_VALUE
  uint32_t preamble;
#endif
  struct ualloc_mem_* next_to_free;
  void  (*free_callback)( void* ptr ); /* function to free allocated memory; may be NULL */
} ualloc_mem;


#ifdef ISRMEM_PREAMBLE_VALUE
  C_INLINE void ISRMEM_INIT_PREAMBLE(ualloc_mem* p_mem) {
    p_mem->preamble = ISRMEM_PREAMBLE_VALUE;
  }

  C_INLINE void ISRMEM_ASSERT_PREAMBLE(ualloc_mem* p_mem) {
    /* Test if memory was allocated by ISRMEM_malloc(). */
    ASSERT(p_mem->preamble == ISRMEM_PREAMBLE_VALUE);
  }

#else
  #define ISRMEM_INIT_PREAMBLE(p_mem)
  #define ISRMEM_ASSERT_PREAMBLE(p_mem)
#endif


/**
 * Allocates memory which is allowed to be deallocated in an ISR.
 * Returns pointer to the user data
 */
C_INLINE void* ISRMEM_malloc(ISRMEM_size_t size) {
  ualloc_mem* p_mem = S_CAST(ualloc_mem*, malloc(size + sizeof(ualloc_mem)));
  if(p_mem != 0) {
    ISRMEM_INIT_PREAMBLE(p_mem);
    p_mem->next_to_free = 0;
    p_mem->free_callback = free;
    return ((uint8_t*)p_mem) + sizeof(ualloc_mem);
  }
  return p_mem;
}

/** Add allocated memory to a chain. Memory will be freed up later.
 *  Supposed to be called from an ISR context to defer freeing up the memory.
 *  This function must not be interrupted by a higher prioritized ISR that
 *  uses the same chain.
 * */
C_INLINE ualloc_mem* ISRMEM_add_to_dealloc_chain(ualloc_mem* root, void* p_user_data) {
  if(p_user_data) {
    ualloc_mem* p_mem = R_CAST(ualloc_mem*, ((S_CAST(uint8_t*, p_user_data)) - sizeof(ualloc_mem)));
    ISRMEM_ASSERT_PREAMBLE(p_mem);

    /** Put new memory to free at the front of the chain */
    if(root != 0) {
      ASSERT(p_mem->next_to_free == 0); /** Memory must not be in chain. This error happens if ISRMEM_defer_free is called more than once for the same memory. */
      p_mem->next_to_free = root;
    }
    root = p_mem;
  }
  return root;
}

/** Immediately free up memory within non-ISR context. */
C_INLINE void ISRMEM_immediate_dealloc(void* p_user_data) {
  if(p_user_data) {
    ASSERT(bapi_irq_isInterruptContext() == false);
    ualloc_mem* p_mem =  R_CAST(ualloc_mem*, ((S_CAST(uint8_t*, p_user_data)) - sizeof(ualloc_mem)));
    ISRMEM_ASSERT_PREAMBLE(p_mem);
    ASSERT(p_mem->next_to_free == 0); /** Memory must not be in chain. */
    void (*free_callback)( void* ptr ) = p_mem->free_callback;
    if(free_callback) {
      (*free_callback)(p_mem);
    }
  }
}

/** Free up a single allocated memory and retrieve the next chained memory.
 *  Supposed to be called from a non-ISR context to free a single
 *  memory item within a chain. This function must not be interrupted
 *  by any ISR calling ISRMEM_chain(..) for the same chain, where
 *  this memory item is part of. */
C_INLINE ualloc_mem* ISRMEM_dealloc_and_remove_from_chain(ualloc_mem* p_mem) {
  if(p_mem != 0) {
    ASSERT(bapi_irq_isInterruptContext() == false);
    ISRMEM_ASSERT_PREAMBLE(p_mem);
    ualloc_mem* next_to_free = p_mem->next_to_free;
    p_mem->next_to_free = 0;
    void (*free_callback)( void* ptr ) = p_mem->free_callback;
    p_mem->free_callback = 0;
    if(free_callback) {
      (*free_callback)(p_mem);
    }
    return next_to_free;
  }
  return 0;
}

#endif /* UTILS_MALLOC_H_ */
