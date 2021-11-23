/*
 * osCom.h
 *
 *  Created on: 07.04.2013
 *      Author: Wolfgang
 */

#ifndef osFlash_H_
#define osFlash_H_

#include "boards/board-api/bapi_flash.h"

#if BAPI_HAS_FLASH_DEVICE > 0

#include "boards/cmsis/Driver_Flash.h"
#include "rtos/cmsis-rtos/cmsis_os_redirect.h"

#ifndef OS_FLASH_FLASH_PROGRAM_BYTES
	#define OS_FLASH_FLASH_PROGRAM_BYTES 0
#endif

#define OS_FLASH_ERASE_BLOCK 1


/**
 * \ingroup cmsis_os_ext_osFlash
 * \brief
 * This is the handle type that will be returned by the function
 *   osFlashOpenDevice(enum bapi_E_FlashDevice flashDeviceIndex, MsecType msecBlockTime).
 * It must be used to perform commands on a flash device, with the set of
 * available command functions that take this handle. For example \ref osFlashProgramData.
 * After having one or multiple commands executed, the handle should be passed to the function
 *   osFlashCloseDevice(osFlasDevicehHandle_t  flashDeviceHandle).
 *
 * When osFlashCloseDevice(osFlasDevicehHandle_t  flashDeviceHandle) returns, the
 *   handle has become invalid, and should be thrown away.
 */
typedef struct _os_flash_device_state* osFlasDevicehHandle_t;


struct _os_flash_cmd_result {
  osStatus_t result;       /* osOK if the command was performed successfully, otherwise osErrorOS. */
  enum bapi_flash_E_Command_ID_ flashCommandID; /* The flash command associated with the result. */
};


/**
 * \ingroup cmsis_os_ext_osFlash
 * \brief This function does some basic initializations which must take place before
 * any other osFlash API function can be called.
 *
 * \note Works also in a Non RTOS environment.
 *
 * It allocates some osMailQ and osMutex
 * objects. So calling it once at an early stage in the main function will avoid
 * heap memory fragmentation. The best place to call in an RTOS environment
 * is within the function osApplicationDefine_suppl (void). In a non RTOS environment
 * it should be called after the hardware initialization.
 */
C_FUNC void osFlashStartupInit(void);


/**
 * \ingroup cmsis_os_ext_osFlash
 * \brief This function provides a flash device handle to be used by functions that perform
 * flash commands.
 *
 * \note Works also in a Non RTOS environment.
 *
 * Example command functions that take the returned handle are:
 *   - \ref osFlashReadData
 *   - \ref osFlashProgramData
 *
 * After having one or multiple commands executed, the handle should be passed to function
 *   osFlashCloseDevice(osFlasDevicehHandle_t  flashDeviceHandle).
 *
 * When osFlashCloseDevice(osFlasDevicehHandle_t  flashDeviceHandle) returns, the
 *   handle should be taken as invalid, and should be thrown away.
 *
 * \note There is a nesting counter supported. Hence for each _successful_ call of \ref osFlashOpenDevice,
 *   it is required to call \ref osFlashCloseDevice. Only the final call of \ref osFlashCloseDevice
 *   will physically close the flash device.
 *
 * @param[in] flashDeviceIndex The flash device for which to obtain the handle
 * @param[in] msecBlockTime.   The time how long the system waits to open the,
 *   flash device. Reason for a timeout could be, that another thread is currently
 *   performing a flash operation.
   @return The flash device handle on success. Otherwise null.
 *
 * __Code Example:__
 ~~~~~~~~{.c}
  #include "rtos/cmsis-rtos-ext/osFlash.h"

  void eraseInternalPartition2() {
     osFlasDevicehHandle_t  flashDeviceHandle = osFlashOpenDevice(bapi_E_InternalFlashDev, 0);
     if(osFlashOpenDevice) {
       static const partition = 2; // We operate on partiton 2

       if(osFlashGetPartitionCount(flashDeviceHandle) >= partition) {
         // Erase partition 2 of the internal flash device.
         osFlashErasePartition(flashDeviceHandle, partition);
       }
     }
     osFlashCose(flashDeviceHandle);
   }
 ~~~~~~~~
 */
C_FUNC osFlasDevicehHandle_t osFlashOpenDevice(enum bapi_E_FlashDevice flashDeviceIndex,
  MsecType msecBlockTime);


/**
 * \ingroup cmsis_os_ext_osFlash
 * \brief This function reads the flash default time out for a flash device.
 *
 * \note Works also in a Non RTOS environment.
 *
 * @param[in] flashDeviceHandle The flash device handle from which to
 *  read the timeout.
 *
 * @return The timeout in milliseconds. 0 if flashDeviceHandle is null.
 *
 */
uint32_t osFlashGetDefaultTimeout(osFlasDevicehHandle_t flashDeviceHandle);

/**
 * \ingroup cmsis_os_ext_osFlash
 * \brief This function sets the flash default time out for a flash device.
 *
 * \note Works also in a Non RTOS environment.
 *
 * @param[in] flashDeviceHandle The flash device handle for which to
 *  set the timeout.
 *
 * @param[in] msecTimeout The the new timeout value in milliseconds.
 *
 * @return osOK if successful. osErrorValue if flashDeviceHandle is null.
 *
 */
osStatus_t osFlashSetDefaultTimeout(osFlasDevicehHandle_t flashDeviceHandle, uint32_t msecTimeout);

/**
 * \ingroup cmsis_os_ext_osFlash
 * \brief This function closes a flash device handle and frees the flash device for
 *   a next call of \ref osFlashOpenDevice.
 *
 * \note Works also in a Non RTOS environment.
 *
 * \sa osFlashOpenDevice(enum bapi_E_FlashDevice flashDeviceIndex,
 *   MsecType msecBlockTime).
 *
 * @param[in] flashDeviceHandle The flash device handle to close.
   @return The flash device handle if successful. Otherwise null.
 *
 */
C_FUNC osStatus_t osFlashCloseDevice(osFlasDevicehHandle_t  flashDeviceHandle);

/**
 * \ingroup cmsis_os_ext_osFlash
 * \brief This function returns the flash device index for a flash device handle.
 *
 * \note Works also in a Non RTOS environment.
 *
 * @param[in] flashDeviceHandle The flash device handle for which to obtain the flash device index.
 */
