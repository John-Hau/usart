/*
 *  $HeadURL: $
 *
 *  $Date: $
 *  $Author: $
 */

/**
 * \file
 * \brief
 * This file defines the ARM CMSIS RTOS Flash Extension API.
 */
#include "osFlash.h"
#include <stdlib.h>
#include <string.h>


osStatus_t ARM_DRIVER_ERROR_to_osError(int32_t armDriverError) {
  switch(armDriverError) {
  case ARM_DRIVER_OK:
    return osOK;
  case ARM_DRIVER_ERROR_BUSY:
    return osErrorTimeout;
  case ARM_DRIVER_ERROR_TIMEOUT:
    return osErrorTimeout;
  case ARM_DRIVER_ERROR_PARAMETER:
    return osError;
  }
  return osError;
}


#if BAPI_HAS_FLASH_DEVICE > 0


#include "boards/cmsis/Driver_Flash.h"
#include "cmsis-driver/Driver_Flash.h"
#include "rtos/c++/osMailQueue.hpp"

STATIC int _osFlashGetFlashControllerIndex(enum bapi_E_FlashDevice flashDeviceIndex) {

  int retval = -1;
  switch(flashDeviceIndex) {
  case bapi_E_FlashDev0:
     retval = bapi_flash_FlashControllerIndex<0>::value;
     break;
#if BAPI_HAS_FLASH_DEVICE > 1
  case bapi_E_FlashDev1:
     retval = bapi_flash_FlashControllerIndex<1>::value;
     break;
#endif
#if BAPI_HAS_FLASH_DEVICE > 2
  case bapi_E_FlashDev2:
     retval = bapi_flash_FlashControllerIndex<2>::value;
     break;
#endif
#if BAPI_HAS_FLASH_DEVICE > 3
  case bapi_E_FlashDev3:
     retval = bapi_flash_FlashControllerIndex<3>::value;
     break;
#endif
#if BAPI_HAS_FLASH_DEVICE > 4
  #error "Too many flash devices defined, pleas enahance to the scheme abobe."
#endif
  }
  /*
   * When this assertion fails, you forgot to assign a flash controller index to the
   * flash device. Refer to macro #BAPI_FLASH_DEFINE_FLASH_CONTROLLER_INDEX.
   */
  ASSERT(retval >= 0);
  return retval;
}

struct _os_flash_mail {
  uint32_t event;                           /**< The CMSIS flash events: ARM_FLASH_EVENT_READY or
                                             *   ARM_FLASH_EVENT_ERROR. */
  enum bapi_E_FlashDevice flashDeviceIndex; /**< The flash device to which the mail is valid for. */
  enum bapi_flash_E_Command_ID_ flashCommandID;

#ifdef __cplusplus
  _os_flash_mail()
    : flashDeviceIndex(bapi_E_FlashDev_Invalid)
    , event(0)
    , flashCommandID(bapi_flash_CMDID_void) {

  }

  _os_flash_mail(enum bapi_E_FlashDevice flashDeviceIndex_, uint32_t event_,
        enum bapi_flash_E_Command_ID_ flashCommandID_)
    : flashDeviceIndex(flashDeviceIndex_)
    , event(event_)
    , flashCommandID(flashCommandID_) {
  }
#endif /* __cplusplus */
};


static const char* _osSyncItemName = "flashController";

struct _os_flash_controller_state {

  /** The flash device index for which the flash controller was occupied. */
  enum bapi_E_FlashDevice flashDeviceIndex;

#if TARGET_RTOS != RTOS_NoRTOS
  /** Only one thread can use the flash controller of the flash device simultaneously. */
  osMutexId_t mutex;
#endif

  void create() {
#if TARGET_RTOS != RTOS_NoRTOS
    //const osMutexDef_t mutexDef = {
    //  _osSyncItemName
    //};
    //mutex = osMutexCreate(&mutexDef);
    const osMutexAttr_t mutexDef = {_osSyncItemName, osMutexRecursive, NULL, 0};
    mutex = osMutexNew(&mutexDef);
    ASSERT(mutex);
#endif
  }

  _os_flash_controller_state()
    : flashDeviceIndex(bapi_E_FlashDev_Invalid)
#if TARGET_RTOS != RTOS_NoRTOS
    , mutex(0)
#endif
    {
  }
};


STATIC _os_flash_controller_state _osFlashControllerState[bapi_flashControllerCnt<>::value];


struct _os_flash_device_state {
  static _os_flash_device_state _osFlashDeviceState[bapi_E_FlashDevCount];

  uint32_t cmdDefaultTimeout;

  struct _os_flash_mail lastCommandResult;

  typedef os::MailQueue<struct _os_flash_mail> mail_queue_t;
  mail_queue_t mq;

  uint32_t dstLen;    /**< The number of bytes to be copied into dstBuffer */
  void* dst;          /**< The destination where to copy the contents of the srcBuffer after
                       *     read command is completed */
  uint8_t* cmdBuffer; /**< Temporary allocated buffer being used to read or program command */
  uint16_t srcOffset; /**< Offset from where to start copying srcBuffer to dstBuffer */

