/*
 * bapi_irq.h
 *
 *  Created on: 07.04.2013
 *      Author: Wolfgang
 */

#ifndef BAPI_ATOMIC_H_
#define BAPI_ATOMIC_H_

/** \file
 * \brief
 * This file declares and implements atomic access functions to C built in data types.
 * */

#include <stddef.h>

#include "baseplate.h"
#include "boards/board-api/bapi_common.h"
#include "boards/board-api/bapi_irq.h"
#include "utils/utils.h"


/* In cases we are on an Cortex M that is supporting __LDREXW, __STREXW, let's use it ! */
#ifdef __CORE_CMINSTR_H
  /** \brief We are on an Cortex M that may support __LDREXW, __STREXW
   * We set this as default, so that when other MCUs will be added, this
   * optimization will take place per default. When the compiler
   * complains for a particular MCU, we can skip the optimization for this MCU
   * via #undef.
   */
  #define USE_ARM_LDREX_STREX

  #ifdef CPU_MKL46Z256VLL4
    /* Skip optimization for this MCU */
    #undef USE_ARM_LDREX_STREX
  #endif

  #ifdef CPU_MKL17Z128VLH4
    /* Skip optimization for this MCU */
    #undef USE_ARM_LDREX_STREX
  #endif

  #ifdef CPU_MKL17Z256VLH4
    /* Skip optimization for this MCU */
    #undef USE_ARM_LDREX_STREX
  #endif

#endif

/**
 * \ingroup _bapi_atomic
 * \brief
 * Call bapi_irq_enterCritical() if the size of the variable is greater than the native MCU byte width.
 * The \em if comparison on the size of the data type will (hopefully) be optimized away
 * by the compiler, since it can be determined at compile time.
 */
#define _atomic_enterCriticalConditional(X) \
  if( sizeof(X) > MCU_CORE_BYTE_WIDTH ) { \
    bapi_irq_enterCritical(); \
  }

/**
 * \ingroup _bapi_atomic
 * \brief
 * Call bapi_irq_exitCritical() if the size of the variable is greater than the native MCU byte width.
 * The \em if comparison on the size of the data type will (hopefully) be optimized away by the
 * compiler, since it can be determined at compile time.
 */
#define _atomic_exitCriticalConditional(X) \
  if( sizeof(X) > MCU_CORE_BYTE_WIDTH ) { \
    bapi_irq_exitCritical(); \
  }


 /**
  * \ingroup _bapi_atomic
  * \brief
  * Asserts if an address is aligned to the MCU native byte width, so that a particular
  * number of bytes can be read/written from/to that address, without disabling
  * interrupts.
  */
#ifdef _DEBUG
  C_INLINE void _atomic_AssertAlignment(volatile const void * const  a, size_t s)  {
    /* if the sizeof(*x) is smaller than the native MCU byte width, we force an address
     * alignment, so that we can avoid an disable irq call. So we don't force alignment
     * of the data.
     */
    if(sizeof(s) <= MCU_CORE_BYTE_WIDTH) {
      unsigned int coreAlignedRest = ((unsigned int)a) % MCU_CORE_BYTE_WIDTH;
      unsigned int dataAlignedRest = coreAlignedRest % s;
      ASSERT(dataAlignedRest == 0);
    }
  }
#else
  C_INLINE void _atomic_AssertAlignment(volatile const void * const  UNUSED(a), size_t UNUSED(s))  {
  }
#endif


/**
 * \addtogroup bapi_atomic
 */
/**@{*/

/**
 * \anchor AtomicOperationsC
 * \name Atomic (Interrupt Safe) operations for built in C data types
 *
 * \brief These functions ensure, that the associated operations are not disturbed by an interrupt,
 * regardless of the underlying MCU architecture.
 * E.g. on a 16 bit architecture, a 32 bit value read will be done along with a disable / enable
 * interrupt sequence, while it is implemented as a simple read on a 32 bit or 64 bit architecture.
 * \warning Works only if the applicable variable is located at an MCU aligned address.
 * See also \ref MCU_aligned_built_in_C_types "MCU aligned built in C data types".
 *
 * \note There are more convenient functions available for C++: \ref AtomicOperationsCPP
 */