C_FUNC enum bapi_E_FlashDevice osFlashHandleToDeviceIndex(osFlasDevicehHandle_t  flashDeviceHandle);


/**
 * \ingroup cmsis_os_ext_osFlash
 * \brief Retrieve the location of flash partition within it's physical
 *    flash device.
 * @param[in] flashDeviceHandle The flash device handle of the flash device that owns
 *   the partition in question.
 * @param[in] partitionIndex of partition for which to obtain the base address.
 *   The index starts at zero and is within the scope of a flash device.
 *
 * @return the location of flash partition within it's physical
 *    flash device.
 */
C_FUNC uint32_t osFlashHandleToBaseAddress(osFlasDevicehHandle_t  flashDeviceHandle, unsigned partitionIndex);


/**
 * \ingroup cmsis_os_ext_osFlash
 * \brief This function returns the number of partitions of a flash device.
 *
 * \note Works also in a Non RTOS environment.
 *
 * The partitions can be defined in a file called flash_partitions.h in the
 * product configuration folder. In order to take impact, the product_config.h
 * header file must have set the macro \ref #ENABLE_FLASH_PARTITIONING to nonzero:
 * \code
 * #define ENABLE_FLASH_PARTITIONING 1
 * \endcode
 *
 * The contents of flash_partitions.h can define partitions by using the macro
 * \ref #FLASH_DEFINE_PARTITIONS
 *
 * The following code example will create:
 *   - 3 partitions for the Internal Flash with the sizes: 64Kbyte,
 *         32Kbyte and the Remaining Size of the flash device
 *   - 4 partitions for the first SPI Flash with the sizes: 128Kbyte,
 *         128Kbyte, 256Kbyte and the Remaining Size of the flash device.
 *
 * __Code Example:__
 ~~~~~~~~{.c}
  #if BAPI_HAS_INT_FLASH_DEVICE

  FLASH_DEFINE_PARTITIONS(bapi_E_InternalFlashDev, 64, 32);

  #endif

  #if BAPI_HAS_SPI_FLASH_DEVICE

  FLASH_DEFINE_PARTITIONS(bapi_E_SpiFlashDev0, 128, 128, 256);

  #endif
 ~~~~~~~~
 *
 *
 * If no partitions are explicitly defined, the flash device will have only one partition
 * that occupies the whole flash memory space.
 *
 * If the granularity of the flash device sectors is not good enough for the defined
 * partition sizes, the next possible bigger partition size will be chosen automatically.
 * The resulting partition sizes can be queried at runtime by calling \ref
 * osFlashPartitionSizeBytes.
 *
 * @param[in] flashDeviceHandle The flash device handle for which to obtain the
 *   number of partitions.
 * @return The number of partitions of the flash device.
 */
C_FUNC unsigned osFlashGetPartitionCount(osFlasDevicehHandle_t flashDeviceHandle);

/**
 * \ingroup cmsis_os_ext_osFlash
 * \brief This function returns information about a partition of a flash device
 *
 * \note Works also in a Non RTOS environment.
 *
 * @param[in] flashDeviceHandle The flash device handle of the flash device that owns
 *   the partition in question.
 * @param[in] partitionIndex of partition for which to obtain the information.
 *   The index starts at zero and is within the scope of a flash device.
 * @return A constant pointer to the information of the flash device partition.
 *
 * __Code Example:__
 ~~~~~~~~{.c}
  #include "rtos/cmsis-rtos-ext/osFlash.h"

   static uint8_t data[256];

  // We assume that the partition is erased already
  void programInternalPartition2() {
     osFlasDevicehHandle_t  flashDeviceHandle = osFlashOpenDevice(bapi_E_InternalFlashDev, 0);
     if(osFlashOpenDevice) {

       static const partition = 2; // We operate on partiton 2

       if(osFlashGetPartitionCount(flashDeviceHandle) >= partition) {
         ARM_FLASH_INFO* flashInfo = osFlashGetInfo(flashDeviceHandle, partitions);
         uint16_t program_unit = flashInfo->program_unit;

         // ensure that the data is a multiple of program_unit.
         if(sizeof(data) % program_unit == 0) {
            const uint32_t itemCnt = sizeof(data) / program_unit;
            osFlashProgramData(flashDeviceHandle, partition, data, itemCnt);
         }
       }
       osFlashCose(flashDeviceHandle);
     }
   }
 ~~~~~~~~
 */
C_FUNC const ARM_FLASH_INFO* osFlashGetInfo(osFlasDevicehHandle_t flashDeviceHandle, unsigned partitionIndex);

/**
 * \ingroup cmsis_os_ext_osFlash
 * \brief This function returns the size of a partition of a flash device.
 *
 * It is strongly recommended to use this function rather than calculating the size from an ARM_FLASH_INFO
 *   structure. This function handles the 2 case of flashes with:
 *     - differently sized sectors.
 *     - equally sized sectors.
 *
 * \note Works also in a Non RTOS environment.
 *
 * @param[in] flashDeviceHandle The flash device handle of the flash that owns
 *   the partition in question.
 * @param[in] partitionIndex of partition for which to obtain the size.
 *   The index counting starts at zero within the scope of a flash device.
 *
 */
C_FUNC uint32_t osFlashPartitionSizeBytes(osFlasDevicehHandle_t flashDeviceHandle, unsigned partitionIndex);


/**
 * \ingroup cmsis_os_ext_osFlash
 * \brief This function returns the size of a block of a flash sector.
 *   A block is the smallest erasable unit within a flash sector. I may happen that a flash sector consists
 *     of only one block that has the same size as the sector. In other words: the block and the sector are
 *     identical.
 *
 * It is strongly recommended to use this function rather than calculating the size from an ARM_FLASH_INFO
 *   structure. This function handles the 2 case of flashes with:
 *     - differently sized sectors with individual block sizes.
 *     - equally sized sectors with all the same block sizes.
 *
 * \note Works also in a Non RTOS environment.
 *
 * @param[in] flashDeviceHandle The flash device handle of the flash that owns
 *   the partition in question.
 * @param[in] partitioIndex of partition for which to obtain the size.
 *   The index counting starts at zero within the scope of a flash device.
 * @param[in] blockAddr The address of the block to obtain the size from.
 *
 */