  uint16_t openCounter;

  void concludeCmdBuffer(struct _os_flash_mail* eventMail) {
    /* cmd buffer is only used for bapi_flash_CMDID_ReadData */
    if(cmdBuffer) {
      if(eventMail->flashCommandID == bapi_flash_CMDID_ReadData) {
        ASSERT(dst != 0);
        MEMCPY(dst, cmdBuffer + srcOffset, dstLen);
        dst = 0;
        free(cmdBuffer);
        cmdBuffer = 0;
      } else if(eventMail->flashCommandID == bapi_flash_CMDID_ProgramData) {
        ASSERT(dst == 0);
        free(cmdBuffer);
        cmdBuffer = 0;
      }
    }
  }

  inline uint8_t* allocateCmdBuffer(uint32_t bufferLen ,uint16_t _srcOffset, void* _dst, uint32_t _dstLen) {
    ASSERT(cmdBuffer == 0);
    ASSERT(dst == 0);
    cmdBuffer = S_CAST(uint8_t*, malloc(bufferLen));
    if(cmdBuffer) {
      dstLen = _dstLen; srcOffset = _srcOffset; dst = _dst;
    }
    return cmdBuffer;
  }

  void create() {
    osStatus_t status = mq.create(2 /* Number of queue elements */, _osSyncItemName);
    ASSERT(status == osOK);
    cmdDefaultTimeout = 90000;
  }

  /** The flash device index for which the flash controller was occupied. */
  enum bapi_E_FlashDevice flashDeviceIndex()const {
    const int index = (this - _osFlashDeviceState);
    return (enum bapi_E_FlashDevice) index;
  }

  int flashControllerIndex()const {
    return _osFlashGetFlashControllerIndex(flashDeviceIndex());
  }

  mail_queue_t& mailQueue() {
    return mq;
  }

#if TARGET_RTOS != RTOS_NoRTOS

  osMutexId_t& mutex() {
    return _osFlashControllerState[flashControllerIndex()].mutex;
  }
#endif

  void setLastCommandResult(const struct _os_flash_mail* eventMail) {
    lastCommandResult = *eventMail;
  }

  const struct _os_flash_mail* getLastCommandResult()const {
    return &lastCommandResult;
  }
};

_os_flash_device_state _os_flash_device_state::_osFlashDeviceState[bapi_E_FlashDevCount];


STATIC void _osLogError(const struct _os_flash_mail* eventMail) {
  osFlasDevicehHandle_t flashDeviceHandle = &_os_flash_device_state::_osFlashDeviceState[eventMail->flashDeviceIndex];
  if(eventMail->flashCommandID != bapi_flash_CMDID_void) {
    flashDeviceHandle->setLastCommandResult(eventMail);
  }
}


void osFlashStartupInit() {
  for(unsigned i = 0; i < ARRAY_SIZE(_osFlashControllerState); i++) {
    _osFlashControllerState[i].create();
  }

  for(unsigned i = 0; i < ARRAY_SIZE(_os_flash_device_state::_osFlashDeviceState); i++) {
    _os_flash_device_state::_osFlashDeviceState[i].create();
  }

}

STATIC osStatus_t _osFlashSendEvent(osFlasDevicehHandle_t flashDeviceHandle, const struct _os_flash_mail* eventMail) {
  osStatus_t status = osErrorNoMemory;
  //struct _os_flash_mail* mail = flashDeviceHandle->mailQueue().allocate();
  //ASSERT(mail); /* We call only _osFlashSendEvent, when the mail queue is empty. So there must be one entry free. */
  //if(mail) {
    //*mail = *eventMail;
    //status = flashDeviceHandle->mailQueue().put(mail);
  //}
  status = flashDeviceHandle->mailQueue().put(eventMail);
  return status;
}

STATIC osStatus_t _osFlashWaitEvent(struct _os_flash_mail* eventMail, osFlasDevicehHandle_t flashDeviceHandle, MsecType msecBlockTime) {

#if TARGET_RTOS == RTOS_NoRTOS

  /* Don't be surprised. The osKernetTick is also available in the NoRTOS world. */
  uint32_t firstTick = osKernelSysTick();
  uint32_t delta = 0;

  /* Just spin around until time expired */
  struct _os_flash_mail* mail = 0;
  do {
    mail = flashDeviceHandle->mailQueue().get(0);
    delta = osKernelSysTick() - firstTick;
  } while((!mail) && ((delta * 1000 / osKernelSysTickFrequency) < msecBlockTime));

#else
  //struct _os_flash_mail* mail = flashDeviceHandle->mailQueue().get(msecBlockTime);
  osStatus_t retStatus =  flashDeviceHandle->mailQueue().get(eventMail, msecBlockTime);
  if( osOK == retStatus ) {
    /* If we completed a read or program command, we must copy the command buffer and clean it up. */
    flashDeviceHandle->concludeCmdBuffer(eventMail);
  }
  return retStatus;
#endif

  //if(mail) {
  //  *eventMail = *mail;
  //  flashDeviceHandle->mailQueue().freeItem(mail);

    /* If we completed a read or program command, we must copy the command buffer and clean it up. */
  //  flashDeviceHandle->concludeCmdBuffer(eventMail);

  //  return osOK;
  //}
  //return osEventTimeout;
}