/**@{*/
C_INLINE void atomic_Uint32bitwiseOR(volatile uint32_t *const x, const uint32_t v) {
  // TODO: Utilize bit banding by placing static variables into end of upper SRAM via linker script
  /* The default solution is to disable interrupts. */
  bapi_irq_enterCritical();
  *x |= v;
  bapi_irq_exitCritical();
}

C_INLINE void atomic_Uint32bitwiseAND(volatile uint32_t *const x, const uint32_t v) {
  // TODO: Utilize bit banding by placing static variables into end of upper SRAM via linker script
  /* The default solution is to disable interrupts. */
  bapi_irq_enterCritical();
  *x &= v;
  bapi_irq_exitCritical();
}

C_INLINE uint32_t atomic_Uint32Get(volatile const uint32_t * const  x)  {
  uint32_t retval;
  _atomic_AssertAlignment(x, sizeof(*x));
  _atomic_enterCriticalConditional(*x);
  retval = *x;
  _atomic_exitCriticalConditional(*x);
  return retval;
}

C_INLINE void atomic_Uint32Set(volatile uint32_t * const  x, const uint32_t v)  {
  _atomic_AssertAlignment(x, sizeof(*x));
  _atomic_enterCriticalConditional(*x);
  *x = v;
  _atomic_exitCriticalConditional(*x);
}

C_INLINE uint32_t atomic_Uint32Replace(volatile uint32_t * const  x, const uint32_t v)  {
  bapi_irq_enterCritical();
  uint32_t retval = *x;
  *x = v;
  bapi_irq_exitCritical();
  return retval;
}

C_INLINE void atomic_Uint32Add(volatile uint32_t *const x, const int32_t a) {
#if defined(USE_ARM_LDREX_STREX) && ( SIZEOF_INT == SIZEOF_LONG )
  /* Utilize arm core features */
  _atomic_AssertAlignment(x, sizeof(*x));
  unsigned long v = __LDREXW((volatile unsigned long *const)x) + a;
  while (__STREXW(v, (volatile unsigned long *const)x)) {
    v = __LDREXW((volatile unsigned long *const)x) + a;
  }
#else
  /* The default solution is to disable interrupts. */
  bapi_irq_enterCritical();
  *x += a;
  bapi_irq_exitCritical();
#endif
}

C_INLINE void atomic_Int32bitwiseOR(volatile int32_t *const x, const int32_t v) {
  atomic_Uint32bitwiseOR((volatile uint32_t *const)x, (const uint32_t)v);
}

C_INLINE void atomic_Int32bitwiseAND(volatile int32_t *const x, const int32_t v) {
  atomic_Uint32bitwiseAND((volatile uint32_t *const)x, (const uint32_t)v);
}

C_INLINE int32_t atomic_Int32Get(volatile const int32_t * const  x)  {
  return (uint32_t)atomic_Uint32Get((volatile const uint32_t * const)x);
}

C_INLINE void atomic_Int32Set(volatile int32_t *const x, const int32_t v) {
  atomic_Uint32Set((volatile uint32_t *const)x, (const uint32_t) v);
}

C_INLINE int32_t atomic_Int32Replace(volatile int32_t * const  x, const int32_t v)  {
  bapi_irq_enterCritical();
  int32_t retval = *x;
  *x = v;
  bapi_irq_exitCritical();
  return retval;
}

C_INLINE void atomic_Int32Add(volatile int32_t *const x, const int32_t a) {
  atomic_Uint32Add((uint32_t *const)(x) , a);
}

C_INLINE void atomic_Uint16bitwiseOR(volatile uint16_t *const x, const uint16_t v) {
  // TODO: Utilize bit banding by placing static variables into end of upper SRAM via linker script
  /* The default solution is to disable interrupts. */
  bapi_irq_enterCritical();
  *x |= v;
  bapi_irq_exitCritical();
}