C_FUNC uint32_t osFlashBlockSizeBytes(osFlasDevicehHandle_t flashDeviceHandle, unsigned partitionIndex, uint32_t blockAddr);

/**
 * \ingroup cmsis_os_ext_osFlash
 * \brief This function returns the capabilities of a flash partition.
 *
 * \note Works also in a Non RTOS environment.
 *
 * The bitfield members of \ref _ARM_FLASH_CAPABILITIES encode the capabilities
 * of the underlying \ref _ARM_DRIVER_FLASH driver. Some of these capabilities are also
 * important for the use of this osFlash API.
 *
 * The element _event_ready_ indicates that the driver is able to generate the
 * ARM_FLASH_EVENT_READY event. The Honeycomb ARM FLASH CMSIS driver supports
 * event generation for all flash devices. Hence you should see always a 1 there.
 *
 * This detail is important for the whole osFlash API. The osFlash API
 * functions that perform a flash command take care of this
 * \ref #ARM_FLASH_EVENT_READY event by putting the calling thread into sleep mode
 * and wakes it up, when the ready event appears.
 * In other words, other threads are not blocked while the current thread is
 * performing a flash command.
 * However, in a _Non RTOS environment_, the osFlash API functions do a spin loop
 * when they need to wait for this event, (unfortunately wasting MCU performance,
 * but there is no other thread that could still run).
 *
 * The element _data_width_ specifies the data access size and also defines the
 * data type (uint8_t, uint16_t, uint32_t, uint64_t) for the _dataIn_ parameter in
 * \ref osFlashReadData functions.
 *
 * The element _erase_chip_ specifies that the ARM_Flash_EraseChip function is
 * supported. Typically full chip erase is much faster than erasing the whole
 * device sector per sector.
 *
 * @param[in] flashDeviceHandle The flash device handle of the flash that owns
 *   the partition in question.
 * @param[in] partitionIndex of partition for which to obtain the capabilities.
 *   The index counting starts at zero within the scope of a flash device.
 *
 */
C_FUNC ARM_FLASH_CAPABILITIES osFlashGetCapabilities(osFlasDevicehHandle_t flashDeviceHandle,
  unsigned partitionIndex);

//#if _DEBUG_FLASH_SECTOR_INFO

/**
 * \ingroup cmsis_os_ext_osFlash
 * \brief Flash Command: This function reads the sector lock down info for the flash
 *   device into a buffer.
 */
C_FUNC osStatus_t osFlashReadSectorLockdown(osFlasDevicehHandle_t flashDeviceHandle, MsecType msecBlockTime,
  uint8_t* buffer, unsigned bufferSize);

C_FUNC osStatus_t osFlashDtReadSectorLockdown(osFlasDevicehHandle_t flashDeviceHandle,
  uint8_t* buffer, unsigned bufferSize);

/**
 * \ingroup cmsis_os_ext_osFlash
 * \brief Flash Command: This function reads the sector protection info for the flash
 *   device into a buffer.
 */
C_FUNC osStatus_t osFlashReadSectorProtection(osFlasDevicehHandle_t flashDeviceHandle, MsecType msecBlockTime,
  uint8_t* buffer, unsigned bufferSize);

C_FUNC osStatus_t osFlashDtReadSectorProtection(osFlasDevicehHandle_t flashDeviceHandle,
  uint8_t* buffer, unsigned bufferSize);

//#endif

/**
 * \ingroup cmsis_os_ext_osFlash
 * \brief Flash Command: This function enables the protection for a complete flash partition.
 *
 * First of all, this function waits, until any pending flash command for the same flash device
 * is completed, or if a timeout appears.
 *
 * In a Non RTOS environment, the thread will wait by doing a spin loop until either
 * the pending command is finished or a timeout expired. In an RTOS environment, the thread
 * will go to sleep until either the pending command is finished or a timeout expired.
 *
 * When any previous pending command is finished or when there wasn't any previous command
 * pending, this function will invoke the enaable protection command and return immediately. I.e. it will
 * __not wait__ until this command is completed. However, the next command that may be invoked
 * may have to wait, if this enable protection command is still pending.
 *
 * If you want to know whether the command was completed successfully, you must
 *   call the function \ref osFlashWaitCommandComplete. If you are not interested how this
 *   flash command has been completed (successfully or unsuccessfully), you can immediately invoke
 *   the next flash command.
 *
 * \note Works also in a Non RTOS environment.
 *
 * @param[in] flashDeviceHandle The flash device handle of the flash that owns
 *   the partition in question.
 * @param[in] partitionIndex of partition for which to disable the protection.
 *   The index counting starts at zero and is within the scope of a flash device.
 * @param[in] msecBlockTime.   The time how long the system waits to invoke the,
 *   flash device command. Reason for a timeout could be, that another thread is
 *   currently invoking a flash operation for this flash device.
 * There might be another thread currently occupying the
 *   flash device with invoking a flash command. This is the time, that this
 *   function might wait for the other thread to finish before giving up.
 * @return
 *        - osOK                   if the command was initiated successfully
 *        - osErrorOS              if not supported (Currently flashes with different
 *                                    sector sizes are not supported ).
 *        - osErrorTimeoutResource if timeout appeared.
 */
C_FUNC osStatus_t osFlashEnablePartitionProtection(osFlasDevicehHandle_t flashDeviceHandle, unsigned partitionIndex,
  MsecType msecBlockTime);

C_INLINE osStatus_t osFlashDtEnablePartitionProtection(osFlasDevicehHandle_t flashDeviceHandle, unsigned partitionIndex) {
  return osFlashEnablePartitionProtection(flashDeviceHandle, partitionIndex, osFlashGetDefaultTimeout(flashDeviceHandle));
}