STATIC osStatus_t _osFlashLock(MsecType* remainingBlockTime, osFlasDevicehHandle_t flashDeviceHandle, MsecType msecBlockTime) {

#if TARGET_RTOS == RTOS_NoRTOS
  const osStatus_t status = osOK;

  if(remainingBlockTime)
  {
	  *remainingBlockTime = msecBlockTime;
  }

#else
  MsecType expired = osKernelGetSysTimerCount();
  const osStatus_t status = osMutexAcquire(flashDeviceHandle->mutex(), msecBlockTime);
  if(remainingBlockTime) {
    expired = osKernelGetSysTimerCount() - expired;
    *remainingBlockTime = (expired >= msecBlockTime) ? 0 : (msecBlockTime - expired);
  }
#endif
  return status;
}


inline osStatus_t _osFlashLock(MsecType* remainingBlockTime, osFlasDevicehHandle_t flashDeviceHandle) {
  return _osFlashLock(remainingBlockTime, flashDeviceHandle, flashDeviceHandle->cmdDefaultTimeout);
}

STATIC void _osFlashUnlock(osFlasDevicehHandle_t  flashDeviceHandle) {

  const enum bapi_E_FlashDevice flashDeviceIndex = flashDeviceHandle->flashDeviceIndex();

#if TARGET_RTOS == RTOS_NoRTOS
  const osStatus_t status = osOK;
#else
  const osStatus_t status = osMutexRelease(flashDeviceHandle->mutex());
#endif
  ASSERT(status == osOK);
}

static void _osFlashEventCallback(enum bapi_E_FlashDevice flashDeviceIndex, enum bapi_flash_E_Command_ID_ flashCommandID, uint32_t event) {

  const osFlasDevicehHandle_t flashDeviceHandle = &_os_flash_device_state::_osFlashDeviceState[flashDeviceIndex];
  struct  _os_flash_mail eventMail(
    flashDeviceIndex, event, flashCommandID
  );
  _osFlashSendEvent(flashDeviceHandle, &eventMail);
  return;
}

osStatus_t osFlashSetDefaultTimeout(osFlasDevicehHandle_t flashDeviceHandle, uint32_t msecTimeout) {
  if(flashDeviceHandle) {
    atomic_Set(&flashDeviceHandle->cmdDefaultTimeout, msecTimeout);
    return osOK;
  }
  return osError;
}

uint32_t osFlashGetDefaultTimeout(osFlasDevicehHandle_t flashDeviceHandle) {
  if(flashDeviceHandle) {
    return atomic_Get(&flashDeviceHandle->cmdDefaultTimeout);
  }
  return 0;
}

osFlasDevicehHandle_t osFlashOpenDevice(enum bapi_E_FlashDevice flashDeviceIndex, MsecType msecBlockTime) {

  osFlasDevicehHandle_t retval = nullptr;

  if(flashDeviceIndex != bapi_E_FlashDev_Invalid) {

    const int flashControllerIndex = _osFlashGetFlashControllerIndex(flashDeviceIndex);

    osStatus_t status = _osFlashLock(nullptr, &_os_flash_device_state::_osFlashDeviceState[flashDeviceIndex], msecBlockTime);

    if(status == osOK) {


      retval = &_os_flash_device_state::_osFlashDeviceState[flashDeviceIndex];


      if(!retval->openCounter) { /* This handle was opened the first time */

        ASSERT(retval->cmdBuffer == 0);
        ASSERT(retval->dst == 0);

        /* Set event callback for all partitions. */
        const unsigned partitionCnt = driver_flash_getPartitionCount(flashDeviceIndex);
        unsigned partitionIndex = 0; int32_t driverInitResult = ARM_DRIVER_OK;

        for(; (partitionIndex < partitionCnt) && (driverInitResult == ARM_DRIVER_OK); partitionIndex++) {
          const struct _ARM_DRIVER_FLASH* driver = driver_flash_getDriver(flashDeviceIndex, partitionIndex);
          ASSERT(driver);
          driverInitResult = driver->Initialize(_osFlashEventCallback);
        }

        /* If there was an error, we un-initialize what we already initialized so far. */
        if(driverInitResult != ARM_DRIVER_OK) {

          for(; (partitionIndex > 0) ; partitionIndex--) {
            const struct _ARM_DRIVER_FLASH* driver = driver_flash_getDriver(flashDeviceIndex, partitionIndex-1);
            driver->Uninitialize();
          }

          status = osError;


        } else {
          /* We have successfully initialized all partition drivers. */
          retval->openCounter++;

          /* Signal that we are ready for a flash command. */
          struct  _os_flash_mail eventMailVoid(
            flashDeviceIndex, ARM_FLASH_EVENT_READY, bapi_flash_CMDID_void
          );
          _osFlashSendEvent(retval, &eventMailVoid);
        }
      } else {
        retval->openCounter++; /* This handle was opened the second time. */
      }

      _osFlashUnlock(retval);

      if(status != osOK) {
        retval = nullptr;
      }
    }
  }
  return retval;
}

