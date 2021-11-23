/*
 * baseplate.h
 *
 *  Created on: 03.04.2013
 *      Author: e673505
 */

#ifndef BASEPLATE_H_
#define BASEPLATE_H_

/**
 * \file
 * \brief This file defines the basic types and macros that the honeycomb framework builds on.
 *
 * Some Macros are mapped to different compiler keywords depending the tool chain being used and
 * depending on the build configuration being used.
 *
 * An important macro is <b>C_INLINE</b> that maps to <b>static inline</b> for newer compiler versions,
 * but to <b>__inline__</b> for older gnu compilers. Hence the C_INLINE macro must be used for any
 * function that is an inline function.
 *
 * Another important macro is <b>STATIC</b> that maps to <b>static</b> for the release build,
 * but to nothing for the debug build. The gnu debugger cannot access static C variables and
 * functions. So static C variables and functions must be declared with the STATIC macro instead
 * of the static keyword, so that static C variables and functions can be debugged.
 *
 */

/*! @brief Define little endian */
#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN 1234
#endif

/*! @brief Define big endian */
#ifndef BIG_ENDIAN
#define BIG_ENDIAN    4321
#endif

#include "build-config.h" /* This must be the first include file. */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "target-rtos.h"
#include "hardware-board.h"

#include "utils/int.hpp"
//#include "../sdks/MCU_VENDOR_NXP/KSDK_2.6.0/CMSIS/Include/cmsis_iccarm.h"
#ifdef _DEBUG

#define DEBUG_CONSOLE_ENABLE		1

/* Make static variables available for debugger in debug
 * configuration by using STATIC rather than static
 * qualifier. */
#define STATIC static
#else
#define STATIC static
#endif


/** Prefix C_FUNC for declaration of pure C (non C++) functions */
#ifdef __cplusplus
  #define C_FUNC extern "C"
  #define C_DECL extern "C"
  #define C_INLINE static inline

#else
	#define C_FUNC extern
  #define C_DECL extern
  #if (__STDC_VERSION__ >= 199901L)
    #define C_INLINE static inline
  #elif __GNUC__
    /* Older gcc compilers support inline C functions via __inline__ */
    #define C_INLINE __inline__
  #else
    #error "The compiler does not support 'inline' for C code as part of the C99 standard."
  #endif
#endif


#ifdef __cplusplus
  #define S_CAST(type, data) static_cast<type>(data)
  #define R_CAST(type, data) reinterpret_cast<type>(data)
#else
  #define S_CAST(type, data) ((type)(data))
  #define R_CAST(type, data) ((type)(data))
#endif

/** Compiler specific settings */
#ifdef __GNUC__
  /*************************************************************/
  /*********************** GNU compiler ************************/
  /*************************************************************/
#define COMPILER_SUPPORTS_ZERO_LENGTH_ARRAY 1

/*
 * The section name within the linker script that defines the
 * section of the Flash Configuration for the Freescale MCU's.
 * See 'Flash configuration field description' in Freescale
 * Reference Manuals
 */
#define FS_FLASH_CONFIG_SECTION_NAME ".FlashConfig"

# define SIZEOF_INT   __SIZEOF_INT__
# define SIZEOF_SHORT __SIZEOF_SHORT__
# define SIZEOF_LONG  __SIZEOF_LONG__


#if __INT32_MAX__==2147483647L
  #define SIZEOF_INT32_T 4
#else
  #error "SIZEOF_INT32_T not defined yet for this architecture. Please look into the Compiler's built in macros via --predef_macros compiler flag."
#endif

#include <sys/types.h>

# define UNUSED(x) UNUSED_ ## x __attribute__((unused))

/* Attribute to avoid removal of non used variables by linker. */
# define USED __attribute__((used))

/* Declaration macro to place a variable in an individual section. */
# define AT_SECTION(S, VAR_DECL) __attribute__((section (S))) extern VAR_DECL