/**
 * \ingroup cmsis_os_ext_osFlash
 * \brief Flash Command: This function disables the protection for a complete flash partition.
 *
 * First of all, this function waits, until any pending flash command for the same flash device
 * is completed, or if a timeout appears.
 *
 * In a Non RTOS environment, the thread will wait by doing a spin loop until either
 * the pending command is finished or a timeout expired. In an RTOS environment, the thread
 * will go to sleep until either the pending command is finished or a timeout expired.
 *
 * When any previous pending command is finished or when there wasn't any previous command
 * pending, this function will invoke the disable protection command and return immediately. I.e. it will
 * __not wait__ until this command is completed. However, the next command that may be invoked
 * may have to wait, if this disable protection command is still pending.
 *
 * If you want to know whether the command was completed successfully, you must
 *   call the function \ref osFlashWaitCommandComplete. If you are not interested how this
 *   flash command has been completed (successfully or unsuccessfully), you can immediately invoke
 *   the next flash command.
 *
 * \note Works also in a Non RTOS environment.
 *
 * @param[in] flashDeviceHandle The flash device handle of the flash that owns
 *   the partition in question.
 * @param[in] partitionIndex of partition for which to disable the protection.
 *   The index counting starts at zero and is within the scope of a flash device.
 * @param[in] msecBlockTime.   The time how long the system waits to invoke the,
 *   flash device command. Reason for a timeout could be, that another thread is
 *   currently invoking a flash operation for this flash device.
 * There might be another thread currently occupying the
 *   flash device with invoking a flash command. This is the time, that this
 *   function might wait for the other thread to finish before giving up.
 * @return
 *        - osOK                   if the command was initiated successfully
 *        - osErrorOS              if not supported (Currently flashes with different
 *                                    sector sizes are not supported ).
 *        - osErrorTimeoutResource if timeout appeared.
 */
C_FUNC osStatus_t osFlashDisablePartitionProtection(osFlasDevicehHandle_t flashDeviceHandle, unsigned partitionIndex,
  MsecType msecBlockTime);

C_INLINE osStatus_t osFlashDtDisablePartitionProtection(osFlasDevicehHandle_t flashDeviceHandle, unsigned partitionIndex) {
  //TODO: PS implement enabling and disabling partition protection
	//return osFlashDisablePartitionProtection(flashDeviceHandle, partitionIndex, osFlashGetDefaultTimeout(flashDeviceHandle));
  return osOK;
}

/**
 * \ingroup cmsis_os_ext_osFlash
 * \brief Flash Command: This function disables the sector lock down for a complete physical flash device.
 *
 * First of all, this function waits, until any pending flash command for the same flash device
 * is completed, or if a timeout appears.
 *
 * In a Non RTOS environment, the thread will wait by doing a spin loop until either
 * the pending command is finished or a timeout expired. In an RTOS environment, the thread
 * will go to sleep until either the pending command is finished or a timeout expired.
 *
 * When any previous pending command is finished or when there wasn't any previous command
 * pending, this function will invoke the disable protection command and return immediately. I.e. it will
 * __not wait__ until this command is completed. However, the next command that may be invoked
 * may have to wait, if this disable protection command is still pending.
 *
 * If you want to know whether the command was completed successfully, you must
 *   call the function \ref osFlashWaitCommandComplete. If you are not interested how this
 *   flash command has been completed (successfully or unsuccessfully), you can immediately invoke
 *   the next flash command.
 *
 * \note Works also in a Non RTOS environment.
 *
 * @param[in] flashDeviceHandle The flash device handle of the flash that owns
 *   the partition in question.
 * @param[in] msecBlockTime.   The time how long the system waits to invoke the,
 *   flash device command. Reason for a timeout could be, that another thread is
 *   currently invoking a flash operation for this flash device.
 * There might be another thread currently occupying the
 *   flash device with invoking a flash command. This is the time, that this
 *   function might wait for the other thread to finish before giving up.
 * @return
 *        - osOK                   if the command was initiated successfully
 *        - osErrorOS              if not supported.
 *        - osErrorTimeoutResource if timeout appeared.
 */
C_FUNC osStatus_t osFlashFreezeSectorLockdown(osFlasDevicehHandle_t flashDeviceHandle, MsecType msecBlockTime);

C_INLINE osStatus_t osFlashDtFreezeSectorLockdown(osFlasDevicehHandle_t flashDeviceHandle, unsigned partitionIndex) {
  return osFlashFreezeSectorLockdown(flashDeviceHandle, osFlashGetDefaultTimeout(flashDeviceHandle));
}

/**
 * \ingroup cmsis_os_ext_osFlash
 * \brief Flash Command: This function erases a complete flash partition.
 *
 * First of all, this function waits, until any pending flash command for the same flash device
 * is completed, or if a timeout appears.
 *
 * In a Non RTOS environment, the thread will wait by doing a spin loop until either
 * the pending command is finished or a timeout expired. In an RTOS environment, the thread
 * will go to sleep until either the pending command is finished or a timeout expired.
 *
 * When any previous pending command is finished or when there wasn't any previous command
 * pending, this function will invoke the erase command and return immediately. I.e. it will
 * __not wait__ until this command is completed. However, the next command that may be invoked
 * may have to wait, if this erase command is still pending.
 *
 * If you want to know whether the command was completed successfully, you must
 *   call the function \ref osFlashWaitCommandComplete. If you are not interested how this
 *   flash command has been completed (successfully or unsuccessfully), you can immediately invoke
 *   the next flash command.
 *
 * \note Works also in a Non RTOS environment.
 *
 * @param[in] flashDeviceHandle The flash device handle of the flash that owns
 *   the partition in question.
 * @param[in] partitionIndex of partition for which to erase.
 *   The index counting starts at zero and is within the scope of a flash device.
 * @param[in] msecBlockTime.   The time how long the system waits to invoke the,
 *   flash device command. Reason for a timeout could be, that another thread is
 *   currently invoking a flash operation for this flash device.
 * There might be another thread currently occupying the
 *   flash device with invoking a flash command. This is the time, that this
 *   function might wait for the other thread to finish before giving up.
 * @return
 *        - osOK                   if the command was initiated successfully
 *        - osErrorOS              if not supported (Currently flashes with different
 *                                    sector sizes are not supported ).
 *        - osErrorTimeoutResource if timeout appeared.
 */
C_FUNC osStatus_t osFlashErasePartition(osFlasDevicehHandle_t flashDeviceHandle, unsigned partitionIndex,
  MsecType msecBlockTime);