enum bapi_E_FlashDevice osFlashHandleToDeviceIndex(osFlasDevicehHandle_t  flashDeviceHandle) {
  return flashDeviceHandle->flashDeviceIndex();
}

uint32_t osFlashHandleToBaseAddress(osFlasDevicehHandle_t  flashDeviceHandle, unsigned partitionIndex) {
  const enum bapi_E_FlashDevice flashDeviceIndex = flashDeviceHandle->flashDeviceIndex();
  const struct _ARM_DRIVER_FLASH* driver = driver_flash_getDriver(flashDeviceIndex, partitionIndex);
  return driver->BaseAddress();
}

osStatus_t osFlashCloseDevice(osFlasDevicehHandle_t  flashDeviceHandle) {
  const enum bapi_E_FlashDevice flashDeviceIndex = flashDeviceHandle->flashDeviceIndex();

  MsecType remainingBlockTime;
  osStatus_t status = _osFlashLock(&remainingBlockTime, flashDeviceHandle);

  if (status == osOK) {

    if (flashDeviceHandle->openCounter == 1) { /* This is the last closure. */

      struct _os_flash_mail eventMail;

      /* Wait until pending command finished. */
      status = _osFlashWaitEvent(&eventMail, flashDeviceHandle, remainingBlockTime);

      ASSERT(status == osOK);

      ASSERT(flashDeviceHandle->cmdBuffer == 0);
      ASSERT(flashDeviceHandle->dst == 0);

      if (status == osOK) {
        _osLogError(&eventMail);

        unsigned partitionIndex = driver_flash_getPartitionCount(flashDeviceIndex);

        for ( ; (partitionIndex > 0); partitionIndex-- ) {
          const struct _ARM_DRIVER_FLASH* driver = driver_flash_getDriver(flashDeviceIndex, partitionIndex - 1);
          const int32_t driverUninitResult = driver->Uninitialize();
          ASSERT(driverUninitResult == ARM_DRIVER_OK);
        }


        flashDeviceHandle->openCounter--;
        ASSERT(flashDeviceHandle->openCounter == 0);
      }
    } else {
      /* Assert that the nesting counter will not underflow. */
      ASSERT(flashDeviceHandle->openCounter > 1);
      if(flashDeviceHandle->openCounter > 1) {
        flashDeviceHandle->openCounter--;
      }
    }
    _osFlashUnlock(flashDeviceHandle);
  }

  return status;
}

C_FUNC const ARM_FLASH_INFO* osFlashGetInfo(osFlasDevicehHandle_t flashDeviceHandle, unsigned partitionIndex) {
  const ARM_FLASH_INFO* retval = 0;

  bapi_irq_enterCritical();
  const enum bapi_E_FlashDevice flashDeviceIndex = flashDeviceHandle->flashDeviceIndex();
  bapi_irq_exitCritical();

  const struct _ARM_DRIVER_FLASH* driver = driver_flash_getDriver(flashDeviceIndex, partitionIndex);
  if(driver) {
    retval = driver->GetInfo();
  }
  return retval;
}

unsigned osFlashGetPartitionCount(osFlasDevicehHandle_t flashDeviceHandle) {
  bapi_irq_enterCritical();
  const enum bapi_E_FlashDevice flashDeviceIndex = flashDeviceHandle->flashDeviceIndex();
  bapi_irq_exitCritical();
  return driver_flash_getPartitionCount(flashDeviceIndex);
}

ARM_FLASH_CAPABILITIES osFlashGetCapabilities(osFlasDevicehHandle_t flashDeviceHandle, unsigned partitionIndex) {
  ARM_FLASH_CAPABILITIES retval = {0};

  bapi_irq_enterCritical();
  const enum bapi_E_FlashDevice flashDeviceIndex = flashDeviceHandle->flashDeviceIndex();
  bapi_irq_exitCritical();

  const struct _ARM_DRIVER_FLASH* driver = driver_flash_getDriver(flashDeviceIndex, partitionIndex);
  if(driver) {
    retval = driver->GetCapabilities();
  }

  return retval;
}

uint32_t osFlashPartitionSizeBytes(osFlasDevicehHandle_t flashDeviceHandle, unsigned partitionIndex) {
  ARM_FLASH_INFO* partitionInfo = osFlashGetInfo(flashDeviceHandle, partitionIndex);
  return driver_flash_calculateSize(partitionInfo);
}

uint32_t osFlashBlockSizeBytes(osFlasDevicehHandle_t flashDeviceHandle, unsigned partitionIndex, uint32_t blockAddr) {
  ARM_FLASH_INFO* partitionInfo = osFlashGetInfo(flashDeviceHandle, partitionIndex);
  return driver_flash_calculateBlockSize(partitionInfo, blockAddr);
}