C_INLINE void atomic_Uint16bitwiseAND(volatile uint16_t *const x, const uint16_t v) {
  // TODO: Utilize bit banding by placing static variables into end of upper SRAM via linker script
  /* The default solution is to disable interrupts. */
  bapi_irq_enterCritical();
  *x &= v;
  bapi_irq_exitCritical();
}

C_INLINE uint16_t atomic_Uint16Get(volatile const uint16_t * const x) {
  uint16_t retval;
  _atomic_AssertAlignment(x, sizeof(*x));
  _atomic_enterCriticalConditional(*x);
  retval = *x;
  _atomic_exitCriticalConditional(*x);
  return retval;
}

C_INLINE void atomic_Uint16Set(volatile uint16_t * const  x, const uint16_t v)  {
  _atomic_AssertAlignment(x, sizeof(*x));
  _atomic_enterCriticalConditional(*x);
  *x = v;
  _atomic_exitCriticalConditional(*x);
}

C_INLINE uint16_t atomic_Uint16Replace(volatile uint16_t * const  x, const uint16_t v)  {
  bapi_irq_enterCritical();
  uint16_t retval = *x;
  *x = v;
  bapi_irq_exitCritical();
  return retval;
}

C_INLINE void atomic_Uint16Add(volatile uint16_t *const x, const int16_t a) {
#if defined(USE_ARM_LDREX_STREX)
  /* Utilize arm core features */
  _atomic_AssertAlignment(x, sizeof(*x));
  uint16_t v = __LDREXH(x) + a;
  while (__STREXH(v, x)) {
    v = __LDREXH(x) + a;
  }
#else
  /* The default solution is to disable interrupts. */
  bapi_irq_enterCritical();
  *x += a;
  bapi_irq_exitCritical();
#endif
}

C_INLINE void atomic_Int16bitwiseOR(volatile int16_t *const x, const int16_t v) {
  atomic_Uint16bitwiseOR((volatile uint16_t *const)x, (const uint16_t)v);
}

C_INLINE void atomic_Int16bitwiseAND(volatile int16_t *const x, const int16_t v) {
  atomic_Uint32bitwiseAND((volatile uint32_t *const)x, (const uint32_t)v);
}

C_INLINE int16_t atomic_Int16Get(volatile const int16_t *const  x)  {
  return (int16_t)atomic_Uint16Get((volatile const uint16_t *const)x);
}

C_INLINE void atomic_Int16Set(volatile int16_t *const x, const int16_t v) {
  atomic_Uint16Set((volatile uint16_t *const)x, (const uint16_t) v);
}

C_INLINE int16_t atomic_Int16Replace(volatile int16_t * const  x, const int16_t v)  {
  bapi_irq_enterCritical();
  int16_t retval = *x;
  *x = v;
  bapi_irq_exitCritical();
  return retval;
}

C_INLINE void atomic_Int16Add(volatile int16_t *const x, const int16_t a) {
  atomic_Uint16Add((uint16_t *const)(x) , a);
}

C_INLINE void atomic_Uint8bitwiseOR(volatile uint8_t *const x, const uint8_t v) {
  // TODO: Utilize bit banding by placing static variables into end of upper SRAM via linker script
  /* The default solution is to disable interrupts. */
  bapi_irq_enterCritical();
  *x |= v;
  bapi_irq_exitCritical();
}

C_INLINE void atomic_Uint8bitwiseAND(volatile uint8_t *const x, const uint8_t v) {
  // TODO: Utilize bit banding by placing static variables into end of upper SRAM via linker script
  /* The default solution is to disable interrupts. */
  bapi_irq_enterCritical();
  *x &= v;
  bapi_irq_exitCritical();
}

C_INLINE uint8_t atomic_Uint8Get(volatile const uint8_t *const  x)  {
 return *x;
}

C_INLINE void atomic_Uint8Set(volatile uint8_t * const  x, const uint8_t v)  {
  *x = v;
}

C_INLINE uint8_t atomic_Uint8Replace(volatile uint8_t * const  x, const uint8_t v)  {
  bapi_irq_enterCritical();
  uint8_t retval = *x;
  *x = v;
  bapi_irq_exitCritical();
  return retval;
}