# define NORETURN __attribute__((noreturn))
# define ALIGNED(x) __attribute__((aligned(x)))
# define PACKED_ALIGNED(x) __attribute__((aligned(x), packed))
# define PACKED __attribute__((packed))

#define packed_struct(name) struct PACKED name
#define packed_array(name) PACKED name

#else


#ifdef __IAR_SYSTEMS_ICC__
  /*************************************************************/
  /*********************** IAR compiler ************************/
  /*************************************************************/
#define COMPILER_SUPPORTS_ZERO_LENGTH_ARRAY 0

/*
 * The section name within the linker script that defines the
 * section of the Flash Configuration for the Freescale MCU's.
 * See 'Flash configuration field description' in Freescale
 * Reference Manuals
 */
#define FS_FLASH_CONFIG_SECTION_NAME "FlashConfig"

#if __cplusplus >= 201103
  /* IAR claims to support 201103 standard, but it doesn't */
#undef __cplusplus
  #define __cplusplus 200300L
#endif

# define SIZEOF_INT __INT_SIZE__
# define SIZEOF_SHORT __SHORT_SIZE__
# define SIZEOF_LONG __LONG_SIZE__

#if   __INT_SIZE__ == 1
typedef int16_t ssize_t;
#elif __INT_SIZE__ == 2
typedef int16_t ssize_t;
#elif __INT_SIZE__ == 4
typedef int32_t ssize_t;
#else
typedef int64_t ssize_t;
#endif

#if __INT32_T_MAX__ == 2147483647L
  #define SIZEOF_INT32_T 4
#else
  #error "SIZEOF_INT32_T not defined yet for this architecture. Please look into the Compiler's built in macros via --predef_macros compiler flag."
#endif

#ifndef UINT32_MAX
  #define UINT32_MAX __UINT32_T_MAX__ /* Workaround 17/September/2015 WSC: It's not coming via #include <stdint.h> above? Strange !*/
#endif

# define UNUSED(x) x

/* Attribute to avoid removal of non used variables by linker. */
# define USED __root

/* Declaration macro to place a variable in an individual section. */
# define AT_SECTION(S, VAR_DECL) extern VAR_DECL @ S

#ifdef _eclipse_LANGUAGE_PROVIDER__
  # define NORETURN /* IAR Error parser does currently not support __noreturn attribute */
#else
  # define NORETURN __noreturn
#endif
# define ALIGNED(x) /* _Pragma("#pragma data_alignment = " UTILS_TO_STRING(x)) */
# define PACKED_ALIGNED(x) ALIGNED(x) PACKED

#ifndef PACKED
# define PACKED __packed
#endif

#define packed_struct(name) PACKED struct name
#define packed_array(name)  PACKED name

/** __IAR_SYSTEMS_ICC__: map iprintf to printf */
#ifdef _DEBUG
	#define iprintf printf
#else
	#define iprintf
#endif

/** __IAR_SYSTEMS_ICC__: map viprintf to vprintf */
#ifndef viprintf
  #define viprintf vprintf
#endif

/** __IAR_SYSTEMS_ICC__: map vsniprintf to vsnprintf */
#ifndef vsniprintf
  #define vsniprintf vsnprintf
#endif

/** __IAR_SYSTEMS_ICC__: map vsniprintf to vsnprintf */
#ifndef sniprintf
  #define sniprintf snprintf
#endif

#define STDIN_FILENO _LLIO_STDIN
#define STDOUT_FILENO _LLIO_STDOUT
#define STDERR_FILENO _LLIO_STDERR

#else
  /*************************************************************/
  /*********************** MSC compiler ************************/
  /*************************************************************/

// #define _SIZE_T_DEFINED


# define SIZEOF_INT sizeof(int)
# define UNUSED(x) x
# define NORETURN
# define ALIGNED(x) __declspec((align(x))
# define PACKED_ALIGNED(x) __declspec((align(x))

#endif /* #ifdef __IAR_SYSTEMS_ICC__ */
#endif /* #ifdef __GNUC__ */


#ifndef va_list__
  #define va_list__ va_list
#endif