C_INLINE osStatus_t osFlashDtErasePartition(osFlasDevicehHandle_t flashDeviceHandle, unsigned partitionIndex) {
  return osFlashErasePartition(flashDeviceHandle, partitionIndex, osFlashGetDefaultTimeout(flashDeviceHandle));
}

/**
 * \ingroup cmsis_os_ext_osFlash
 * \brief Flash Command: This function erases a single sector within a flash partition.
 *
 * First of all, this function waits, until any pending flash command for the same flash device
 * is completed, or if a timeout appears.
 *
 * In an RTOS environment, other threads will still continue to run.
 * In a Non RTOS environment, the thread will wait by doing a spin loop until either
 * the pending command is finished or a timeout expired.
 *
 * When any previous pending command is finished or when there wasn't any previous command
 * pending, this function will invoke the erase command and return immediately. I.e. it will
 * __not wait__ until this command is completed. However, the next command that may be invoked
 * may have to wait, if this erase command is still pending.
 *
 * If you want to know whether the command was completed successfully, you must
 *   call the function \ref osFlashWaitCommandComplete. If you are not interested how this
 *   flash command has been completed (successfully or unsuccessfully), you can immediately invoke
 *   the next flash command.
 *
 * \note Works also in a Non RTOS environment.
 *
 * @param[in] flashDeviceHandle The flash device handle of the flash that owns
 *   the partition in question.
 * @param[in] partitionIndex of partition for which to erase.
 *   The index counting starts at zero and is within the scope of a flash device.
 * @param[in] addr The sector address. This address is relative to the start of the partition.
 *   That means that you need to pass zero if you want to erase the first sector of the partition.
 *   Use the function osFlashGetInfo(osFlasDevicehHandle_t flashDeviceHandle, unsigned partitionIndex)
 *   to obtain sector address information. E.g. if you have equally sized
 *   sectors in the partition, you use 1, 2, 3 ... times the sector_size info
 *   from the flash info structure to calculate the 2nd, 3rd 4th ... sector address.
 * @param[in] msecBlockTime.   The time how long the system waits to invoke the,
 *   flash device command. Reason for a timeout could be, that another thread is
 *   currently invoking a flash operation for this flash device.
 * @return
 *      - osOK                   if the command was initiated successfully
 *      - osErrorOS              if not supported (Currently flashes with different
 *                                  sector sizes are not supported).
 *      - osErrorTimeoutResource if timeout appeared.
 *      - osErrorValue           if the addr exceeds the size of the partition.
 *      - osErrorValue           if the addr is not the first address of the sector.
 */
C_FUNC osStatus_t osFlashEraseSector(osFlasDevicehHandle_t flashDeviceHandle, unsigned partitionIndex,
  uint32_t addr, MsecType msecBlockTime);

C_INLINE osStatus_t osFlashDtEraseSector(osFlasDevicehHandle_t flashDeviceHandle, unsigned partitionIndex,
  uint32_t addr) {
  return osFlashEraseSector(flashDeviceHandle, partitionIndex, addr, osFlashGetDefaultTimeout(flashDeviceHandle));
}

#if OS_FLASH_ERASE_BLOCK
/**
 * \ingroup cmsis_os_ext_osFlash
 * \brief Flash Command: This function erases a single block within a partition.
 *
 * A block is the smallest erasable unit within a partition. You can obtain the block
 * size by reading the \ref ARM_FLASH_INFO via function \ref osFlashGetInfo.
 *
 * If the ARM_FLASH_INFO has a sector_info that is nonzero, you must calculate
 * to which sector the block belongs and  extract the block size from the component
 * _block_size_in_pages_ of the _sector_info_.
 * Otherwise, the partition has equally structured sectors, and you must extract the
 * block size component _block_size_in_pages_ of \ref ARM_FLASH_INFO itself.
 *
 * If you want to know the block size in bytes you must multiply this value with
 * component page_size of the ref ARM_FLASH_INFO structure.
 *
 * First of all, this function waits, until any pending flash command for the same flash device
 * is completed, or if a timeout appears.
 *
 * In an RTOS environment, other threads will still continue to run.
 * In a Non RTOS environment, the thread will wait by doing a spin loop until either
 * the pending command is finished or a timeout expired.
 *
 * When any previous pending command is finished or when there wasn't any previous command
 * pending, this function will invoke the erase command and return immediately. I.e. it will
 * __not wait__ until this command is completed. However, the next command that may be invoked
 * may have to wait, if this erase command is still pending.
 *
 * If you want to know whether the command was completed successfully, you must
 *   call the function \ref osFlashWaitCommandComplete. If you are not interested how this
 *   flash command has been completed (successfully or unsuccessfully), you can immediately invoke
 *   the next flash command.
 *
 * \note Works also in a Non RTOS environment.
 *
 * @param[in] flashDeviceHandle The flash device handle of the flash that owns
 *   the partition in question.
 * @param[in] partitionIndex of partition for which to erase.
 *   The index counting starts at zero and is within the scope of a flash device.
 * @param[in] blockAddr The address of the block to be erased. This address is relative to the start of the partition.
 *   That means that you need to pass zero if you want to erase the first block of the partition.
 *   Use the function osFlashGetInfo(osFlasDevicehHandle_t flashDeviceHandle, unsigned partitionIndex)
 *   to obtain block address information. E.g. if you have equally structured
 *   sectors in the partition, you use 1, 2, 3 ... times the block size __in byte units__
 *   from the flash info structure to calculate the 2nd, 3rd 4th ... block address.
 * @param[in] msecBlockTime.   The time how long the system waits to invoke the,
 *   flash device command. Reason for a timeout could be, that another thread is
 *   currently invoking a flash operation for this flash device.
 * @return
 *      - osOK                   if the command was initiated successfully
 *      - osErrorOS              if not supported (Currently flashes with different
 *                                  sector sizes are not supported).
 *      - osErrorTimeoutResource if timeout appeared.
 *      - osErrorValue           if the _blockAddr_ exceeds the size of the partition.
 *      - osErrorValue           if the _blockAddr_ is not the first address of the sector.
 */
C_FUNC osStatus_t osFlashEraseBlock(osFlasDevicehHandle_t flashDeviceHandle, unsigned partitionIndex,
  uint32_t blockAddr, MsecType msecBlockTime);