uint32_t osFlashReadData_BytesToDataItemCount(osFlasDevicehHandle_t flashDeviceHandle, unsigned partitionIndex, uint32_t bytesCnt) {
  ARM_FLASH_CAPABILITIES capabilities = osFlashGetCapabilities(flashDeviceHandle, partitionIndex);
  unsigned dataWidth = ARM_Flash_capabilitiesToDataWidthInBytes(capabilities);
  return (bytesCnt + dataWidth - 1) / dataWidth; /* Return the enlarged data item count. */
}

uint32_t osFlashProgramData_BytesToDataItemCount(osFlasDevicehHandle_t flashDeviceHandle,
  unsigned partitionIndex, uint32_t bytesCnt) {
  ARM_FLASH_INFO* partitionInfo = osFlashGetInfo(flashDeviceHandle, partitionIndex);
  return bytesCnt / partitionInfo->program_unit; /* Return the cut off data item count. */
}

C_INLINE osStatus_t _osFlashWaitCommandComplete(osFlasDevicehHandle_t flashDeviceHandle, MsecType msecBlockTime)
  {
  struct _os_flash_mail eventMail;
  osStatus_t status = _osFlashWaitEvent(&eventMail, flashDeviceHandle, msecBlockTime);
  if (status == osOK) {
    _osLogError(&eventMail);
    /* Signal that we are ready for the next flash command. */
    struct _os_flash_mail eventMailVoid(flashDeviceHandle->flashDeviceIndex(), ARM_FLASH_EVENT_READY, bapi_flash_CMDID_void);
    _osFlashSendEvent(flashDeviceHandle, &eventMailVoid);
  }
  return status;
}

osStatus_t osFlashReadData(osFlasDevicehHandle_t flashDeviceHandle, unsigned partitionIndex,
  uint32_t addr, void *dataIn, uint32_t dataItemCnt, MsecType msecBlockTime) {

  MsecType remainingBlockTime;
  osStatus_t status =  _osFlashLock(&remainingBlockTime, flashDeviceHandle, msecBlockTime);

  if(status == osOK) {
    struct _os_flash_mail eventMail;

    /* Wait until pending command finished. */
    status = _osFlashWaitEvent(&eventMail, flashDeviceHandle, remainingBlockTime);

    if(status == osOK) {
      _osLogError(&eventMail);
      const struct _ARM_DRIVER_FLASH* driver = driver_flash_getDriver(flashDeviceHandle->flashDeviceIndex(), partitionIndex);

      /* Read the data and wait until read. */
      status = ARM_DRIVER_ERROR_to_osError(driver->ReadData(addr, dataIn, dataItemCnt) );
    }
    _osFlashUnlock(flashDeviceHandle);
  }

  return status;
}

#if OS_FLASH_FLASH_PROGRAM_BYTES

osStatus_t osFlashProgramBytes(osFlasDevicehHandle_t flashDeviceHandle, unsigned partitionIndex,
  uint32_t addr, const void *dataOut, uint32_t bytesCnt, MsecType msecBlockTime) {

  if(bytesCnt) {

    const ARM_FLASH_INFO* flashInfo = osFlashGetInfo(flashDeviceHandle, partitionIndex);

    const unsigned programUnit = flashInfo->program_unit;
    const int frontPaddingBytes = addr % programUnit;

    osStatus_t retval = osErrorNoMemory;

    if(frontPaddingBytes) {
      /* Start address is not aligned with dataWidth */
      uint32_t dataItemCount = (frontPaddingBytes + bytesCnt + programUnit - 1) / programUnit;

      /* Allocate a buffer that is aligned with the flash's program unit */
      uint8_t* _buffer = flashDeviceHandle->allocateCmdBuffer(dataItemCount * programUnit, frontPaddingBytes, 0, 0);

      if(_buffer) {

        /* Fill front block with erased value (typically 0xff) */
        MEMSET(_buffer, flashInfo->erased_value, programUnit);

        if(dataItemCount > 1) {
          /* Fill end block with erased value (typically 0xff) */
          MEMSET(&_buffer[(dataItemCount-1) * programUnit],
            flashInfo->erased_value, programUnit);
        }

        /* Copy data to buffer. */
        MEMCPY(&_buffer[frontPaddingBytes], dataOut, bytesCnt);

        retval = osFlashProgramData(flashDeviceHandle, partitionIndex, addr-frontPaddingBytes, _buffer, dataItemCount, msecBlockTime);
        /* Note: allocated buffer will be freed, when data is written. */
      }
    } else {
      uint32_t dataItemCount = (bytesCnt + programUnit - 1) / programUnit;
      if(dataItemCount * programUnit != bytesCnt) {
        /* End address is not aligned with dataWidth */

        /* Allocate a buffer that is aligned with the flash's program unit */
        uint8_t* _buffer = flashDeviceHandle->allocateCmdBuffer(dataItemCount * programUnit, 0, 0, 0);
        if(_buffer) {

          /* Fill end block with erased value (typically 0xff) */
          MEMSET(&_buffer[(dataItemCount-1) * programUnit], /* Note that dataItemCount can't be 0 because byteCnt isn't 0. */
            flashInfo->erased_value, programUnit);

          /* Copy data to buffer. */
          MEMCPY(_buffer, dataOut, bytesCnt);

          retval = osFlashProgramData(flashDeviceHandle, partitionIndex, addr-frontPaddingBytes, _buffer, dataItemCount, msecBlockTime);
          /* Note: allocated buffer will be freed, when data is read and copied to dataIn. */
        }
      } else {
        /* Start address and End address are aligned with dataWidth */
        retval = osFlashProgramData(flashDeviceHandle, partitionIndex, addr, dataOut, dataItemCount, msecBlockTime);
      }
    }
    return retval;
  }

  /* zero bytes to program */
  return osOK;

}

