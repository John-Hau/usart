/*
 * osaUtils.h
 *
 *  Created on: 21.04.2013
 *      Author: Wolfgang
 */

#ifndef UTILS_H_
#define UTILS_H_

#include "baseplate.h"
#include "stdint.h"

/** ENDOF_MEMBER: Return the offset that points behind the member of a structure.
 */
#define ENDOF_MEMBER(structure, member) \
  offsetof(structure, member) + sizeof( R_CAST(const structure*, 0)->member )


/** ARRAY_SIZE:
 *  Calculates the number of items in a static array
 */

#define ARRAY_SIZE_TYPED(a, T) \
    ((T)(sizeof(a)/sizeof(a[0])))


#if defined(__IAR_SYSTEMS_ICC__) && defined(_eclipse_LANGUAGE_PROVIDER__)

#ifndef ARRAY_SIZE
  #define ARRAY_SIZE(a) ARRAY_SIZE_TYPED(a, unsigned)
#endif

#else /* defined(__IAR_SYSTEMS_ICC__) && defined(_eclipse_LANGUAGE_PROVIDER__) */

#ifndef ARRAY_SIZE
  #define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))
#endif

#endif /* defined(__IAR_SYSTEMS_ICC__) && defined(_eclipse_LANGUAGE_PROVIDER__) */

#define MEMBER_SIZE(type, member) sizeof(R_CAST(const type *, 0)->member)

#ifndef MIN
   #define MIN(a, b) \
     ((a) < (b) ? (a) : (b))
#endif


#ifndef MAX
  #define MAX(a, b) \
    ((a) > (b) ? (a) : (b))
#endif

#define UTILS_STRINGIFY(x) #x
#define UTILS_TO_STRING(x) UTILS_STRINGIFY(x)

#if defined(_eclipse_LANGUAGE_PROVIDER__)
  #define OFFSETOF(T, member) \
    R_CAST( size_t , &((R_CAST(T*,0))->member) )
#else
  #include <stddef.h>
  #define OFFSETOF(T, member) \
    offsetof(T, member)
#endif

/* Empty macro detection */
#define _NO_OTHER_MACRO_STARTS_WITH_THIS_PREFIX_ (1)

#define FIRST_ARG_PREFIXED(first, ...) \
  _NO_OTHER_MACRO_STARTS_WITH_THIS_PREFIX_ ## first

#define MACRO_PREFIXED(macro...) \
  FIRST_ARG_PREFIXED(macro)

#define IS_NONEMPTY_MACRO(macro) \
  (MACRO_PREFIXED(macro) != _NO_OTHER_MACRO_STARTS_WITH_THIS_PREFIX_)

/**
 * Compare a memory area with a particular value.
 * @return 0 if the complete memory area matches
 *   _val_. Otherwise -1.
 */
C_INLINE int memvcmp(const uint8_t * memory, const uint8_t val, const size_t size)
{
  size_t i = 0;
  for(;i < size; i++) {
    if(memory[i] != val) {
      return -1;
    }
  }
  return 0;
}

C_INLINE unsigned __snprintmem__(const uint8_t* memblock,
  unsigned size, char* buffer, unsigned buffersize, const char hex_str[]) {
  unsigned j = 0;
  {
    unsigned i = 0;
    while ((i < size) && (j + 1 < buffersize)) {
      buffer[j] = hex_str[(memblock[i] >> 4) & 0x0F];
      j++;
      if (j + 1 < buffersize) {
        buffer[j] = hex_str[memblock[i] & 0x0F];
        j++;
      }
      i++;
    }
  }
  buffer[j] = 0;
  return j;
}

C_INLINE unsigned __rsnprintmem__(const uint8_t* memblock,
  unsigned size, char* buffer, unsigned buffersize, const char hex_str[]) {
  unsigned j = 0;
  {
    unsigned i = size;
    while ((i > 0) && (j + 1 < buffersize)) {
      i--;
      buffer[j] = hex_str[(memblock[i] >> 4) & 0x0F];
      j++;
      if (j + 1 < buffersize) {
        buffer[j] = hex_str[memblock[i] & 0x0F];
        j++;
      }
    }
  }
  buffer[j] = 0;
  return j;
}

/**
 * \brief prints the contents of memory block as hex string
 *   into memory.
 *
 * @param memblock [in] pointer to the memory block to be printed
 * @param size     [in] the size of the memory block to be printed
 * @param buffer   [in] the buffer where to print
 * @param buffer   [in] the size of the buffer
 *
 * @return number of printed characters.
 */
C_INLINE unsigned _snprintmem_(const uint8_t* memblock,
  unsigned size, char* buffer, unsigned buffersize) {
  return __snprintmem__(memblock, size, buffer, buffersize,
    "0123456789abcdef");
}


/**
 * \brief prints the contents of memory block as hex string
 *   into memory.
 *
 *  This is a non-inline function which will consume less
 *  memory than function \ref _snprintmem_ when called more than
 *  once in a client application. The disadvantage is,
 *  that it requires utils library to be linked.
 *
 * @param memblock [in] pointer to the memory block to be printed
 * @param size     [in] the size of the memory block to be printed
 * @param buffer   [in] the buffer where to print
 * @param buffer   [in] the size of the buffer
 *
 * @return number of printed characters.
 */
