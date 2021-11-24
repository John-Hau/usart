/*
 * osaUtils.h
 *
 *  Created on: 25.02.2017
 *      Author: Wolfgang
 */

#ifndef _MALLOC_TRACER_HPP_INCLUDED_
#define _MALLOC_TRACER_HPP_INCLUDED_

#include "baseplate.h"

#ifdef __cplusplus

#ifndef MALLOC_TRACER_TRACE_LEVEL
  #define MALLOC_TRACER_TRACE_LEVEL 1 /* We may support further levels in future */
#endif


#include <stdint.h>
#include <string.h>

#if defined(__GNUC__)
#include <malloc.h>
#endif

#ifdef __IAR_SYSTEMS_ICC__
#include <stdlib.h>
#include <stddef.h>
#include <LowLevelIOInterface.h>

C_FUNC   size_t __iar_dlmalloc_usable_size(const void*);
C_INLINE size_t malloc_usable_size(const void* mem) {
  return __iar_dlmalloc_usable_size(mem);
}

#endif

#if MALLOC_TRACER_TRACE_LEVEL > 0

#include "boards/board-api/bapi_atomic.h"

struct MallocTracer {

private:
  typedef void*(*malloc_t)(size_t bytes);
  typedef void(*free_t)(void* mem);
#if defined(__GNUC__)
  typedef size_t(*usableSize_t)(void* mem);
#endif
#ifdef __IAR_SYSTEMS_ICC__
  typedef size_t(*usableSize_t)(const void* mem);
#endif

  malloc_t mallocFunc;
  free_t freeFunc;
  usableSize_t usableSizeFunc;

  int32_t m_mallocCount;
  int32_t m_mallocSize;

public:
  MallocTracer(malloc_t _mallocFunc = ::malloc
    , free_t _freeFunc = ::free
    , usableSize_t _usableSizeFunc = ::malloc_usable_size)
    : mallocFunc(_mallocFunc), freeFunc(_freeFunc)
    , usableSizeFunc(_usableSizeFunc)
    , m_mallocSize(0), m_mallocCount(0) {
  }


  void setMallocFunc(malloc_t _mallocFunc
    , free_t _freeFunc
    , size_t(* _usableSizeFunc)(const void* mem)
    ) {
    mallocFunc = _mallocFunc;
    freeFunc = _freeFunc;
    usableSizeFunc = R_CAST(usableSize_t, _usableSizeFunc);
  }

  /* like calloc */
  void* c(size_t num, size_t size) {
    void* mem = mallocFunc(num * size);
    if(mem) {
      size_t us = usableSizeFunc(mem);
      if(us == 0) {
        us = num * size;
      }
      MEMSET(mem, 0, num * size);
      atomic_Int32Add(&m_mallocCount, 1);
      atomic_Int32Add(&m_mallocSize, us);
    }
    return mem;
  }

  /* like malloc */
  void* m(int32_t bytes) {
    void* mem = mallocFunc(bytes);
    if(mem) {
      size_t us = usableSizeFunc(mem);
      if(us == 0) {
        us = bytes;
      }
      atomic_Int32Add(&m_mallocCount, 1);
      atomic_Int32Add(&m_mallocSize, us);
    }
    return mem;
  }

  /* like free */
  void f(void* mem) {
    if(mem) {
      ASSERT_DEBUG(usableSizeFunc);
      size_t us = usableSizeFunc(mem);
      ASSERT_DEBUG(us);
      atomic_Int32Add(&m_mallocSize, (-1 * us));
      atomic_Int32Add(&m_mallocCount, -1);
      freeFunc(mem);
    }
  }

  int32_t mallocCount() const {
    return m_mallocCount;
  }

  int32_t mallocSize() const {
    return m_mallocSize;
  }
};
#else

struct MallocTracer {
private:
  typedef void*(*malloc_t)(size_t bytes);
  typedef void(*free_t)(void* mem);
#if defined(__GNUC__)
  typedef size_t(*usableSize_t)(void* mem);
#else
  typedef size_t(*usableSize_t)(const void* mem);
#endif

  malloc_t mallocFunc;
  free_t freeFunc;

public:
  MallocTracer(
      malloc_t _mallocFunc = ::malloc
    , free_t _freeFunc = ::free
    , usableSize_t UNUSED(_usableSizeFunc) = nullptr)
    : mallocFunc(_mallocFunc), freeFunc(_freeFunc) {
  }

  void setMallocFunc(malloc_t _mallocFunc
    , free_t _freeFunc
    , size_t(* _usableSizeFunc)(const void* mem)
    ) {
    mallocFunc = _mallocFunc;
    freeFunc = _freeFunc;
//    usableSizeFunc = R_CAST(usableSize_t, _usableSizeFunc);
  }

  /* like calloc */
  void* c(size_t num, size_t size) {
    void* mem = mallocFunc(num * size);
    if(mem) {
      MEMSET(mem, 0, num * size);
    }
    return mem;
  }

  /* like malloc */
  void* m(int32_t bytes) {
    return mallocFunc(bytes);
  }

  /* like free */
  void f(void* mem) {
    freeFunc(mem);
  }

  int32_t mallocCount()const {
    return 0;
  }

  int32_t mallocSize()const {
    return 0;
  }
};

#endif /* MALLOC_TRACER_TRACE_LEVEL */

#endif /* __cplusplus */

#endif /* _MALLOC_TRACER_HPP_INCLUDED_ */