C_INLINE void atomic_Uint8Add(volatile uint8_t *const x, const int8_t a) {
#if defined(USE_ARM_LDREX_STREX)
  /* Utilize arm core features */
  _atomic_AssertAlignment(x, sizeof(*x));
  uint8_t v = __LDREXB(x) + a;
  while (__STREXB(v, x)) {
    v = __LDREXB(x) + a;
  }
#else
  /* The default solution is to disable interrupts. */
  bapi_irq_enterCritical();
  *x += a;
  bapi_irq_exitCritical();
#endif
}

C_INLINE void atomic_Int8bitwiseOR(volatile int8_t *const x, const int8_t v) {
  atomic_Uint8bitwiseOR((volatile uint8_t *const)x, (const uint8_t)v);
}

C_INLINE void atomic_Int8bitwiseAND(volatile int8_t *const x, const int8_t v) {
  atomic_Uint8bitwiseAND((volatile uint8_t *const)x, (const uint8_t)v);
}

C_INLINE int8_t atomic_Int8Get(volatile const int8_t *const  x)  {
  return (int8_t)atomic_Uint8Get((volatile const uint8_t *const)x);
}

C_INLINE void atomic_Int8Set(volatile int8_t *const x, const int8_t v) {
  atomic_Uint8Set((volatile uint8_t *const)x, (const uint8_t) v);
}

C_INLINE int8_t atomic_Int8Replace(volatile int8_t * const  x, const int8_t v)  {
  bapi_irq_enterCritical();
  int8_t retval = *x;
  *x = v;
  bapi_irq_exitCritical();
  return retval;
}

C_INLINE void atomic_Int8Add(volatile int8_t *const x, const int8_t a) {
  atomic_Uint8Add((uint8_t *const)(x) , a);
}

C_INLINE bool atomic_BoolGet(volatile const bool *const  x)  {
  return *x;
}

C_INLINE void atomic_BoolSet(volatile bool *const x, const bool v) {
  *x = v;
}

C_INLINE bool atomic_BoolReplace(volatile bool * const  x, const bool v)  {
  bapi_irq_enterCritical();
  bool retval = *x;
  *x = v;
  bapi_irq_exitCritical();
  return retval;
}

#ifdef __IAR_SYSTEMS_ICC__
  /* IAR compiler has problems with pointer to void pointer. So we use
   * unsigned int which should big enough to hold a void pointer. */
  typedef unsigned int _atomic_voidPtr_t;
  typedef unsigned int _atomic_voidCptr_t;
  #define ATOMIC_VOID_PTR_ASSERT() ASSERT(sizeof(_atomic_voidPtr_t) == sizeof(void*))
#else
  typedef void* _atomic_voidPtr_t;
  typedef const void* _atomic_voidCptr_t;
  #define ATOMIC_VOID_PTR_ASSERT()
#endif

C_INLINE _atomic_voidPtr_t atomic_VoidPtrReplace(_atomic_voidPtr_t *const x, const _atomic_voidPtr_t v)  {
  ATOMIC_VOID_PTR_ASSERT();

  bapi_irq_enterCritical();
  _atomic_voidPtr_t retval = *x;
  *x = v;
  bapi_irq_exitCritical();
  return retval;
}

C_INLINE _atomic_voidCptr_t atomic_VoidCptrReplace(_atomic_voidCptr_t *const x, const _atomic_voidCptr_t v)  {
  ATOMIC_VOID_PTR_ASSERT();

  bapi_irq_enterCritical();
  _atomic_voidCptr_t retval = *x;
  *x = v;
  bapi_irq_exitCritical();
  return retval;
}


#define atomic_PtrReplace(PTR_TYPE, x, v) \
    R_CAST(PTR_TYPE, atomic_VoidPtrReplace(R_CAST(_atomic_voidPtr_t *const, x) , R_CAST(const _atomic_voidPtr_t,v)));

#define atomic_CptrReplace(PTR_TYPE, x, v) \
    R_CAST(PTR_TYPE, atomic_VoidCptrReplace(R_CAST(_atomic_voidCptr_t *const, x) , R_CAST(const _atomic_voidCptr_t,v)));

