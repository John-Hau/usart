/*
 * osCom.h
 *
 *  Created on: 07.04.2013
 *      Author: Wolfgang
 */

#ifndef globalMallocTracer_HPP_INCLUDED_
#define globalMallocTracer_HPP_INCLUDED_


#include "baseplate.h"
#include "clib-adapter/common/_globalMallocTracer.h"
#include "utils/mallocTracer.hpp"


/**
 * \ingroup global_malloc_tracer
 * \copydoc _mallocCountGlobalHeap()
 */
C_INLINE int32_t mallocCountGlobalHeap() {
  return _mallocCountGlobalHeap();
}

/**
 * \ingroup global_malloc_tracer
 * \copydoc _mallocSizeGlobalHeap()
 */
C_INLINE int32_t mallocSizeGlobalHeap() {
  return _mallocSizeGlobalHeap();
}

#ifdef __cplusplus

/**
 * \ingroup global_malloc_tracer
 * \copydoc _mallocTracerRtos()
 */
C_INLINE struct MallocTracer* mallocTracerRtos() {
#if !RTOS_USE_ALTERNATE_MEMORY_ALLOCATOR
  return _mallocTracerRtos();
#else
  #error "mallocTracerRtos not supported if RTOS_USE_ALTERNATE_MEMORY_ALLOCATOR enabled."
#endif
}

#endif

#endif /* globalMallocTracer_HPP_INCLUDED_ */