C_FUNC unsigned snprintmem(const uint8_t* memblock, unsigned size,
  char* buffer, unsigned buffersize);


/**
 * \brief prints the contents of memory block as hex string
 *  into memory in reverse byte order.
 *
 * @param memblock [in] pointer to the memory block to be printed
 * @param size     [in] the size of the memory block to be printed
 * @param buffer   [in] the buffer where to print
 * @param buffer   [in] the size of the buffer
 *
 * @return number of printed characters.
 */
C_INLINE unsigned _rsnprintmem_(const uint8_t* memblock,
  unsigned size, char* buffer, unsigned buffersize) {
  return __snprintmem__(memblock, size, buffer, buffersize,
    "0123456789abcdef");
}

/**
 * \brief prints the contents of memory block as hex string
 *  into memory in reverse byte order.
 *
 *  This is a non-inline function which will consume less
 *  memory than function \ref _rsnprintmem_ when called more than
 *  once in a client application. The disadvantage is,
 *  that it requires utils library to be linked.
 *
 * @param memblock [in] pointer to the memory block to be printed
 * @param size     [in] the size of the memory block to be printed
 * @param buffer   [in] the buffer where to print
 * @param buffer   [in] the size of the buffer
 *
 * @return number of printed characters.
 */
C_FUNC unsigned rsnprintmem(const uint8_t* memblock, unsigned size,
  char* buffer, unsigned buffersize);

#ifdef  __IAR_SYSTEMS_ICC__

#if (__CORE__ == __ARM6M__)

  C_INLINE uint8_t countLeadingZeroesUint32(uint32_t x) {
    uint8_t retval = 0;
    while(!(x & 0x80000000)) {
      retval++;
      x <<= 1;
    }
    return retval;
  }

#else

  #include "intrinsics.h"
  C_INLINE uint8_t countLeadingZeroesUint32(uint32_t x) {
    return __CLZ(x) + (8 * (sizeof(uint32_t) - sizeof(unsigned long)));
  }


#endif

#elif __GNUC__

  C_INLINE uint8_t countLeadingZeroesUint32(uint32_t x) {
    return __builtin_clz(x)  + (8 * (sizeof(uint32_t) - sizeof(unsigned int)));
  }

#endif

C_INLINE uint8_t countLeadingZeroesUint16(uint16_t x) {
  return countLeadingZeroesUint32(x) - 16;
}

C_INLINE uint8_t countLeadingZeroesUint8(uint16_t x) {
  return countLeadingZeroesUint32(x) - 24;
}

#ifdef __cplusplus

template<typename T> inline uint8_t countLeadingZeroes(T x);


template<> inline uint8_t countLeadingZeroes<uint32_t>(uint32_t x) {
  return countLeadingZeroesUint32(x);
}

template<> inline uint8_t countLeadingZeroes<uint16_t>(uint16_t x) {
  return countLeadingZeroesUint16(x);
}

template<> inline uint8_t countLeadingZeroes<uint8_t>(uint8_t x) {
  return countLeadingZeroesUint8(x);
}

#ifdef  __IAR_SYSTEMS_ICC__
template<> inline uint8_t countLeadingZeroes<unsigned long>(unsigned long x) {
  return countLeadingZeroesUint32(x);
}
#endif

//#if __IAR_SYSTEMS_ICC__
//template<> inline uint8_t countLeadingZeroes<unsigned short>(unsigned short x) {
//  return countLeadingZeroesUint16(x);
//}
//
//template<> inline uint8_t countLeadingZeroes<unsigned char>(unsigned char x) {
//  return countLeadingZeroesUint8(x);
//}
//#endif

//template<> inline uint8_t countLeadingZeroes<int32_t>(int32_t x) {
//  return countLeadingZeroesUint32(x);
//}
//
template<> inline uint8_t countLeadingZeroes<int16_t>(int16_t x) {
  return countLeadingZeroesUint16(x);
}
//
//template<> inline uint8_t countLeadingZeroes<int8_t>(int8_t x) {
//  return countLeadingZeroesUint8(x);
//}
//
//#ifdef  __IAR_SYSTEMS_ICC__
template<> inline uint8_t countLeadingZeroes<long>(long x) {
  return countLeadingZeroesUint32(x);
}
//#endif

//#if __IAR_SYSTEMS_ICC__
//template<> inline uint8_t countLeadingZeroes<short>(short x) {
//  return countLeadingZeroesUint16(x);
//}
//#endif
//
//template<> inline uint8_t countLeadingZeroes<char>(char x) {
//  return countLeadingZeroesUint8(x);
//}

#endif /* __cplusplus */

#ifdef __cplusplus

#include <limits>