/**@}*/
/**@}*/


#ifdef __cplusplus

#include "utils/int.hpp"

/**
 * \addtogroup bapi_atomic
 */
/**@{*/

/**
 * \anchor AtomicOperationsCPP
 * \name Atomic (Interrupt Safe) C++ overloads for built in C data types.
 *
 * These functions ensure the same as their native C peers (See \ref AtomicOperationsC),
 * but don't need the _Int<X> _Uint<X> appendix, because C++ uses the parameter types
 * to deduce which function to call.
 */
/**@{*/
  inline void atomic_bitwiseOR(volatile uint32_t *const x, const uint32_t v) {
    atomic_Uint32bitwiseOR(x, v);
  }

  inline void atomic_bitwiseAND(volatile uint32_t *const x, const uint32_t v) {
    atomic_Uint32bitwiseAND(x, v);
  }

  inline uint32_t atomic_Get(volatile const uint32_t * const  x) {
    return atomic_Uint32Get(x);
  }

  inline void atomic_Set(volatile uint32_t * const  x, const uint32_t v) {
    atomic_Uint32Set(x, v);
  }

  inline void atomic_Add(volatile uint32_t *const x, const int32_t a) {
    atomic_Uint32Add(x, a);
  }

  inline void atomic_Add(volatile unsigned int *const x, const unsigned int a) {
    atomic_Add(
       R_CAST( volatile unsigned_int<sizeof(unsigned int)>::type *const, x)
      ,S_CAST( const unsigned_int<sizeof(unsigned int)>::type, a)
    );
  }

/*
 * atomic operations specialization for int32_t
 */
  inline void atomic_bitwiseOR(volatile int32_t *const x, const int32_t v) {
    atomic_Int32bitwiseOR(x, v);
  }

  inline void atomic_bitwiseAND(volatile int32_t *const x, const int32_t v) {
    atomic_Int32bitwiseAND(x, v);
  }

  inline int32_t atomic_Get(volatile const int32_t * const  x) {
    return atomic_Int32Get(x);
  }

  inline void atomic_Set(volatile int32_t * const  x, const int32_t v) {
    atomic_Int32Set(x, v);
  }

  inline void atomic_Add(volatile int32_t *const x, const int32_t a) {
    atomic_Int32Add(x, a);
  }

/*
 * atomic operations specialization for uint16_t
 */
  inline void atomic_bitwiseOR(volatile uint16_t *const x, const uint16_t v) {
    atomic_Uint16bitwiseOR(x, v);
  }

  inline void atomic_bitwiseAND(volatile uint16_t *const x, const uint16_t v) {
    atomic_Uint16bitwiseAND(x, v);
  }

  inline uint16_t atomic_Get(volatile const uint16_t * const  x) {
    return atomic_Uint16Get(x);
  }

  inline void atomic_Set(volatile uint16_t * const  x, const uint16_t v) {
    atomic_Uint16Set(x, v);
  }

  inline void atomic_Add(volatile uint16_t *const x, const int16_t a) {
    atomic_Uint16Add(x, a);
  }

/*
 * atomic operations specialization for int16_t
 */
  inline void atomic_bitwiseOR(volatile int16_t *const x, const int16_t v) {
    atomic_Int16bitwiseOR(x, v);
  }

  inline void atomic_bitwiseAND(volatile int16_t *const x, const int16_t v) {
    atomic_Int16bitwiseAND(x, v);
  }

  inline int16_t atomic_Get(volatile const int16_t * const  x) {
    return atomic_Int16Get(x);
  }

  inline void atomic_Set(volatile int16_t * const  x, const int16_t v) {
    atomic_Int16Set(x, v);
  }

  inline void atomic_Add(volatile int16_t *const x, const int16_t a) {
    atomic_Int16Add(x, a);
  }

