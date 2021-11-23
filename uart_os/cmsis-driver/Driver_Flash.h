/* -----------------------------------------------------------------------------
 * Copyright (c) 2013-2014 ARM Ltd.
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software. Permission is granted to anyone to use this
 * software for any purpose, including commercial applications, and to alter
 * it and redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software in
 *    a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 *
 *
 * $Date:        31. Mar 2014
 * $Revision:    V2.00
 *
 * Project:      Flash Driver definitions
 * -------------------------------------------------------------------------- */

/* History:
 *  Version 2.00
 *    Renamed driver NOR -> Flash (more generic)
 *    Non-blocking operation
 *    Added Events, Status and Capabilities
 *    Linked Flash information (GetInfo)
 *  Version 1.11
 *    Changed prefix ARM_DRV -> ARM_DRIVER
 *  Version 1.10
 *    Namespace prefix ARM_ added
 *  Version 1.00
 *    Initial release
 */

#ifndef __CMSIS_DRIVER_FLASH_H
#define __CMSIS_DRIVER_FLASH_H

#include "baseplate.h"
#include "boards/board-api/bapi_flash.h"
#include "boards/cmsis/Driver_Flash.h"


/**
 * \file
 * \brief
 * This file provides the standard CMSIS FLASH API as well as ARM_DRIVER_FLASH
 * supplementary functionality. The API is provided by the include of
 * the header file boards/cmsis/Driver_Flash.h
 */



/**
 * \addtogroup cmsis_driver_flash
 */
/**@{*/

/**
 * 26/Jun./2016 WSC: Added additional parameter bapi_E_UartIndex_ and commandID to the callback function type ARM_Flash_SignalEvent_t.
 * This was the only way to integrate the ARM_DRIVER_FLASH API easily into honeycomb. All other tried solution became
 * too complicated.
 */
typedef void (*ARM_Flash_SignalEvent_t) (enum bapi_E_FlashDevice flashDeviceIndex, enum bapi_flash_E_Command_ID_ commandID
  , uint32_t event);    ///< Pointer to \ref ARM_Flash_SignalEvent : Signal Flash Event.


/**
\brief Access structure of the Flash Driver
*/
struct _ARM_DRIVER_FLASH {
  /**
   * \brief Get driver version.
   *
   * Returns version information of the driver implementation in ARM_DRIVER_VERSION
   *   - API version is the version of the CMSIS-Driver specification used to implement this driver.
   *   - Driver version is source code version of the actual driver implementation.
   *
   * \return The API Version and Driver Version in an _ARM_DRIVER_VERSION structure.
   *
   Example:
   ~~~~~~~~{.c}
   void setup_flash (void) {
     ARM_DRIVER_FLASH *drv = driver_flash_getDriver(bapi_E_FlashDev0, 0);
     ARM_DRIVER_VERSION  version = drv->GetVersion ();
     if (version.api < 0x10A) {      // requires at minimum API version 1.10 or higher
       // error handling
       return;
     }
   }
   ~~~~~~~~
   */
  ARM_DRIVER_VERSION      (*GetVersion)     (void);

  /**
    * \brief Get driver capabilities.
    * \return \ref _ARM_FLASH_CAPABILITIES
    *
    * Retrieves information about capabilities in this driver implementation.
    * The bitfield members of the struct _ARM_DRIVER_CAPABILITIES encode various
    * capabilities, for example if a hardware is capable to
    * create signal events using the \ref ARM_DRIVER_SignalEvent callback
    * function.
    *
   Example:
   ~~~~~~~~{.c}
   void read_capabilities (void)  {
     ARM_DRIVER_FLASH *drv = driver_flash_getDriver(bapi_E_FlashDev0, 0);
     ARM_DRIVER_CAPABILITIES drv_capabilities = drv->GetCapabilities();
     // interrogate capabilities

   }
   ~~~~~~~~
    *
    *
    */
  ARM_FLASH_CAPABILITIES  (*GetCapabilities)(void);


  /**
   * \brief Initialize FLASH Interface.
   *
   * The function initializes the FLASH interface of a flash partition. It is called when the
   * middleware component starts operation.
   *
   * @param [in] cb_event Pointer to a callback function with the signature of
   * \ref ARM_FLASH_SignalEvent
   *
   * \return #ARM_DRIVER_OK upon success. #ARM_DRIVER_ERROR, if the driver
   * is already initialized with a different cb_event.
   *
   * The function performs the following operations:
   *  - Initializes the resources needed for the FLASH interface.
   *  - Registers the ARM_FLASH_SignalEvent callback function.
   *
   * The parameter cb_event is a pointer to the callback function with the
   * signature of \ref ARM_FLASH_SignalEvent; use a NULL pointer when no
   * callback signals are required.
   *
   */
  int32_t                 (*Initialize)     (ARM_Flash_SignalEvent_t cb_event);