C_INLINE osStatus_t osFlashDtEraseBlock(osFlasDevicehHandle_t flashDeviceHandle, unsigned partitionIndex,
  uint32_t blockAddr) {
  return osFlashEraseBlock(flashDeviceHandle, partitionIndex, blockAddr, osFlashGetDefaultTimeout(flashDeviceHandle));
}

#endif

/**
 * \ingroup cmsis_os_ext_osFlash
 * \brief This function converts the number of bytes to the number of data items that is applicable
 *   for a flash partition program. Any number of bytes need to be converted to number of data items for
 *   a flash program command. Refer to \ref osFlashWriteData. Note that data item count for
 *   a program command is different to data item count for a read command.
 *
 * @param[in] flashDeviceHandle The flash device handle of the flash that owns
 *   the partition in question.
 * @param[in] partitionIndex of the partition to for which the conversion should be done.
 *   The index counting starts at zero within the scope of a flash device.
 * @param[in] bytesCnt The bytes count that shall be converted to data items count.
 * @return The data items count for a program operation.
 *
 * \note If _bytesCnt_ is not aligned with the _program_unit_, the __return value is
 *  cut off__. E.g. if _program_unit_ is 4 and bytesCnt is 5, the return value is 1.
 *
 */
C_FUNC uint32_t osFlashProgramData_BytesToDataItemCount(osFlasDevicehHandle_t flashDeviceHandle,
  unsigned partitionIndex, uint32_t bytesCnt);


/**
 * \ingroup cmsis_os_ext_osFlash
 * \brief Flash Command: This function programs data at an address of the flash partition.
 *
 * First of all, this function waits, until any pending flash command for the same flash device
 * is completed, or if a timeout appears.
 *
 * In an RTOS environment, other threads will still continue to run.
 * In a Non RTOS environment, the thread will wait by doing a spin loop until either
 * the pending command is finished or a timeout expired.
 *
 * When any previous pending command is finished or when there wasn't any previous command
 * pending, this function will invoke the erase command and return immediately. I.e. it will
 * __not wait__ until this command is completed. However, the next command that may be invoked
 * may have to wait, if this program command is still pending.
 *
 * \warning When the function returns, the memory of the _dataOut_ pointer must stay valid,
 *   because the program command that is executed in the background will still access
 *   this memory. The memory that _dataOut_ is pointing to can be made invalid, when any of
 *   the below conditions are fulfilled:
 *    - Any other flash command function call for the same flash device handle has returned.
 *    - A \ref osFlashCloseDevice function call for the same flash device handle has returned.
 *    - A \ref osFlashWaitCommandComplete() has returned without timeout result.
 *
 * If you want to know whether the command was completed successfully, you must
 *   call the function \ref osFlashWaitCommandComplete. If you are not interested how this
 *   flash command has been completed (successfully or unsuccessfully), you can immediately invoke
 *   the next flash command.
 *
 * \note Works also in a Non RTOS environment.
 *
 * @param[in] flashDeviceHandle The flash device handle of the flash that owns
 *   the partition in question.
 * @param[in] partitionIndex of partition to program the data.
 *   The index counting starts at zero within the scope of a flash device.
 * @param[in] addr Address where to place the data in flash. This address is relative to the start
 *   of the partition. That means, if you want to start writing at the start of the
 *   partition, you need to pass zero. This __address needs to be aligned__ with the
 *   _program_unit_ that is valid for the partition. The program_unit can be obtained by
 *   calling \ref osFlashGetInfo. E.g. if the _program_unit_ is 8, addr modulo 8 must be zero.
 * @param[in] dataOut Pointer to the data that shall go into the flash partition.
 * @param[in] cnt The number of _program_units_ to write to the flash. E.g. if the
 *   _program_unit_ is 8 and _cnt_ is 3, then 24 bytes will be written to the flash partition.
 * @param[in] msecBlockTime.   The time how long the system waits to invoke the
 *   flash device command. Reason for a timeout could be, that another thread is
 *   currently invoking a flash operation for this flash device.
 * @return
 *      - osOK                   if the command was initiated successfully
 *      - osErrorOS              if not supported (Currently flashes with different
 *                                  sector sizes are not supported).
 *      - osErrorTimeoutResource if timeout appeared.
 *      - osErrorValue           if the combination of addr and cnt exceeds partly or
 *                                  fully the flash partition boundary.
 */
C_FUNC osStatus_t osFlashProgramData(osFlasDevicehHandle_t flashDeviceHandle, unsigned partitionIndex,
  uint32_t addr, const void *dataOut, uint32_t cnt, MsecType msecBlockTime);

C_INLINE osStatus_t osFlashDtProgramData(osFlasDevicehHandle_t flashDeviceHandle, unsigned partitionIndex,
  uint32_t addr, const void *dataOut, uint32_t cnt) {
  return osFlashProgramData(flashDeviceHandle, partitionIndex, addr, dataOut, cnt, osFlashGetDefaultTimeout(flashDeviceHandle));
}

#if OS_FLASH_FLASH_PROGRAM_BYTES