#endif

osStatus_t osFlashReadBytes(osFlasDevicehHandle_t flashDeviceHandle, unsigned partitionIndex,
  uint32_t addr, void *dataIn, uint32_t bytesCnt, MsecType msecBlockTime) {

  if(bytesCnt) {
    const ARM_FLASH_CAPABILITIES capabilities = osFlashGetCapabilities(flashDeviceHandle, partitionIndex);

    const unsigned dataWidth = ARM_Flash_capabilitiesToDataWidthInBytes(capabilities);
    const int frontPaddingBytes = addr % dataWidth;

    osStatus_t retval = osErrorNoMemory;

    if(frontPaddingBytes) {
      /* Start address is not aligned with dataWidth */
      uint32_t dataItemCount = (frontPaddingBytes + bytesCnt + dataWidth - 1) / dataWidth;

      /* Allocate a buffer that is aligned with the flash's data width */
      uint8_t* _buffer = flashDeviceHandle->allocateCmdBuffer(dataItemCount * dataWidth, frontPaddingBytes, dataIn, bytesCnt);
      if(_buffer) {
        retval = osFlashReadData(flashDeviceHandle, partitionIndex, addr-frontPaddingBytes, _buffer, dataItemCount, msecBlockTime);
        /* Note: allocated buffer will be freed, when data is read and copied to dataIn. */
      }

    } else {
      uint32_t dataItemCount = (bytesCnt + dataWidth - 1) / dataWidth;
      if(dataItemCount * dataWidth != bytesCnt) {
        /* End address is not aligned with dataWidth */

        /* Allocate a buffer that is aligned with the flash's data width */
        uint8_t* _buffer = flashDeviceHandle->allocateCmdBuffer(dataItemCount * dataWidth, 0, dataIn, bytesCnt);
        if(_buffer) {
          retval = osFlashReadData(flashDeviceHandle, partitionIndex, addr, _buffer, dataItemCount, msecBlockTime);
          /* Note: allocated buffer will be freed, when data is read and copied to dataIn. */
        }
      } else {
        /* Start address and End address are aligned with dataWidth */
        retval = osFlashReadData(flashDeviceHandle, partitionIndex, addr, dataIn, dataItemCount, msecBlockTime);
      }
    }
    return retval;
  }

  /* zero bytes to read */
  return osOK;
}

osStatus_t osFlashEraseSector(osFlasDevicehHandle_t flashDeviceHandle, unsigned partitionIndex,
  uint32_t addr, MsecType msecBlockTime) {

  MsecType remainingBlockTime;
  osStatus_t status = _osFlashLock(&remainingBlockTime, flashDeviceHandle, msecBlockTime);

  if(status == osOK) {
    struct _os_flash_mail eventMail;

    status = _osFlashWaitEvent(&eventMail, flashDeviceHandle, remainingBlockTime);
    if(status == osOK) {
      _osLogError(&eventMail);
      const struct _ARM_DRIVER_FLASH* driver = driver_flash_getDriver(flashDeviceHandle->flashDeviceIndex(), partitionIndex);
      status = ARM_DRIVER_ERROR_to_osError(driver->EraseSector(addr));
    }
    _osFlashUnlock(flashDeviceHandle);
  }
  return status;
}

#if OS_FLASH_ERASE_BLOCK
osStatus_t osFlashEraseBlock(osFlasDevicehHandle_t flashDeviceHandle, unsigned partitionIndex,
  uint32_t blockAddr, MsecType msecBlockTime) {

  MsecType remainingBlockTime;
  osStatus_t status = _osFlashLock(&remainingBlockTime, flashDeviceHandle, msecBlockTime);

  if(status == osOK) {
    struct _os_flash_mail eventMail;

    status = _osFlashWaitEvent(&eventMail, flashDeviceHandle, remainingBlockTime);
    if(status == osOK) {
      _osLogError(&eventMail);
      const struct _ARM_DRIVER_FLASH* driver = driver_flash_getDriver(flashDeviceHandle->flashDeviceIndex(), partitionIndex);
      status = ARM_DRIVER_ERROR_to_osError(driver->EraseBlock(blockAddr));
    }
    _osFlashUnlock(flashDeviceHandle);
  }
  return status;
}
#endif /* OS_FLASH_ERASE_BLOCK*/