  /**
   * \brief De-initialize USART Interface.
   *
   * \return Common \ref cmsis_driver_general_return_codes "Status Error Codes"
   *
   * The function de-initializes the resources of
   * FLASH interface. It is called when the middleware component stops
   * operation and releases the software resources used by the interface.
   */
  int32_t                 (*Uninitialize)   (void);

  /**
    * \brief Control FLASH Interface Power.
    *
    * @param [in] state Power state
    * \return \ref cmsis_driver_general_return_codes "Status Error Codes"
    *
    * Allows you to control the power modes of the FLASH interface.
    */
  int32_t                 (*PowerControl)   (ARM_POWER_STATE state);

  /**
   * \brief Read data from Flash.
   *
   * @param [in]  addr Flash Partition Data address __aligned with data type__.
   * @param [out] data Pointer to a buffer storing the data read from Flash.
   * @param [in] cnt Number of __data items (not bytes)__ to read.
   * @return Status Error Codes
   *
   * This function reads data from the Flash device.
   * The parameter _addr_ specifies the address from where to read data (needs
   * to be aligned to data type size). The parameter _data_ specifies the pointer
   * to a buffer storing the data read. The data type is _uint8_t_, _uint16_t_
   * _uint32_t or uint64_t_ and is specified by the _data_width_ in
   * \ref #ARM_FLASH_CAPABILITIES. The parameter _cnt_ specifies the number of data
   * items to read.
   *
   * The function executes in the following way:
   *   -) When the operation is non-blocking (typical for SPI Flash) then the
   *   function only starts the operation and returns with zero number of data
   *   items read. When the operation is completed the ref #ARM_FLASH_EVENT_READY
   *   event is generated (if supported and reported by GetCapabilities). In
   *   case of errors the \ref #ARM_FLASH_EVENT_ERROR event is generated at the same
   *   time. Progress of the operation can also be monitored by calling the
   *   GetStatus function and checking the busy flag.
   *
   *   -) When the operation is blocking (typical for memory mapped Flash) then
   *    the function returns after the data is read and returns the number of
   *    data items read.
   *
   * \note The Honeycomb implementation only follows the non-blocking approach.
   *
   */
  int32_t                 (*ReadData)       (uint32_t addr,       void *data, uint32_t cnt);

  /**
   * \brief Program data to Flash.
   *
   * @param[in] addr Data address.
   * @param[in] data Pointer to a buffer containing the data to be programmed to Flash.
   * @param[in] cnt Number of data items to program.
   * @return number of data items programmed or Status Error Codes
   *
   * This function programs data to the Flash device.
   * The parameter _addr_ specifies the address to where to program data (needs to be
   *  aligned to program_unit specified in ARM_FLASH_INFO).
   * The parameter _data_ specifies the pointer to a buffer containing data to be
   * programmed. The data type is uint8_t, uint16_t, uint32_t or uint64_t and is
   * specified by the _data_width_ in ARM_FLASH_CAPABILITIES. The parameter _cnt_ specifies
   * the number of data items to program (data size needs to be a multiple of _program_unit_).
   *
   * The function executes in the following ways:
   *  - When the operation is non-blocking (typically) then the function only starts the
   *      operation and returns with zero number of data items programmed. When the
   *      operation is completed the ARM_FLASH_EVENT_READY event is generated (if supported
   *      and reported by ARM_Flash_GetCapabilities). In case of errors the
   *      ARM_FLASH_EVENT_ERROR event is generated at the same time. Progress of the
   *      operation can also be monitored by calling the ARM_Flash_GetStatus function and
   *      checking the busy flag.
   *  - When the operation is blocking then the function returns after the data is programmed
   *   and returns the number of data items programmed.
   *
   * \note The Honeycomb implementation only follows the non-blocking approach.
   *
   */
  int32_t                 (*ProgramData)    (uint32_t addr, const void *data, uint32_t cnt);

  /**
   * \brief Erase Flash Block.
   *
   * @param [in] addr Block address
   * @return Status Error Codes
   * This function erases a flash block of a flash specified by the parameter _addr_ (points
   * to start of the block). _addr is relative to the start of the flash partition.
   *
   * The function is non-blocking and returns as soon as the driver has started the operation.
   * When the operation is completed the ARM_FLASH_EVENT_READY event is generated (if supported
   * and reported by ARM_Flash_GetCapabilities). In case of errors the ARM_FLASH_EVENT_ERROR
   * event is generated at the same time. Progress of the operation can also be monitored by
   * calling the ARM_Flash_GetStatus function and checking the busy flag.
   */
  int32_t                 (*EraseBlock)    (uint32_t addr);