/**
 * \ingroup cmsis_os_ext_osFlash
 * \brief Flash Command: This function programs data at an address of the flash partition.
 *
 * First of all, this function waits, until any pending flash command for the same flash device
 * is completed, or if a timeout appears.
 *
 * In an RTOS environment, other threads will still continue to run.
 * In a Non RTOS environment, the thread will wait by doing a spin loop until either
 * the pending command is finished or a timeout expired.
 *
 * When any previous pending command is finished or when there wasn't any previous command
 * pending, this function will invoke the erase command and return immediately. I.e. it will
 * __not wait__ until this command is completed. However, the next command that may be invoked
 * may have to wait, if this program command is still pending.
 *
 * \warning When the function returns, the memory of the _dataOut_ pointer must stay valid,
 *   because the program command that is executed in the background will still access
 *   this memory. The memory that _dataOut_ is pointing to can be made invalid, when any of
 *   the below conditions are fulfilled:
 *    - Any other flash command function call for the same flash device handle has returned.
 *    - A \ref osFlashCloseDevice function call for the same flash device handle has returned.
 *    - A \ref osFlashWaitCommandComplete() has returned without timeout result.
 *
 * If you want to know whether the command was completed successfully, you must
 *   call the function \ref osFlashWaitCommandComplete. If you are not interested how this
 *   flash command has been completed (successfully or unsuccessfully), you can immediately invoke
 *   the next flash command.
 *
 * \note Works also in a Non RTOS environment.
 *
 * @param[in] flashDeviceHandle The flash device handle of the flash that owns
 *   the partition in question.
 * @param[in] partitionIndex of partition to program the data.
 *   The index counting starts at zero within the scope of a flash device.
 * @param[in] addr Address where to place the data in flash. This address is relative to the start
 *   of the partition. That means, if you want to start writing at the start of the
 *   partition, you need to pass zero. This __address needs to be aligned__ with the
 *   _program_unit_ that is valid for the partition. The program_unit can be obtained by
 *   calling \ref osFlashGetInfo. E.g. if the _program_unit_ is 8, addr modulo 8 must be zero.
 * @param[in] dataOut Pointer to the data that shall go into the flash partition.
 * @param[in] bytesCnt The number of _bytes_ to write to the flash.
 * @param[in] msecBlockTime.   The time how long the system waits to invoke the
 *   flash device command. Reason for a timeout could be, that another thread is
 *   currently invoking a flash operation for this flash device.
 * @return
 *      - osOK                   if the command was initiated successfully
 *      - osErrorOS              if not supported (Currently flashes with different
 *                                  sector sizes are not supported).
 *      - osErrorTimeoutResource if timeout appeared.
 *      - osErrorValue           if the combination of addr and cnt exceeds partly or
 *                                  fully the flash partition boundary.
 *      - osErrorNoMemory        if start address of the bytesCnt is not aligned
 *                                  with the programUnit of the flash device,
 *                                  a temporary buffer must be allocated that
 *                                  can hold padding bytes filled with _erase_ _value_ (typically 0xFF) .
 *                                  osErrorNoMemory is returned if this memory
 *                                  allocation fails.
 */
C_FUNC osStatus_t osFlashProgramBytes(osFlasDevicehHandle_t flashDeviceHandle, unsigned partitionIndex,
  uint32_t addr, const void *dataOut, uint32_t bytesCnt, MsecType msecBlockTime);

#endif

/**
 * \deprecated Not required anymore, when osFlashReadBytes instead of osFlashReadData (also deprecated)
 *   is consistently used.
 * \ingroup cmsis_os_ext_osFlash
 * \brief This function converts the number of bytes to the number of data items that is applicable
 *   for a flash partition read. Any number of bytes need to be converted to number of data items for
 *   a flash read command. Refer to \ref osFlashReadData. Note that data item count for
 *   a program command is different to data item count for a read command.
 *
 * @param[in] flashDeviceHandle The flash device handle of the flash that owns
 *   the partition in question.
 * @param[in] partitionIndex of the partition to for which the conversion should be done.
 *   The index counting starts at zero within the scope of a flash device.
 * @param[in] bytesCnt The bytes count that shall be converted to data items count.
 * @return The data items count for a read operation.
 *
 * \note If _bytesCnt_ is not aligned with the _data_width_, the __return value is
 *  enlarged__. E.g. if _data_width_ is 4 and bytesCnt is 7, the return value is 2.
 *
 */
C_FUNC uint32_t osFlashReadData_BytesToDataItemCount(osFlasDevicehHandle_t flashDeviceHandle,
  unsigned partitionIndex, uint32_t bytesCnt);

/**
 * \deprecated Use osFlashReadBytes instead.
 * \ingroup cmsis_os_ext_osFlash
 * \brief Flash Command: This function reads data from a flash partition.
 *
 * First of all, this function waits, until any pending flash command for the same flash device
 * is completed, or if a timeout appears.
 *
 * In a Non RTOS environment, the thread will wait by doing a spin loop until either
 * the pending command is finished or a timeout expired. In an RTOS environment, the thread
 * will go to sleep until either the pending command is finished or a timeout expired.
 *
 * When any previous pending command is finished or when there wasn't any previous command
 * pending, this function will invoke the read command and return immediately. I.e. it will
 * __not wait__ until this command is completed. However, the next command that may be invoked
 * may have to wait, if this read command is still pending.
 *
 * \warning When the function returns, the memory of the _dataIn_ pointer must stay valid,
 *   because the program command that is executed in the background will still access
 *   this memory. The memory that _dataIn_ is pointing to is allowed to be made invalid, when any of
 *   the below conditions are fulfilled:
 *    - Any other flash command function call for the same flash device handle has returned.
 *    - A \ref osFlashCloseDevice function call for the same flash device handle has returned.
 *    - A \ref osFlashWaitCommandComplete() has returned without timeout result.
 *
 * If you want to know whether the command was completed successfully, you must
 *   call the function \ref osFlashWaitCommandComplete. If you are not interested how this
 *   flash command has been completed (successfully or unsuccessfully), you can immediately invoke
 *   the next flash command.
 *
 * \note Works also in a Non RTOS environment.
 *
 * @param[in] flashDeviceHandle The flash device handle of the flash that owns
 *   the partition in question.
 * @param[in] partitionIndex of the partition to read data from.
 *   The index counting starts at zero within the scope of a flash device.
 * @param[in] addr The flash address to read the data from. This address is relative to the
 *   start of the partition. The address __needs to be aligned__ to the _data_width_ as per
 *   \ref ARM_FLASH_CAPABILITIES. Refer to \ref osFlashGetCapabilities.
 *   E.g. if the _data_width_ is 4, addr modulo 4 must be zero.
 * @param[in] dataItemCnt The number of _data_width_ data items to read from the flash. E.g. if the
 *   _data_width_ is 4 and _cnt_ is 3, then 12 bytes will be read from the flash partition.
 * @param[in] dataIn Pointer to the memory where to place the data read from the flash partition.
 * @param[in] msecBlockTime.   The time how long the system waits to invoke the
 *   flash device command. Reason for a timeout could be, that another thread is
 *   currently invoking a flash operation for this flash device.
 * @return
 *      - osOK                   if the command was initiated successfully
 *      - osErrorOS              if not supported (Currently flashes with different
 *                                  sector sizes are not supported).
 *      - osErrorTimeoutResource if timeout appeared.
 *      - osErrorValue           if the combination of addr and cnt exceeds partly or
 *                                  fully the flash partition boundary.
 */