osStatus_t osFlashErasePartition(osFlasDevicehHandle_t flashDeviceHandle, unsigned partitionIndex,
  MsecType msecBlockTime) {

  MsecType remainingBlockTime;
  osStatus_t status = _osFlashLock(&remainingBlockTime, flashDeviceHandle, msecBlockTime);

  if(status == osOK) {
    struct _os_flash_mail eventMail;
    status = _osFlashWaitEvent(&eventMail, flashDeviceHandle, remainingBlockTime);
    if(status == osOK) {
      _osLogError(&eventMail);
      const struct _ARM_DRIVER_FLASH* driver = driver_flash_getDriver(flashDeviceHandle->flashDeviceIndex(), partitionIndex);
      status = ARM_DRIVER_ERROR_to_osError(driver->EraseChip());
    }
    _osFlashUnlock(flashDeviceHandle);
  }
  return status;
}

osStatus_t osFlashEnablePartitionProtection(osFlasDevicehHandle_t flashDeviceHandle, unsigned partitionIndex,
  MsecType msecBlockTime) {

  MsecType remainingBlockTime;
  osStatus_t status = _osFlashLock(&remainingBlockTime, flashDeviceHandle, msecBlockTime);

  if(status == osOK) {
#if 0  //PS TODO Protection implementation
    struct _os_flash_mail eventMail;
    status = _osFlashWaitEvent(&eventMail, flashDeviceHandle, remainingBlockTime);
    if(status == osOK) {
      _osLogError(&eventMail);
      const struct _ARM_DRIVER_FLASH* driver = driver_flash_getDriver(flashDeviceHandle->flashDeviceIndex(), partitionIndex);
      const int32_t result = driver->EnableProtection();

      status = ARM_DRIVER_ERROR_to_osError(result);
    }
#endif
    _osFlashUnlock(flashDeviceHandle);
  }
  return status;

}

osStatus_t osFlashDisablePartitionProtection(osFlasDevicehHandle_t flashDeviceHandle, unsigned partitionIndex,
  MsecType msecBlockTime) {

  MsecType remainingBlockTime;
  osStatus_t status = _osFlashLock(&remainingBlockTime, flashDeviceHandle, msecBlockTime);

  if(status == osOK) {
#if 0 //PS TODO Protection implementation
    struct _os_flash_mail eventMail;
    status = _osFlashWaitEvent(&eventMail, flashDeviceHandle, remainingBlockTime);
    if(status == osOK) {
      _osLogError(&eventMail);
      const struct _ARM_DRIVER_FLASH* driver = driver_flash_getDriver(flashDeviceHandle->flashDeviceIndex(), partitionIndex);
      const int32_t result = driver->DisableProtection();
      status = ARM_DRIVER_ERROR_to_osError(result);
    }
#endif
    _osFlashUnlock(flashDeviceHandle);
  }
  return status;
}


C_FUNC osStatus_t osFlashFreezeSectorLockdown(osFlasDevicehHandle_t flashDeviceHandle, MsecType msecBlockTime) {

  MsecType remainingBlockTime;
  osStatus_t status = _osFlashLock(&remainingBlockTime, flashDeviceHandle, msecBlockTime);

  if(status == osOK) {
#if 0 //PS TODO
    struct _os_flash_mail eventMail;
    status = _osFlashWaitEvent(&eventMail, flashDeviceHandle, remainingBlockTime);
    if(status == osOK) {
      _osLogError(&eventMail);
      const struct _ARM_DRIVER_FLASH* driver = driver_flash_getDriver(flashDeviceHandle->flashDeviceIndex(), 0);
      const int32_t result = driver->FreezeSectorLockdown();
      status = ARM_DRIVER_ERROR_to_osError(result);
    }
#endif
    _osFlashUnlock(flashDeviceHandle);
  }

  return status;
}


//#if _DEBUG_FLASH_SECTOR_INFO

osStatus_t osFlashReadSectorLockdown(osFlasDevicehHandle_t flashDeviceHandle, MsecType msecBlockTime,
  uint8_t* buffer, unsigned bufferSize) {

  MsecType remainingBlockTime;
  osStatus_t status = _osFlashLock(&remainingBlockTime, flashDeviceHandle, msecBlockTime);

  if(status == osOK) {
#if 0 //PS TODO
    struct _os_flash_mail eventMail;
    status = _osFlashWaitEvent(&eventMail, flashDeviceHandle, remainingBlockTime);
    if(status == osOK) {
      _osLogError(&eventMail);
      const struct _ARM_DRIVER_FLASH* driver = driver_flash_getDriver(flashDeviceHandle->flashDeviceIndex(), 0);
      const int32_t result = driver->ReadSectorLockdown(buffer, bufferSize);
      status = ARM_DRIVER_ERROR_to_osError(result);
    }
#endif
    _osFlashUnlock(flashDeviceHandle);
  }
  return status;

}

C_FUNC osStatus_t osFlashDtReadSectorLockdown(osFlasDevicehHandle_t flashDeviceHandle,
  uint8_t* buffer, unsigned bufferSize) {
  //return osFlashReadSectorLockdown(flashDeviceHandle, flashDeviceHandle->cmdDefaultTimeout, buffer, bufferSize);
  //PS TODO: Sector lockdown
	return osOK;
}