#if !defined(__cplusplus) || (__cplusplus < 201103)
  # define nullptr (0) /* To support nullptr for C and C++ standard before 2011-03 */
#endif


  /* Useful Typedefs */
  /** \brief const byte pointer, useful to define a pointer to a const pointer.
   *  (e.g. const_byte_ptr_t * ptr2constBytePtr;)
   */
  typedef const uint8_t* const_byte_ptr_t;
  /** \brief type to carry milliseconds. */
  typedef uint32_t MsecType;

  /* Typedefs to map legacy data types used to the standard ones */
//   typedef unsigned long UINT32;
//   typedef unsigned char UINT8;
   typedef unsigned char BYTE;
//   typedef unsigned short UINT16;
   typedef unsigned short WORD;
   typedef uint32_t DWORD;
   typedef float  FLOAT32;


   // Useful Macros
   #define MSEC_PER_SEC ((MsecType)(1000))
   #define MSEC_MAX (~((MsecType)(0)))


/**
 * Preprocessor macro for the processor core byte width:
 *    8 bit processor -> 1 byte processor
 *   16 bit processor -> 2 byte processor
 *   32 bit processor -> 4 byte processor
 *   64 bit processor -> 8 byte processor
 */
#define MCU_CORE_BYTE_WIDTH (SIZEOF_INT)

/**
 * \brief Macro to declare a type that is address-aligned with it's size.
 *
 * Examples:
 *  if the type is uint16_t is will be placed add an even address.
 *  if the type is uint32_t it will be placed at and address that
 *    can be divided by 4 without rest.
 */
#define SIZE_ALIGNED(type) ALIGNED(sizeof(type)) type

/** The alignment for the Persistent POD structures */
#define PPOD_ALIGNMENT (sizeof(int))
#define PPOD_ALIGNED ALIGNED(PPOD_ALIGNMENT)

#if (__cplusplus >= 201103L)
  #define PPOD_TEST
#endif

/* Error detection macros
 *
 */


/* Use ASSERT to catch coding bugs which should be fixed before release */
#ifdef _DEBUG
  #define ASSERT(expr) if(!(expr)) { bapi_fatalError(__FILE__, __LINE__); }

  /* ASSERT in debug compilation, otherwise do nothing. */
  #define ASSERT_DEBUG(expr) ASSERT(expr)
#else
  #define ASSERT(expr) if(!(expr)) { bapi_fatalError(0, 0); }

  /* ASSERT in debug compilation, otherwise do nothing. */
  #define ASSERT_DEBUG(expr)
#endif

#define ASSERT_FROM_ISR(expr) \
  ASSERT(expr)

/* Use SYSLOG to log runtime errors (e.g. no memory, unexpected message
 * received etc.) */
#define SYSLOG(expr) if(!(expr)) { bapi_fatalError(__FILE__, __LINE__); }

#define SYSLOG_FROM_ISR(expr) \
  SYSLOG(expr)

/*
 * IAR language provider workarounds. IAR has problems with size_t
 *
 * Note: The cmake macro _create_language_provider_ invokes the compiler for the indexer with -D_eclipse_LANGUAGE_PROVIDER__
 *
 */
#if defined(__IAR_SYSTEMS_ICC__) && defined(_eclipse_LANGUAGE_PROVIDER__)

  /* Should never be called, is just a dummy to workaround the IAR language provider problem with size_t */
  C_INLINE void * _util_memset(void* d, int c, unsigned n) {
    ASSERT(false);
    return d;
  }

  /* Use MEMSET instead of memset to avoid wrong memset bug tags due to size_t problems. */
  #define MEMSET(dst, value, n) _util_memset(dst, value, S_CAST(unsigned, n))

  /* Should never be called, is just a dummy to workaround the IAR language provider problem with size_t */
  C_INLINE void * _util_memcpy(void* d, const void* s, unsigned n) {
    ASSERT(false); // Should never be called, is just a dummy to workaround the IAR language provider problem with size_t
    return d;
  }

  /* Use MEMCPY instead of memcpy to avoid wrong memcpy bug tags due to size_t problems. */
  #define MEMCPY(dst, src, n) _util_memcpy(dst, src, S_CAST(unsigned, n))

  /* Should never be called, is just a dummy to workaround the IAR language provider problem with size_t */
  C_INLINE int _util_memcmp(const void* m1, const void* m2, unsigned n) {
    ASSERT(false); // Should never be called, is just a dummy to workaround the IAR language provider problem with size_t
    return 0;
  }

  /* Use MEMCMP instead of memcmp to avoid wrong memcmp bug tags due to size_t problems. */
  #define MEMCMP(m1, m2, n)   _util_memcmp(m1, m2, S_CAST(unsigned, n))