  /**
   * \brief Erase Flash Sector.
   *
   * @param [in] addr Sector address
   * @return Status Error Codes
   * This function erases a flash sector of a flash specified by the parameter _addr_ (points
   * to start of the sector). _addr is relative to the start of the flash partition.
   *
   * The function is non-blocking and returns as soon as the driver has started the operation.
   * When the operation is completed the ARM_FLASH_EVENT_READY event is generated (if supported
   * and reported by ARM_Flash_GetCapabilities). In case of errors the ARM_FLASH_EVENT_ERROR
   * event is generated at the same time. Progress of the operation can also be monitored by
   * calling the ARM_Flash_GetStatus function and checking the busy flag.
   */
  int32_t                 (*EraseSector)    (uint32_t addr);


  /**
   * \brief
   * Erase complete Flash. Optional function for faster full chip erase.
   *
   * @return Status Error Codes
   *
   * This optional function erases the complete flash device partition. If the device does not
   * support global erase or only a portion of the Flash memory space is used for storing files
   * then this functions returns the error value ARM_DRIVER_ERROR_UNSUPPORTED. Function
   * ARM_Flash_GetCapabilities reports if the function ARM_Flash_EraseChip is supported.
   * The function is non-blocking and returns as soon as the driver has started the operation.
   * When the operation is completed the ARM_FLASH_EVENT_READY event is generated (if supported
   * and reported by ARM_Flash_GetCapabilities). In case of errors the ARM_FLASH_EVENT_ERROR event
   * is generated at the same time. Progress of the operation can also be monitored by calling
   * the ARM_Flash_GetStatus function and checking the busy flag.
   */
  int32_t                 (*EraseChip)      (void);

  /**
   * \brief
   * Get Flash status.
   * @return Flash status ARM_FLASH_STATUS
   * Retrieves current Flash interface status.
   */
  ARM_FLASH_STATUS        (*GetStatus)      (void);

  /** Get Flash information.
    *
    * \return Pointer to Flash information \ref struct _ARM_FLASH_INFO
    *
    * Retrieves information about the Flash device.
    */
  ARM_FLASH_INFO *        (*GetInfo)        (void);

  /**
   * \brief Retrieve the flash device index that is associated with the flash driver
   *    The flash device index defined in bapi_flash.h
   *
   * \note This is a complementary non ARM standard API functions
   * @return The flash device index that is associatd with this flash driver.
   */
  enum bapi_E_FlashDevice (*GetFlashDeviceIndex)(void);


  /**
   * \brief Retrieve the location of Flash partition within it's physical
   *    flash device.
   *
   * \note This is a complementary non ARM standard API functions
   * @return the location of flash partition within it's physical
   *    flash device. Honeycomb provides as way to split a physical
   *    flash device into several partitions that can be treated
   *    as logical devices. For each partition, a separate dedicated
   *    Flash driver is provided.
   */
  uint32_t (*BaseAddress)(void);

  /**
   * \brief Enable the Flash partition protection
   *
   * \note This is a complementary non ARM standard API functions
   * @return
   *  - ARM_DRIVER_OK                Operation succeeded
   *  - ARM_DRIVER_ERROR_BUSY        Driver is busy
   *  - ARM_DRIVER_ERROR_TIMEOUT     Timeout occurred
   *  - ARM_DRIVER_ERROR_UNSUPPORTED Operation not supported
   */
  int32_t (*EnableProtection)(void);

  /**
   * \brief Disable the Flash partition protection
   *
   * \note This is a complementary non ARM standard API functions
   * @return
   *  - ARM_DRIVER_OK                Operation succeeded
   *  - ARM_DRIVER_ERROR_BUSY        Driver is busy
   *  - ARM_DRIVER_ERROR_TIMEOUT     Timeout occurred
   *  - ARM_DRIVER_ERROR_UNSUPPORTED Operation not supported
   */
  int32_t (*DisableProtection)(void);

  /**
   * \brief Read for each sector the lock down info (flash specific).
   *
   * \note This is a complementary non ARM standard API functions
   * @return
   *  - ARM_DRIVER_OK                Operation succeeded
   *  - ARM_DRIVER_ERROR_BUSY        Driver is busy
   *  - ARM_DRIVER_ERROR_TIMEOUT     Timeout occurred
   *  - ARM_DRIVER_ERROR_UNSUPPORTED Operation not supported
   *  - ARM_DRIVER_PARAMETER         buffer too small
   */
  int32_t (*ReadSectorLockdown)(uint8_t* buffer, unsigned bufferSize);

  /**
   * \brief Read for each sector the protection info (flash specific).
   *
   * \note This is a complementary non ARM standard API functions
   * @return
   *  - ARM_DRIVER_OK                Operation succeeded
   *  - ARM_DRIVER_ERROR_BUSY        Driver is busy
   *  - ARM_DRIVER_ERROR_TIMEOUT     Timeout occurred
   *  - ARM_DRIVER_ERROR_UNSUPPORTED Operation not supported
   *  - ARM_DRIVER_PARAMETER         buffer too small
   */
  int32_t (*ReadSectorProtection)(uint8_t* buffer, unsigned bufferSize);