namespace utils {

/*
 * Moves the pointer to an object behind this object
 * assuming that the object has a size() method, that
 * returns the size of the current project in bytes.
 *
 * pT pointer type of the object
 *
 * @param[in] p the pointer to the object.
 */
template<typename T> T* nextT(T* p) {
  const size_t size = p->size();
  return R_CAST(T*, R_CAST(uint8_t*, p) + size);
}

template<typename T> const T* nextT(const T* p) {
  const size_t size = p->size();
  return R_CAST(const T*, R_CAST(const uint8_t*, p) + size);
}

/* 
 * Check if a value is within a range that is given by 2 compile time constants.
 * Using Interval::isWithin will avoid compiler complains that is about comparing
 * an unsigned value against being >=0. The specialization of Interval
 * with lowestValue being 0 will not do this comparison.
 */
template<typename T, int lowestValue, int highestValue, bool isSigned = std::numeric_limits<T>::is_signed>
struct Interval {
  static bool isWithin(T value) {
    return (value >= lowestValue) && (value<=highestValue);
  }
};

/* Specialization for unsigned T with lowestValue being 0: Just check against highest value. */
template<typename T, int highestValue>
struct Interval<T, 0, highestValue, false> {
  static bool isWithin(T value) {
    /* Need to check only for highest value, because value is unsigned and cannot be below 0. */
    return (value<=highestValue);
  }
};


/**
 * \brief This structure defines memory blocks in a manner that it
 * can be passed as template parameters for \ref struct IS_OVERLAPPING.
 */
template<unsigned startAddr, unsigned blockLen> struct MEMORY_BLOCK {
  enum {START = startAddr, LEN = blockLen};
};



/**
 * \brief This structure provides compile time overlap information
 *   of 2 memory blocks.
 * IS_OVERLAPPING::value becomes 1 if memory block 1 and memory block 2
 *   are overlapping. Otherwise 0. The 2 memory blocks are defined by
 *   the intervals:
 *     - [B1::START ... B1::START + B1::LEN[
 *     - [B2::START ... B2::START + B2::LEN[
 *
 * That means that B1 and B2 must be structures or classes that have
 *  compile time constants START and LEN.
 *
 * __Code Example1:__
 ~~~~~~~~{.c}
 #include "utils\utils.h"

 struct memblock1 {enum {START = 0x1000, LEN=0x3000};};
 struct memblock2 {enum {START = 0x2000, LEN=0x1000};};

 bool overlap = utils::IS_OVERLAPPING<memblock1, memblock2>::value;
 *
 ~~~~~~~~
 *
 *
 * __Code Example2:__
 ~~~~~~~~{.c}
 #include "utils\utils.h"

 typedef utils::MEMORY_BLOCK<0x1000, 0x3000> memblock1;
 typedef utils::MEMORY_BLOCK<0x2000, 0x1000> memblock2;

 bool overlap = utils::IS_OVERLAPPING<memblock1, memblock2>::value;
 ~~~~~~~~
 *
 */
template<typename B1, typename B2> struct IS_OVERLAPPING {
 enum {value = B2::START >= (B1::START + B1::LEN) ? 0 : B1::START >= (B2::START + B2::LEN) ? 0 : 1 };
};


/**
 * \brief This function provides runtime time overlap information
 *   of 2 memory blocks. The first block B is defined by compile
 *   time template parameters, while the second block R is defined
 *   by runtime parameters start an len.
 *
 * The 2 memory blocks are defined by
 *   the intervals:
 *     - [B::START ... B::START + B::LEN[
 *     - [start    ... start + len[
 *
 * That means that B needs to be a structure or class that must have
 *  compile time constants START and LEN.
 *
 * @return true if memory block B and memory block R are overlapping.
 *   Otherwise false.
 *
 * __Code Example:__
 ~~~~~~~~{.c}
 #include "utils\utils.h"

 typedef utils::MEMORY_BLOCK<0x1000, 0x3000> memblock;
 bool overlap = utils::isOverlapping<memblock>(0x2000, 0x1000);
 ~~~~~~~~
 *
 */
template<typename B> bool ctIsOverlapping(uint32_t start, uint32_t len) {
  return start >= (B::START + B::LEN) ? 0 : B::START >= (start + len) ? false : true;
};


/**
 * \brief This function provides runtime time overlap information
 *   of 2 memory blocks. Both blocks are defined by runtime
 *   parameter start1, len1 and start1, len2.
 *
 * The 2 memory blocks are defined by
 *   the intervals:
 *     - [start2    ... start2 + len2[
 *     - [start2    ... start2 + len2[
 *
 *
 * @return true if memory block B1 and memory block B2 are overlapping.
 *   Otherwise false.
 *
 * __Code Example:__
 ~~~~~~~~{.c}
 #include "utils\utils.h"

 typedef utils::MEMORY_BLOCK<0x1000, 0x3000> memblock;
 bool overlap = utils::isOverlapping(0x1000, 0x3000, 0x2000, 0x1000);
 ~~~~~~~~
 *
 */
C_INLINE bool rtIsOverlapping(uint32_t start1, uint32_t len1, uint32_t start2, uint32_t len2) {
  return start1 >= (start2 + len2) ? 0 : start2 >= (start2 + len2) ? false : true;
};


}

#endif

#endif /* UTILS_H_ */