osStatus_t osFlashReadSectorProtection(osFlasDevicehHandle_t flashDeviceHandle, MsecType msecBlockTime,
  uint8_t* buffer, unsigned bufferSize) {

  MsecType remainingBlockTime;
  osStatus_t status = _osFlashLock(&remainingBlockTime, flashDeviceHandle, msecBlockTime);

  if(status == osOK) {
#if 0
    struct _os_flash_mail eventMail;
    status = _osFlashWaitEvent(&eventMail, flashDeviceHandle, remainingBlockTime);
    if(status == osOK) {
      _osLogError(&eventMail);
      const struct _ARM_DRIVER_FLASH* driver = driver_flash_getDriver(flashDeviceHandle->flashDeviceIndex(), 0);
      const int32_t result = driver->ReadSectorProtection(buffer, bufferSize);
      status = ARM_DRIVER_ERROR_to_osError(result);
    }
#endif
    _osFlashUnlock(flashDeviceHandle);
  }
  return status;

}

C_FUNC osStatus_t osFlashDtReadSectorProtection(osFlasDevicehHandle_t flashDeviceHandle,
  uint8_t* buffer, unsigned bufferSize) {
  //return osFlashReadSectorProtection(flashDeviceHandle, flashDeviceHandle->cmdDefaultTimeout, buffer, bufferSize);
  //PS TODO: Implement sextor protection
	return osOK;
}

//#endif

osStatus_t osFlashProgramData(osFlasDevicehHandle_t flashDeviceHandle, unsigned partitionIndex,
  uint32_t addr, const void *dataOut, uint32_t cnt, MsecType msecBlockTime) {

  MsecType remainingBlockTime;
  osStatus_t status = _osFlashLock(&remainingBlockTime, flashDeviceHandle, msecBlockTime);

  if(status == osOK) {
    struct _os_flash_mail eventMail;
    status = _osFlashWaitEvent(&eventMail, flashDeviceHandle, remainingBlockTime);
    if(status == osOK) {
      _osLogError(&eventMail);
      const struct _ARM_DRIVER_FLASH* driver = driver_flash_getDriver(flashDeviceHandle->flashDeviceIndex(), partitionIndex);
      status = ARM_DRIVER_ERROR_to_osError(driver->ProgramData(addr, dataOut, cnt));
    }
    _osFlashUnlock(flashDeviceHandle);
  }
  return status;
}

struct _os_flash_cmd_result osFlashWaitCommandComplete(osFlasDevicehHandle_t flashDeviceHandle, MsecType msecBlockTime) {

  struct _os_flash_cmd_result commandResult = {
      osErrorTimeout, bapi_flash_CMDID_Invalid
  };

  MsecType remainingBlockTime;
  osStatus_t status = _osFlashLock(&remainingBlockTime, flashDeviceHandle, msecBlockTime);

#if TARGET_RTOS == RTOS_FreeRTOS
  /* I am seeing the following behaviour in case of FreeRTOS.
   * Sometimes, a task that is waiting for a os_mail is not
   * woken immediately, when a mail is there. The timeout
   * needs to expire so that the task is woken. The returned status
   * of osMailGet(..) will be osEventMail since a mail
   * was received, but just the task wasn't woken immediately.
   * It cannot be reproduced with the debugger, because the
   * problem does not appear when a breakpoint is hit
   * before the osMailGet(..) call. It seems that an osThreadYield().
   * before the osMailGet(..) will fix the problem.
   * It was also observed that creating a stack variable somewhere
   * in the call chain before the osMailGet(..) happens, will solve
   * the problem sometimes. The behaviour was observed with
   * gcc compiler without optimization and FreeRTOS. It is
   * unsure if other combinations may cause this problem as
   * well. However, in a situation when that problem appeared
   * with gcc and FreeRTOS, it didn't appear with iar and
   * FreeRTOS in the same situation. The problem has been
   * observed 2 times within 2 years now. Once along with the implementation
   * of the usb device driver and another time along with
   * the serial flash driver: The flash driver puts a mail
   * into the mail queue, but the waiting task is not woken
   * immediately. Strange !!! */
#if 0 // The workaround is switched off by default.
  // TODO: Find root cause and remove this workaround for a delays as described above thread.
  osThreadYield();
#endif
#endif

  if(status != osErrorTimeout) {
    status = _osFlashWaitCommandComplete(flashDeviceHandle, remainingBlockTime);
    if(status != osErrorTimeout) {
      commandResult.flashCommandID = flashDeviceHandle->getLastCommandResult()->flashCommandID;
      commandResult.result = (flashDeviceHandle->getLastCommandResult()->event & ARM_FLASH_EVENT_ERROR) ?
        osError : osOK;
    }
    _osFlashUnlock(flashDeviceHandle);
  }

  return commandResult;
}

#endif /* #if BAPI_HAS_FLASH_DEVICE > 0 */