/*
 * atomic operations specialization for uint8_t
 */
  inline void atomic_bitwiseOR(volatile uint8_t *const x, const uint8_t v) {
    atomic_Uint8bitwiseOR(x, v);
  }

  inline void atomic_bitwiseAND(volatile uint8_t *const x, const uint8_t v) {
    atomic_Uint8bitwiseAND(x, v);
  }

  inline uint8_t atomic_Get(volatile const uint8_t * const  x) {
    return atomic_Uint8Get(x);
  }

  inline void atomic_Set(volatile uint8_t * const  x, const uint8_t v) {
    atomic_Uint8Set(x, v);
  }

  inline void atomic_Add(volatile uint8_t *const x, const int8_t a) {
    atomic_Uint8Add(x, a);
  }

/*
 * atomic operations specialization for int8_t
 */
  inline void atomic_bitwiseOR(volatile int8_t *const x, const int8_t v) {
    atomic_Int8bitwiseOR(x, v);
  }

  inline void atomic_bitwiseAND(volatile int8_t *const x, const int8_t v) {
    atomic_Int8bitwiseAND(x, v);
  }

  inline int8_t atomic_Get(volatile const int8_t * const  x) {
    return atomic_Int8Get(x);
  }

  inline void atomic_Set(volatile int8_t * const  x, const int8_t v) {
    atomic_Int8Set(x, v);
  }

  inline void atomic_Add(volatile int8_t *const x, const int8_t a) {
    atomic_Int8Add(x, a);
  }

  /*
   * atomic operations specialization for bool
   */
  inline bool atomic_Get(volatile const bool *const  x)  {
    return atomic_BoolGet(x);
  }

  inline void atomic_Set(volatile bool *const x, const bool v) {
    atomic_BoolSet(x, v);
  }

  inline bool atomic_Replace(volatile bool * const  x, const bool v)  {
    return atomic_BoolReplace(x, v);
  }


  template<typename T> inline T atomic_Replace(volatile T * const  x, const T v)  {
    bapi_irq_enterCritical();
    T retval = *x;
    *x = v;
    bapi_irq_exitCritical();
    return retval;
  }

  template<typename T> inline void atomic_Increment(volatile T *const x) {
    atomic_Add(x, 1);
  }

  template<typename T> inline void atomic_Decrement(volatile T *const x) {
    atomic_Add(x,-1);
  }

  template<typename EnumType> void atomic_enum_Set(volatile EnumType* x, const EnumType value) {
    typedef typename unsigned_int<sizeof(EnumType)>::type IntegerType;
    volatile IntegerType* ix = R_CAST(volatile IntegerType*,x);
    atomic_Set(ix, value);
  }

  template<typename EnumType> EnumType atomic_enum_Get(volatile const EnumType* x) {
    typedef typename unsigned_int<sizeof(EnumType)>::type IntegerType;
    volatile const IntegerType* ix = R_CAST(volatile const IntegerType*,x);
    return S_CAST(EnumType, atomic_Get(ix));
  }
/**@}*/
/**@}*/

#endif /* #ifdef __cplusplus */

/**
 * \addtogroup bapi_atomic
 */
/**@{*/

/**
 * \anchor MCU_aligned_built_in_C_types
 * \name MCU aligned built in C data types
 * \brief Variables of these types can be used along with the atomic operations, because they are aligned to the MCU architecture.
*/
/**@{*/
#if MCU_CORE_BYTE_WIDTH >= 4
  #define atomic_uint32_t volatile SIZE_ALIGNED(uint32_t)
  #define atomic_int32_t  volatile SIZE_ALIGNED(int32_t)
#else
  #define atomic_uint32_t volatile uint32_t
  #define atomic_int32_t  volatile int32_t
#endif

#if MCU_CORE_BYTE_WIDTH >= 2
  #define atomic_uint16_t SIZE_ALIGNED(uint16_t)
  #define atomic_int16_t  SIZE_ALIGNED(int16_t)
#else
  #define atomic_uint16_t volatile uint16_t
  #define atomic_int16_t  volatile int16_t
#endif

#define atomic_uint8_t volatile uint8_t
#define atomic_int8_t  volatile int8_t
/**@}*/
/**@}*/

#endif /* #ifndef BAPI_ATOMIC_H_ */