  /**
   * \brief Freeze the sector lock down for the whole flash device.
   *
   * \note This is a complementary non ARM standard API functions
   * @return
   *  - ARM_DRIVER_OK                Operation succeeded
   *  - ARM_DRIVER_ERROR_BUSY        Driver is busy
   *  - ARM_DRIVER_ERROR_TIMEOUT     Timeout occurred
   *  - ARM_DRIVER_ERROR_UNSUPPORTED Operation not supported
   */
  int32_t (*FreezeSectorLockdown)();
};

typedef const struct _ARM_DRIVER_FLASH ARM_DRIVER_FLASH;


/**
 * \brief
 * Obtain the CMSIS _ARM_DRIVER_FLASH driver structure for a particular flash partition
 *   of a flash device.
 *
 * The CMSIS Driver specifications defines just fixed names for the Flash
 * driver instances. This does badly support access to drivers at runtime
 * via Flash driver index. This supplementary function closes this gap.
 *
 * @param[in] flashDeviceIndex The flash device for which to obtain the flash partition.
 * @param[in] partitionIndex   The partition index within the flash device for which to
 *    obtain the driver.
 * @return Pointer to the FLASH Driver structure.
 */

C_FUNC const struct _ARM_DRIVER_FLASH* driver_flash_getDriver(enum bapi_E_FlashDevice flashDeviceIndex,
  unsigned partitionIndex);


C_INLINE uint32_t  driver_flash_calculateSize(ARM_FLASH_INFO* flashInfo) {
  return bapi_flash_sizeOfSectors(flashInfo, 0, flashInfo->sector_count);
}

C_INLINE uint32_t  driver_flash_calculateBlockSize(ARM_FLASH_INFO* flashInfo, uint32_t blockAddr) {
  return bapi_flash_sizeOfBlock(flashInfo, blockAddr);
}

/**
 * \brief
 * Obtain the number of partitions for a particular flash device.
 *
 * @param[in] flashDeviceIndex The flash device for which to obtain the number of partitions.
 * @return Number of partitions.
 */
C_FUNC unsigned driver_flash_getPartitionCount(enum bapi_E_FlashDevice flashDeviceIndex);


enum {_FLASH_PARTITION_SIZE_UNIT=0x0400}; /* Partitions size in kilobytes (1Kbyte = 1024 bytes)*/
typedef uint16_t _flash_partition_size_t;

C_INLINE uint32_t driver_flash_partitionSizeInBytes(_flash_partition_size_t partitionSize) {
  return S_CAST(uint32_t, _FLASH_PARTITION_SIZE_UNIT) * partitionSize;
}

/* ENABLE_FLASH_PARTITIONING may be set to 0 or 1 in product_config.h */
#if ENABLE_FLASH_PARTITIONING > 0

/**
 * \brief define the partitioning of for a flash device.
 *
 * @param flashDeviceIndex the flash device for which to define the partitions
 *
 * The parameters following the flashDeviceIndex define the size of the flash
 * partitions in kilobyte (=1024 byte) units. The size of the partitions will
 * be the smallest possible size that is greater or equal to the given size.
 *    -) E.g. if the flash section size is 1kByte, and the partition size is given
 *        as 3KByte, then the partition size will be 3Kbyte
 *    -) E.g. if the flash section size is 2kByte, and the partition size is given
 *        as 3KByte, then the partition size will be 4Kbyte
 *
 * Example on a 32Kbyte flash that has 1kbyte section size:
 * FLASH_DEFINE_PARTITIONS(flashDeviceIndex, 6, 14) will define
 *   3 partitions:
 *    -) partition0:  6 Kbyte (6 * 1024 bytes)
 *    -) partition1: 14 Kbyte (14 * 1024 bytes)
 *    -) partition2: 12 Kbyte (the remaining flash space)
 *
 */
#ifdef __cplusplus
  #define FLASH_DEFINE_PARTITIONS(flashDeviceIndex, ...) \
    template<> struct _FlashPartitionsDef<flashDeviceIndex> { \
      static const _flash_partition_size_t value[]; \
    }; \
    const _flash_partition_size_t _FlashPartitionsDef<flashDeviceIndex>::value[] = { \
      __VA_ARGS__, 0 /* 0 indicates a partition that takes
                          the rest of the flash device. */ \
    }

#else  /* __cplusplus */
  #define FLASH_DEFINE_PARTITIONS(flashDeviceIndex, ...)
#endif /* __cplusplus */

#endif /* #if ENABLE_FLASH_PARTITIONING > 0 */

/**@} cmsis_driver_flash */

#endif /* __DRIVER_FLASH_H */