C_FUNC osStatus_t osFlashReadData(osFlasDevicehHandle_t flashDeviceHandle, unsigned partitionIndex,
  uint32_t addr, void *dataIn, uint32_t dataItemCnt, MsecType msecBlockTime);

C_INLINE osStatus_t osFlashDtReadData(osFlasDevicehHandle_t flashDeviceHandle, unsigned partitionIndex,
  uint32_t addr, void *dataIn, uint32_t dataItemCnt) {
  return osFlashReadData(flashDeviceHandle, partitionIndex, addr, dataIn, dataItemCnt, osFlashGetDefaultTimeout(flashDeviceHandle));
}

/**
 * \ingroup cmsis_os_ext_osFlash
 * \brief Flash Command: This function reads data from a flash partition in byte units.
 *
 * First of all, this function waits, until any pending flash command for the same flash device
 * is completed, or if a timeout appears.
 *
 * In a Non RTOS environment, the thread will wait by doing a spin loop until either
 * the pending command is finished or a timeout expired. In an RTOS environment, the thread
 * will go to sleep until either the pending command is finished or a timeout expired.
 *
 * When any previous pending command is finished or when there wasn't any previous command
 * pending, this function will invoke the read command and return immediately. I.e. it will
 * __not wait__ until this command is completed. However, the next command that may be invoked
 * may have to wait, if this read command is still pending.
 *
 * \warning When the function returns, the memory of the _dataIn_ pointer must stay valid,
 *   because the program command that is executed in the background will still access
 *   this memory. The memory that _dataIn_ is pointing to is allowed to be made invalid, when any of
 *   the below conditions are fulfilled:
 *    - Any other flash command function call for the same flash device handle has returned.
 *    - A \ref osFlashCloseDevice function call for the same flash device handle has returned.
 *    - A \ref osFlashWaitCommandComplete() has returned without timeout result.
 *
 * If you want to know whether the command was completed successfully, you must
 *   call the function \ref osFlashWaitCommandComplete. If you are not interested how this
 *   flash command has been completed (successfully or unsuccessfully), you can immediately invoke
 *   the next flash command.
 *
 * \note Works also in a Non RTOS environment.
 *
 * @param[in] flashDeviceHandle The flash device handle of the flash that owns
 *   the partition in question.
 * @param[in] partitionIndex of the partition to read data from.
 *   The index counting starts at zero within the scope of a flash device.
 * @param[in] addr The flash address to read the data from. This address is relative to the
 *   start of the partition. The address __needs to be aligned__ to the _data_width_ as per
 *   \ref ARM_FLASH_CAPABILITIES. Refer to \ref osFlashGetCapabilities.
 *   E.g. if the _data_width_ is 4, addr modulo 4 must be zero.
 * @param[in] bytesCnt The number of bytes to read.
 * @param[in] dataIn Pointer to the memory where to place the data read from the flash partition.
 * @param[in] msecBlockTime.   The time how long the system waits to invoke the
 *   flash device command. Reason for a timeout could be, that another thread is
 *   currently invoking a flash operation for this flash device.
 * @return
 *      - osOK                   if the command was initiated successfully
 *      - osErrorOS              if not supported (Currently flashes with different
 *                                  sector sizes are not supported).
 *      - osErrorTimeoutResource if timeout appeared.
 *      - osErrorValue           if the combination of addr and cnt exceeds partly or
 *                                  fully the flash partition boundary.
 *      - osErrorNoMemory        if start address of the bytesCnt is not aligned
 *                                  with the dataItemWidth of the flash device,
 *                                  a temporary buffer must be allocated that
 *                                  can store more bytes than requested.
 *                                  osErrorNoMemory is returned if this memory
 *                                  allocation fails.
 */
C_FUNC osStatus_t osFlashReadBytes(osFlasDevicehHandle_t flashDeviceHandle, unsigned partitionIndex,
  uint32_t addr, void *dataIn, uint32_t bytesCnt, MsecType msecBlockTime);

C_INLINE osStatus_t osFlashDtReadBytes(osFlasDevicehHandle_t flashDeviceHandle, unsigned partitionIndex,
  uint32_t addr, void *dataIn, uint32_t bytesCnt) {
  return osFlashReadBytes(flashDeviceHandle, partitionIndex, addr, dataIn, bytesCnt, osFlashGetDefaultTimeout(flashDeviceHandle));
}

/**
 * \ingroup cmsis_os_ext_osFlash
 * \brief This function peeks the flash command completion state, or waits until an
 * invoked flash command is completed.
 *
 *
 * In a Non RTOS environment, the thread will wait by doing a spin loop until either
 * the pending command is finished or a timeout expired. In an RTOS environment, the thread
 * will go to sleep until either the pending command is finished or a timeout expired.
 *
 * The function can be called multiple times with or without invoking a new flash command
 * before. The function will always give back the result of the last performed
 * flash command.
 *
 * The function can be used just to peek whether the most recently invoked command is completed,
 * by passing a 0 to the msecBlockTime parameter.
 *
 * If there wasnn't any previously performed flash command, the flashCommandID component
 *  of the returned result will be _bapi_flash_CMDID_Invalid_. This is the case
 *  after function \ref osFlashOpenDevice returned.
 *
 * @param[in] flashDeviceHandle The flash device handle of the flash to wait for command
 *   completion.
 * @param[in] msecBlockTime The time after which the function should return early when
 *   because flash command has not yet been completed.
 * @return The command result structure.
 *   The result component of this structure will return
 *     - osOK,       if the flash command was completed successfully.
 *     - osErrorOS   if the flash command was completed with errors.
 *     - osTimeout   if the flash command could not be completed
 *                      within the given timeout.
 *
 */
C_FUNC struct _os_flash_cmd_result osFlashWaitCommandComplete(osFlasDevicehHandle_t flashDeviceHandle,
  MsecType msecBlockTime);


#endif /* #if BAPI_HAS_FLASH_DEVICE > 0 */

#endif /* osFlash_H_ */
