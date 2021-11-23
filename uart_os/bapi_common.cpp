/*
 * bapi_isr.c
 *
 *  Created on: 10.04.2013
 *      Author: e673505
 */

/** \file
 * \brief
 * This file implements the common board API interface functions/variables that are declared
 * in bapi_common.h and are common to all hardware boards.
 */

#include <baseplate.h>
#include <baseplate_mcu_vendors.h>
#include <hardware-board.h>
//#include <stdio.h>
#include "boards/board-api/bapi_common.h"
#include <cstdbool>
#include <cstdint>

#if MCU_VENDOR == MCU_VENDOR_SILABS

  #include "em_device.h"

#elif MCU_VENDOR == (MCU_VENDOR_FREESCALE) || (MCU_VENDOR == MCU_VENDOR_NXP)

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

  #include "fsl_device_registers.h"

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif

#else
  #error "Fatal Error: MCU Vendor."
#endif

#include "utils/utils.h"


/* WSC: Wondering why this is not available from CMSIS core header files. */
#if defined(K64F12_SERIES )
  /* 1MB Flash, 256KB RAM */
  #define RAM_START R_CAST(const uint8_t*, 0x1FFF0000)
  #define RAM_END   R_CAST(const uint8_t*, 0x20030000)

  #define FLASH_START R_CAST(const uint8_t*, 0x00000410)
  #define FLASH_END   R_CAST(const uint8_t*, 0x00100000)

#elif defined (K24F12_SERIES)

  /* 1MB Flash, 256KB RAM */
  #define RAM_START R_CAST(const uint8_t*, 0x1FFF0000)
  #define RAM_END   R_CAST(const uint8_t*, 0x20030000)

  #define FLASH_START R_CAST(const uint8_t*, 0x00000410)
  #define FLASH_END   R_CAST(const uint8_t*, 0x00100000)

#elif defined (KL46Z4_SERIES)

#if defined(CPU_MKL46Z256VLL4) || defined(CPU_MKL46Z256VMC4) || defined(CPU_MKL46Z256VMP4) || defined(CPU_MKL46Z256VLH4)
  /* 256KB Flash, 32KB RAM */
  #define RAM_START R_CAST(const uint8_t*, 0x1FFFE000)
  #define RAM_END   R_CAST(const uint8_t*, 0x20006000)

  #define FLASH_START R_CAST(const uint8_t*, 0x00000410)
  #define FLASH_END   R_CAST(const uint8_t*, 0x00040000)

#elif defined(CPU_MKL46Z128VLH4) || defined(CPU_MKL46Z128VMC4) || defined(CPU_MKL46Z128VLL4)
  /* 128KB Flash, 16KB RAM */
  #define RAM_START R_CAST(const uint8_t*, 0x1FFFF000)
  #define RAM_END   R_CAST(const uint8_t*, 0x20003000)

  #define FLASH_START R_CAST(const uint8_t*, 0x00000410)
  #define FLASH_END   R_CAST(const uint8_t*, 0x00020000)

#endif

#elif defined (KL17Z4_SERIES)

  /* 32KB RAM */
  #define RAM_START R_CAST(const uint8_t*, 0x1FFFE000)
  #define RAM_END   R_CAST(const uint8_t*, 0x20006000)

#if (defined(CPU_MKL17Z256VLH4) || defined(CPU_MKL17Z256VFT4) || defined(CPU_MKL17Z256VFM4) || defined(CPU_MKL17Z256VMP4))
  /* 256KB Flash, 32KB RAM */
  #define FLASH_START R_CAST(const uint8_t*, 0x00000410)
  #define FLASH_END   R_CAST(const uint8_t*, 0x00040000)

#elif (defined(CPU_MKL17Z128VFM4) || defined(CPU_MKL17Z128VFT4) ||  defined(CPU_MKL17Z128VMP4) || defined(CPU_MKL17Z128VLH4))
  /* 128KB Flash, 32KB RAM */
  #define FLASH_START R_CAST(const uint8_t*, 0x00000410)
  #define FLASH_END   R_CAST(const uint8_t*, 0x00020000)

#endif

#elif defined (K66F18_SERIES)

  /* 256KB RAM */
  #define RAM_START R_CAST(const uint8_t*, 0x1FFF0000)
  #define RAM_END   R_CAST(const uint8_t*, 0x20030000)

#if (defined(CPU_MK66FN2M0VLQ18) || defined(CPU_MK66FN2M0VMD18))

  /* 2MB Flash */
  #define FLASH_START R_CAST(const uint8_t*, 0x00000410)
  #define FLASH_END   R_CAST(const uint8_t*, 0x00200000)

#elif (defined(CPU_MK66FX1M0VLQ18) || defined(CPU_MK66FX1M0VMD18))

  /* 1MB Flash (currently not considering FlexNVM) */   // TODO: Consider FlexNVM ?
  #define FLASH_START R_CAST(const uint8_t*, 0x00000410)
  #define FLASH_END   R_CAST(const uint8_t*, 0x00100000)

#endif
#elif (defined(CPU_MIMXRT1062CVL5A))
 /* 1 MB RAM */
  #define RAM_START R_CAST(const uint8_t*, 0x20200000)
  #define RAM_END   R_CAST(const uint8_t*, 0x20207FFF)

  #define FLASH_START R_CAST(const uint8_t*, 0x0)
  #define FLASH_END   R_CAST(const uint8_t*, 0x0)

#elif (defined(CPU_MIMXRT1051CVJ5B))
 /* ? MB RAM */ /*Need TBD*/ 
  #define RAM_START R_CAST(const uint8_t*, 0x20200000)
  #define RAM_END   R_CAST(const uint8_t*, 0x20207FFF)

  #define FLASH_START R_CAST(const uint8_t*, 0x0)
  #define FLASH_END   R_CAST(const uint8_t*, 0x0)

#else
 #error "Memory map not defined for current MCU."
#endif

#define RAM_BLOCK_SIZE   (RAM_END - RAM_START)
#define FLASH_BLOCK_SIZE (FLASH_END - FLASH_START)

C_FUNC bool bapi_isValidMemblock(const uint8_t *memblock, unsigned size) {
  if(memblock >= RAM_START && memblock + size < RAM_END) {
    return true;
  }
  if(memblock >= FLASH_START && memblock + size < FLASH_END) {
    return true;
  }

  /*
   * Zero length block is treated as valid memory block, independent of
   * the memory block pointer.
   */
  return (size == 0);
}

C_FUNC unsigned bapi_snprintmem(const uint8_t *memblock, unsigned size, char *buffer, unsigned buffersize) {

  unsigned retval = 0;
  if (size) {
    if (buffersize) {

      /* Test Address range of data pointer. */
      if ((memblock != NULL) && (memblock >= RAM_START && memblock < RAM_END)) {

        if(size <= RAM_BLOCK_SIZE) {
          retval = _snprintmem_(memblock, size, buffer, buffersize);
        } else {
          retval = sniprintf(buffer, buffersize, "Invalid Size for: %p", memblock);
        }

      } else {

        if ((memblock != NULL) && (memblock >= FLASH_START && memblock < FLASH_END)) {

          if(size <= FLASH_BLOCK_SIZE) {
            retval = _snprintmem_(memblock, size, buffer, buffersize);
          } else {
            retval = sniprintf(buffer, buffersize, "Invalid Size for: %p", memblock);
          }

        } else {
          retval = sniprintf(buffer, buffersize, "Invalid Pointer: %p", memblock);
        }
      }
    }
  }
  else {
    retval = sniprintf(buffer, buffersize, "No Data");
  }

  return retval;
}