#else /* defined(__IAR_SYSTEMS_ICC__) && defined(_eclipse_LANGUAGE_PROVIDER__) */

  #define MEMSET(dst, value, n) memset(dst, value, n)
  #define MEMCPY(dst, src, n)   memcpy(dst, src, n)
  #define MEMCMP(m1, m2, n)     memcmp(m1, m2, n)

#endif /* defined(__IAR_SYSTEMS_ICC__) && defined(_eclipse_LANGUAGE_PROVIDER__) */

/*
 * IAR language provider workarounds. IAR has problems with va_list__
 *   This is a try, but does not 100% solve the problem.
 * Note: The cmake macro _create_language_provider_ invokes the compiler for the indexer with -D_eclipse_LANGUAGE_PROVIDER__
 *
 */
#if defined(__IAR_SYSTEMS_ICC__) && defined(_eclipse_LANGUAGE_PROVIDER__)
  /** This is a workaround for the indexer running under __IAR_SYSTEMS_ICC__
   *   va_start resolves to _VA_START, that isn't defined when the indexer
   *   is running. */
  #ifndef _VA_START
    #define _VA_START(a,b)
  #endif
  /** This is a workaround for the indexer running under __IAR_SYSTEMS_ICC__
   *   va_end resolves to _VA_END, that isn't defined when the indexer
   *   is running. */
  #ifndef _VA_END
    #define _VA_END(a)
  #endif
  /** This is a workaround for the indexer running under __IAR_SYSTEMS_ICC__
   *   va_list resolves to _VA_LIST, that isn't defined when the indexer
   *   is running. */
  #define va_list__ int
  int vprintf__(const char *_Restrict, va_list__);
  #define viprintf  vprintf__
  #define vprintf   vprintf__

  int vsnprintf__(char *_Restrict, size_t, const char *_Restrict,  va_list__);
  #define vsniprintf  vsnprintf__
  #define vsnprintf   vsnprintf__

  /** This is a workaround for the indexer running under __IAR_SYSTEMS_ICC__.
   * We need to map __packed to nothing, because the IAR error parser seems
   * not to support the __packed keyword. */
  #define __packed

  /** This is a workaround for the indexer running under __IAR_SYSTEMS_ICC__
   * _Filet resolves to void*, because it isn't defined when the indexer is running.
   * _Filet is the data type for FILE* which is e.g. required for setvbuf().
   */
  #define _Filet void*
#endif /* defined(__IAR_SYSTEMS_ICC__) && defined(_eclipse_LANGUAGE_PROVIDER__) */


/** --- Headerfile for PRODUCT --- */
#include "product_config.h"
#include "config.h"


#ifndef DEFAULT_CONSOLE_BAUDRATE
  #error "DEFAULT_CONSOLE_BAUDRATE not defined in the product configuration header file."
#endif

#ifndef BAPI_RQ_SYSTEM_TICK_FREQUENCY_HZ
  #error "BAPI_RQ_SYSTEM_TICK_FREQUENCY_HZ not defined in the product configuration header file."
#endif

#ifndef TARGET_RTOS
  #error "TARGET_RTOS not defined in the product configuration header file. Use RTOS_NoRTOS if you don't use an OS."
#endif

#include "boards/board-api/bapi_common.h"

#endif /* BASEPLATE_H_ */
