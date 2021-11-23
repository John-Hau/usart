/*
 *  $HeadURL: $
 *
 *  $Date: $
 *  $Author: $
 */


/**
 * /file
 * /brief This file Implements the cmsis-driver API for FLASH and implements the functions
 * that provide instances of ARM_DRIVER_FLASH structures for each FLASH partition that is
 * defined by the board API (refer to bapi_flash.h).
 */


#include "baseplate.h"

#include "boards/board-api/bapi_flash.h"
#include "boards/board-api/bapi_atomic.h"
#include "Driver_Flash.h"

#if (BAPI_HAS_FLASH_DEVICE > 0)

#define ARM_FLASH_DRV_VERSION    ARM_DRIVER_VERSION_MAJOR_MINOR(2, 0)  /* driver version */



namespace Driver_FLASH {

/**
 * \ingroup _cmsis_driver_flash
 * \brief Namespace for flash driver corresponding interface functions
 */
  /* Driver Version */
STATIC const ARM_DRIVER_VERSION driverVersion = {
    ARM_FLASH_API_VERSION,
    ARM_FLASH_DRV_VERSION
};


/**
 * \ingroup _cmsis_driver_flash
 *
 * Calculate the number of sectors needed to cover the required partition size by the invocation of
 *   macro \ref #FLASH_DEFINE_PARTITIONS.
 *
 * This function must be used in case that all sectors have the same size.
 *
 * @param requiredPartitionSizeInKbytes[in] The partitions size to be covered by the flash sectors.
 * @param sectorSize[in]    The size of one sector of the flash.
 * @param numSectors[in]    The number of available subsequent sectors to cover the
 *   the required partition size.
 *
 * @return The number of sectors that cover the required partition size if sufficient
 *   sectors are available. Otherwise 0.
 */
C_INLINE uint32_t calculateNumSectorsBySectorSize(_flash_partition_size_t requiredPartitionSizeInKbytes
  , uint32_t sectorSize, uint16_t max_sector_count, uint32_t* partitionSizeResult) {
  uint32_t retval = driver_flash_partitionSizeInBytes(requiredPartitionSizeInKbytes);
  retval = (retval + sectorSize - 1) / sectorSize;

  if(partitionSizeResult) {
    *partitionSizeResult = retval * sectorSize;
  }

  return retval > max_sector_count ? 0 : retval;
}

/**
 * \ingroup _cmsis_driver_flash
 *
 * Calculate the number of sectors needed to cover the required partition size by the invocation of
 *   macro \ref #FLASH_DEFINE_PARTITIONS.
 *
 * This function must be used in case the flash has sectors with different sizes.
 *
 * @param partitionSize[in]         The partitions size to be covered by the flash sectors.
 * @param flashInfo[in]             The flash info of the physical flash device.
 * @param startSectorIndex[in]      The sector from which to start the partition.
 * @param partitionSizeResult[out]  The resulting partition size in bytes.
 *  The resulting partition size may be bigger than required, because of the granularity
 *  of the sectors of the physical flash device.
 *
 * @return The number of sectors that cover the required partition size if sufficient
 *   sectors are available. Otherwise 0.
 */
C_INLINE uint32_t calculateNumSectorsBySectorInfo(_flash_partition_size_t requiredPartitionSizeInKbytes
  , const ARM_FLASH_INFO *flashInfo, uint16_t startSectorIndex, uint32_t* partitionSizeResult) {

  /* The partition size in bytes to be covered by the flash sectors. */
  uint32_t requiredPartitionSize = driver_flash_partitionSizeInBytes(requiredPartitionSizeInKbytes);

  uint32_t coveredPartitionSizeInBytes = 0;
  uint16_t sectorIndex = startSectorIndex;


  struct _ARM_FLASH_SECTOR sectorInfo;

  /*
   * loop over sectors until  the required partition size is covered
   * or no more sectors are available or until we have crossed the
   * number of available sector infos.
   */

  /* Try getting the first sector info. */
  uint16_t originSectorInfoIndex = ARM_Flash_getSectorInfoAt(&sectorInfo, flashInfo, sectorIndex);

  while(
        (coveredPartitionSizeInBytes < requiredPartitionSize)  /* partition size covered ?  */
     && (sectorIndex < flashInfo->sector_count)        /* sectors still available ? */
     && ((originSectorInfoIndex) == sectorIndex)       /* sector infos available ?  */
    ) {

    /* Add up the covered partition size. */
    coveredPartitionSizeInBytes += sectorInfo.sectorSize();

    /* Try getting the next sector info. */
    originSectorInfoIndex =
      ARM_Flash_getSectorInfoAt(&sectorInfo, flashInfo, ++sectorIndex);

  }


  /*
   * We have covered all sectors that provide sector infos. But we may need
   * to add subsequent uniform sectors that have the same size as the sector
   * associated with the last sector info.
   */

  if((coveredPartitionSizeInBytes < requiredPartitionSize) && (sectorIndex < flashInfo->sector_count) ) {

    /* If we haven't achieved the required partition size and also not the end of
     * available sectors, the reason for the above loop exit must have been that
     * the last sector info in the above loop was crossed. We assert this condition.
     */
    ASSERT(originSectorInfoIndex < sectorIndex);


    {
      const uint32_t sizeOfEachRemainingSector = sectorInfo.sectorSize();
      const uint32_t uncoveredPartitionSize = requiredPartitionSize - coveredPartitionSizeInBytes;

      const uint16_t additionalSectorCnt =
        (uncoveredPartitionSize + sizeOfEachRemainingSector - 1) / sizeOfEachRemainingSector;

      sectorIndex += additionalSectorCnt;
      coveredPartitionSizeInBytes += additionalSectorCnt * sizeOfEachRemainingSector;
    }

  }

  if(partitionSizeResult) {
    *partitionSizeResult = coveredPartitionSizeInBytes;
  }

  return (sectorIndex <= flashInfo->sector_count) ? sectorIndex - startSectorIndex : 0;
}

/**
 * \ingroup _cmsis_driver_flash
 *
 * Calculate the number of sectors needed to cover the required partition size by the invocation of
 *   macro \ref #FLASH_DEFINE_PARTITIONS
 *
 * @param partitionSizeInKbytes[in] The partitions size to be covered by the flash sectors.
 * @param flashInfo[in]             The flash info of the physical flash device.
 * @param startSector[in]           The sector from which to start the partition.
 * @param partitionSizeResult[out]  The resulting partition size in bytes.
 *  The resulting partition size may be bigger than required, because of the granularity
 *  of the sectors of the physical flash device.
 *
 * @return The number of sectors that cover the required partition size if sufficient
 *   sectors are available. Otherwise 0.
 */
C_INLINE uint32_t calculateNumSectors(_flash_partition_size_t requiredPartitionSizeInKbytes
  , const ARM_FLASH_INFO *flashInfo, uint16_t startSector, uint32_t* partitionSizeResult) {

  ASSERT(startSector < flashInfo->sector_count);

  if(ARM_Flash_hasUniformSectors(flashInfo)) {
    /* we have equally sized sectors, so we can calculate by sector size. */
    return calculateNumSectorsBySectorSize(requiredPartitionSizeInKbytes
      ,flashInfo->sector_size, flashInfo->sector_count - startSector
      ,partitionSizeResult);
  }

  return calculateNumSectorsBySectorInfo(requiredPartitionSizeInKbytes, flashInfo
    , startSector, partitionSizeResult);
}

/**
 * \ingroup _cmsis_driver_flash
 *
 * A structure that extends the standard _ARM_FLASH_INFO structure to carry additional
 *  data, that is required for a logical flash partition. This is the info about the
 *  physical flash device to which the partition belongs as well as the byte offset
 *  of the partition within the physical device.
 */
struct FlashPartitionInfo : public _ARM_FLASH_INFO {

  uint32_t m_flashDeviceAddressOffset; /**< The start address of this partition within the flash device */
  uint16_t m_flashDeviceSectorOffset;  /**< The first sector of this partition within the flash device */
  enum bapi_E_FlashDevice m_flashDeviceIndex; /**< The flash device of this partition */

  /**
   * The partition info must be initialized before the partition can be used. This function
   *  ensures, that the partition info is initialized, if not done already.
   */
  void ensureInitialized() {

    /* If null is returned by bapi_flash_getFlashInfoUserData, this partition info is already
     * initialized, and we must skip the initialization.
     */
    const _flash_partition_size_t* partitionSizesInKbyte =
      S_CAST(const _flash_partition_size_t*, bapi_flash_getFlashInfoUserData(this));

    if (partitionSizesInKbyte) {


      /* The page size temporarily contains the partition index in case this partition info
       * is not initialized. So we pick it up here and go ahead with the initialization.
       */
      _flash_partition_size_t partitionIndex = page_size;

      const struct bapi_flash_api* driver = bapi_flash_getDriver(m_flashDeviceIndex);
      ASSERT(driver);

      const struct _ARM_FLASH_INFO* flashDeviceInfo = driver->GetFlashDeviceInfo(m_flashDeviceIndex);

      *S_CAST(_ARM_FLASH_INFO*, this) = *flashDeviceInfo;


      /* Determine the start sector, start address and the required number of sectors. */
      unsigned p = 0;
      while ( p < partitionIndex ) {

        /* Add number of previous sectors */
        uint32_t addressOffset = 0;

        const uint32_t sectorCnt = calculateNumSectors(partitionSizesInKbyte[p], flashDeviceInfo
          , m_flashDeviceSectorOffset, &addressOffset);

        ASSERT(sectorCnt); /* 0 signals a sector overflow. */

        m_flashDeviceSectorOffset  += sectorCnt;
        m_flashDeviceAddressOffset += addressOffset;

        ++p;
      }

      /* Adjust our sector info */
      if (! ARM_Flash_hasUniformSectors(flashDeviceInfo)) {
        /* We have differently sized sectors. */

        struct _ARM_FLASH_SECTOR sectorInfoTmp;

        uint16_t originSectorInfoIndex = ARM_Flash_getSectorInfoAt(&sectorInfoTmp, flashDeviceInfo, m_flashDeviceSectorOffset);

        if(originSectorInfoIndex < m_flashDeviceSectorOffset) {
          /* if our first sector is based on the origin sector's last sector
           * info, we can assume uniform sectors for our partition. */
          sector_info = 0;
          sector_size = sectorInfoTmp.sectorSize();
        } else {
          /* Otherwise we take over the sector info from the physical flash device. */
          sector_info = &flashDeviceInfo->sector_info[m_flashDeviceSectorOffset];
          sector_info_count = flashDeviceInfo->sector_info_count - m_flashDeviceSectorOffset;
        }
      }

      if (!partitionSizesInKbyte[p]) {
        /* It is the last partition */
        sector_count = flashDeviceInfo->sector_count - m_flashDeviceSectorOffset;

        /* Assert we have no sector overflow. */
        ASSERT(flashDeviceInfo->sector_count > m_flashDeviceSectorOffset);

      }
      else {
        sector_count = calculateNumSectors(partitionSizesInKbyte[p]
          , flashDeviceInfo, m_flashDeviceSectorOffset, nullptr);
        ASSERT(sector_count); /* 0 signals a sector overflow. */
      }

      /* After initialization is done, the user data must have become obsolete. */
      ASSERT(0 == bapi_flash_getFlashInfoUserData(this));
    }
  }

  FlashPartitionInfo(
    enum bapi_E_FlashDevice flashDeviceIndex
    , _flash_partition_size_t partitionIndex /**< The partition index of this partition within the flash device. */
    , const _flash_partition_size_t* partitionSizes
    ) : m_flashDeviceAddressOffset(0), m_flashDeviceSectorOffset(0), m_flashDeviceIndex(flashDeviceIndex) {

    /* Note: this constructor is called at a very early startup stage when static
     * variables are initialized. At this stage, no spi drivers are setup. Hence
     * we cannot read spi flash info and therefore not initialize this partition
     * info. So we defer the initialization. We store partitionSizes and partition index
     * in this PARTITION info. This data is required by deferred initialization function.
     * Once the initialization is done, the partitionSizes and partition index
     * is overridden. */

    /* Make flash info invalid, and store partitions sizes for later initialization. */
    bapi_flash_invalidateFlashInfo(this, partitionSizes);

    /* Store the partition index temporarily in the page_size, so that we have it
     * available at later initialization */
    page_size = partitionIndex;
  }
};


/**
 * \ingroup _cmsis_driver_flash
 * \brief The single callback function, that will be called by the bapi layer for
 *   any flash command of any flash partition. It is used to signal a successful
 *   or unsuccessful completion event of a flash command.
 *
 *   @param cookie will carry a pointer to a FlashPartition object.
 *
 *   For description of other parameters refer to \ref bapi_flash_callback_t.
 *
 */
static void myBapiFlash_ISRCallback(enum bapi_E_FlashDevice flashDeviceIndex,
  int32_t armDriverState, enum bapi_flash_E_Command_ID_ flashCommandID, uint32_t cookie);


/**
 * \ingroup _cmsis_driver_flash
 * \brief
 *
 * This class implements the API functions defined by the \ref struct _ARM_FLASH_DRIVER.
 *   for all partitions of all physical flash devices.
 */
class FlashPartition
{

  struct FlashPartitionInfo    m_partitionInfo;
  ARM_Flash_SignalEvent_t m_eventCallback;
  ARM_FLASH_STATUS        m_flashStatus;

  /**
   * \brief Retrieve the bapi flash driver that is associated with this flash partition.
   */
  inline const bapi_flash_api* bapi_flash_driver()const {
    const bapi_flash_api* drv =  bapi_flash_getDriver(m_partitionInfo.m_flashDeviceIndex);
    ASSERT(drv);
    return drv;
  }

  /**
   * \brief Retrieve the cookie that must be passed to a flash command function. This cookie
   *   is a pointer to this FlashPartition as a uint32_t data type.
   */
  inline uint32_t cookie()const {
    return R_CAST(uint32_t, &m_partitionInfo);
  }

public:
  /* Constructor */
  FlashPartition(enum bapi_E_FlashDevice flashDeviceIndex /**< The flash device of this partition. */
    , _flash_partition_size_t partitionIndex              /**< The partition index of this partition within the flash device. */
    , const _flash_partition_size_t* partitionSizes       /**< The size information of all partitions of the flash device. */
    ) : m_eventCallback(0), m_partitionInfo(flashDeviceIndex, partitionIndex, partitionSizes)  {
	  return;
  }

  /**
   * \brief Retrieve the physical flash device that this flash partition belongs to.
   */
  enum bapi_E_FlashDevice GetFlashDeviceIndex() {
    return m_partitionInfo.m_flashDeviceIndex;
  }



  /**
   * \brief Invoke the callback that was passed to the Initialize function for thie Flash partition.
   */
  inline void CallEventCallback(enum bapi_E_FlashDevice flashDeviceIndex, enum bapi_flash_E_Command_ID_ flashCommandID, int32_t armDriverState) {
    ASSERT(flashDeviceIndex == m_partitionInfo.m_flashDeviceIndex);
    m_flashStatus.error = (armDriverState != ARM_DRIVER_OK);

    if(m_eventCallback) {
      uint32_t event = armDriverState == ARM_DRIVER_OK ? ARM_FLASH_EVENT_READY : (ARM_FLASH_EVENT_ERROR | ARM_FLASH_EVENT_READY);
      m_eventCallback(flashDeviceIndex, flashCommandID, event);
    }
  }

  /**
   * \brief Implements function \ref struct _ARM_DRIVER_FLASH.GetCapabilities().
   */
  ARM_FLASH_CAPABILITIES GetCapabilities(void)const {
    return bapi_flash_driver()->GetCapabilities(m_partitionInfo.m_flashDeviceIndex);
  }

  /**
   * \brief Initialize the flash event callback.
   * This function Initialize the flash event callback -> not implemented!
   * \return status code.
   */
  int32_t Initialize(ARM_Flash_SignalEvent_t cb_event) {

    int32_t result = bapi_flash_initialize(m_partitionInfo.m_flashDeviceIndex);

    if(result == ARM_DRIVER_OK) {
      m_partitionInfo.ensureInitialized();


      if (cb_event) {
        if(GetCapabilities().event_ready) {

          /* The flash supports the event_ready signaling via callback.*/

          /* Hook ourself into the bapi layer. */
          bapi_flash_setUserCallback(m_partitionInfo.m_flashDeviceIndex, myBapiFlash_ISRCallback);

  #ifdef _DEBUG
          ARM_Flash_SignalEvent_t oldCallback =
  #endif
            atomic_PtrReplace(ARM_Flash_SignalEvent_t, &m_eventCallback, cb_event);

  #ifdef _DEBUG
          /* Assert that this Flash partition wasn't initialized already. Only
           * non-initialized Flash partitions are allowed to be initialized. */
          ASSERT(!oldCallback);
  #endif

        } else {

          /* The flash device does not support the event_ready capabilities.
           * So it cannot call a callback. Since a callback was passed,
           * the client has the expectation that we call the callback.
           * Since we cannot fulfill this expectation, we return with an
           * 'unsupported' status. */
          result = ARM_DRIVER_ERROR_UNSUPPORTED;
        }
      }
    }

    return result;
  }
  /**
   * \brief Uninitialize the flash event callback.
   * This function Uninitialize the flash event callback -> not implemented!
   * \return status code.
   */
  int32_t Uninitialize(void) {
    atomic_PtrReplace(ARM_Flash_SignalEvent_t, &m_eventCallback, S_CAST(ARM_Flash_SignalEvent_t, 0));
    return ARM_DRIVER_OK;
  }
  /**
   * \brief PowerControl the flash.
   * This function PowerControl the flash -> not implemented!
   * \return status code.
   */
  int32_t PowerControl(ARM_POWER_STATE state)const
    {
    return ARM_DRIVER_ERROR_UNSUPPORTED;
  }
  /**
   * \brief ReadData the flash.
   * This function ReadData from the flash!
   * \param  addr is relative the the partition
   * \data   pointer contains the buffer for the data
   * \cnt    number of bytes to read
   * \return status code.
   */
  int32_t ReadData(uint32_t addr, void *data, uint32_t cnt) {

    int32_t result = ARM_DRIVER_ERROR_PARAMETER;
    if ((addr + cnt) <= GetSize()) {
      const bapi_flash_api* drv = bapi_flash_driver();
      addr = addr + GetBaseAddress(); // calculate absolute flash address
      result = drv->ReadData(m_partitionInfo.m_flashDeviceIndex, addr, data, cnt, cookie());
    }

    if(result != ARM_DRIVER_OK) {
      CallEventCallback(m_partitionInfo.m_flashDeviceIndex, bapi_flash_CMDID_ReadData, result);
    }
    return result;
  }

  /**
   * \brief ProgramData the flash.
   * This function Programs Data to the flash!
   * \param  addr is relative to the partition
   * \data   pointer contains the buffer for the data to write
   * \cnt    number of bytes to write
   * \return status code.
   */
  int32_t ProgramData(uint32_t addr, const void *data, uint32_t cnt) {
    int32_t result = ARM_DRIVER_ERROR_PARAMETER;
    if ((addr + cnt) <= GetSize()) {
      const bapi_flash_api* drv = bapi_flash_driver();
      addr = addr + GetBaseAddress(); // calculate absolute flash address
      result = drv->ProgramData(m_partitionInfo.m_flashDeviceIndex, addr, data, cnt, cookie());
    }

    if(result != ARM_DRIVER_OK) {
      CallEventCallback(m_partitionInfo.m_flashDeviceIndex, bapi_flash_CMDID_ProgramData, result);
    }
    return result;
  }


  /**
   * \brief EraseSector of the partition.
   *    This function erases a single sector of this flash partition!
   * \param  addr is the sector address relative the the partition
   * \return status code.
   */
  int32_t EraseSector(uint32_t addr)
    {
    int32_t result = ARM_DRIVER_ERROR_PARAMETER;
    if (addr < GetSize()) {
      const bapi_flash_api* drv = bapi_flash_driver();
      addr = addr + GetBaseAddress(); // calculate absolute flash address
      result = drv->SectorErase(m_partitionInfo.m_flashDeviceIndex, addr, 1, cookie());
    }

    if(result != ARM_DRIVER_OK) {
      CallEventCallback(m_partitionInfo.m_flashDeviceIndex, bapi_flash_CMDID_EraseSector, result);
    }
    return result;
  }

  /**
   * \brief EraseSector of the partition.
   *    This function erases a single sector of this flash partition!
   * \param  addr is the sector address relative the the partition
   * \return status code.
   */
  int32_t EraseBlock(uint32_t addr)
    {
    int32_t result = ARM_DRIVER_ERROR_PARAMETER;
    if (addr < GetSize()) {
      const bapi_flash_api* drv = bapi_flash_driver();
      addr = addr + GetBaseAddress(); // calculate absolute flash address
      result = drv->BlockErase(m_partitionInfo.m_flashDeviceIndex, addr, 1, cookie());
    }

    if(result != ARM_DRIVER_OK) {
      CallEventCallback(m_partitionInfo.m_flashDeviceIndex, bapi_flash_CMDID_EraseBlock, result);
    }
    return result;
  }

  /**
   * \brief EraseChip the flash.
   * This function erases all sectors of this partition.
   * \return status code.
   */
  /* TODO: Support asynchronous internal flash chip erase. */
  int32_t EraseChip(void) {

    const struct bapi_flash_api* drv = bapi_flash_driver();
    int32_t result = drv->SectorErase(m_partitionInfo.m_flashDeviceIndex, GetBaseAddress(), m_partitionInfo.sector_count, cookie());

    if(result != ARM_DRIVER_OK) {
      CallEventCallback(m_partitionInfo.m_flashDeviceIndex, bapi_flash_CMDID_EraseSector, result);
    }

    return result;
  }

  /**
   * \brief Get sector lockdown info of the flash
   * \return status code.
   */
  int32_t ReadSectorLockdown(uint8_t* buffer, unsigned bufferSize) {
    const struct bapi_flash_api* drv = bapi_flash_driver();
    int32_t result = drv->ReadSectorLockdown(m_partitionInfo.m_flashDeviceIndex,
      GetBaseAddress(), buffer, bufferSize, cookie());

    if(result < ARM_DRIVER_OK) {
      CallEventCallback(m_partitionInfo.m_flashDeviceIndex, bapi_flash_CMDID_ReadSectorInfo, result);
    }

    return result;
  }

  /**
   * \brief Get sector protection info of the flash
   * \return status code.
   */
  int32_t ReadSectorProtection(uint8_t* buffer, unsigned bufferSize) {
    const struct bapi_flash_api* drv = bapi_flash_driver();
    int32_t result = drv->ReadSectorProtection(m_partitionInfo.m_flashDeviceIndex,
      GetBaseAddress(), buffer, bufferSize, cookie());

    if(result < ARM_DRIVER_OK) {
      CallEventCallback(m_partitionInfo.m_flashDeviceIndex, bapi_flash_CMDID_ReadSectorInfo, result);
    }

  }

  /**
   * \brief Enable protection of the flash partition.
   * \return status code.
   */
  int32_t EnableProtection(void) {

    const struct bapi_flash_api* drv = bapi_flash_driver();
    int32_t result = drv->EnableSectorProtection(m_partitionInfo.m_flashDeviceIndex, GetBaseAddress(), m_partitionInfo.sector_count, cookie());

    if(result != ARM_DRIVER_OK) {
      CallEventCallback(m_partitionInfo.m_flashDeviceIndex, bapi_flash_CMDID_EnableSectorProtect, result);
    }

    return result;
  }

  /**
   * \brief Enable protection of the flash partition.
   * \return status code.
   */
  int32_t FreezeSectorLockdown(void) {

    const struct bapi_flash_api* drv = bapi_flash_driver();
    int32_t result = drv->FreezeSectorLockdown(m_partitionInfo.m_flashDeviceIndex, cookie());

    if(result != ARM_DRIVER_OK) {
      CallEventCallback(m_partitionInfo.m_flashDeviceIndex, bapi_flash_CMDID_FreezeSectorLockdown, result);
    }

    return result;
  }


  /**
   * \brief Disable protection of the flash partition.
   * \return status code.
   */
  int32_t DisableProtection(void) {

    const struct bapi_flash_api* drv = bapi_flash_driver();
    int32_t result = drv->DisableSectorProtection(m_partitionInfo.m_flashDeviceIndex, GetBaseAddress(), m_partitionInfo.sector_count, cookie());

    if(result != ARM_DRIVER_OK) {

      if(result == ARM_DRIVER_ERROR_UNSUPPORTED) {
        /* If protection is not supported we assume that the sector is always unprotected. */
        result = ARM_DRIVER_OK;
      }

      CallEventCallback(m_partitionInfo.m_flashDeviceIndex, bapi_flash_CMDID_DisableSectorProtect, result);
    }

    return result;
  }


  /**
   * \brief GetStatus the flash.
   * This function returns the busy state and status of the last flash command on this partition!
   * \return @ARM_FLASH_STATUS.
   */
  ARM_FLASH_STATUS GetStatus(void) {
    const struct bapi_flash_api* driver = bapi_flash_getDriver(m_partitionInfo.m_flashDeviceIndex);

    bapi_irq_enterCritical();

    m_flashStatus.busy = driver->DriverBusy(m_partitionInfo.m_flashDeviceIndex);
    ARM_FLASH_STATUS retval = m_flashStatus;

    bapi_irq_exitCritical();
    return (retval);
  }

  /**
   * \brief GetInfo the flash.
   * This function returns the info about this partition
   * \return @ARM_FLASH_INFO.
   */
  ARM_FLASH_INFO *GetInfo(void) {
    return &m_partitionInfo;
  }

  /**
   * \brief GetInfo the flash.
   * This function returns the total size of this partition
   * \return @ARM_FLASH_INFO.
   */
  uint32_t GetSize()const {
    return bapi_flash_sizeOfSectors(&m_partitionInfo, 0, m_partitionInfo.sector_count);
  }

  uint32_t GetBaseAddress()const {
    return m_partitionInfo.m_flashDeviceAddressOffset;
  }
};

void myBapiFlash_ISRCallback(enum bapi_E_FlashDevice flashDeviceIndex,
  int32_t armDriverState, enum bapi_flash_E_Command_ID_ flashCommandID, uint32_t cookie) {

  FlashPartition* partition = R_CAST(FlashPartition*, cookie);
  partition->CallEventCallback(flashDeviceIndex, flashCommandID, armDriverState);

  return;
}

} /* namespace Driver_FLASH */


/**
 * \ingroup _cmsis_driver_flash
 *
 * A helper struct allowing to declare partitions for a physical flash device via macro
 *   \ref #FLASH_DEFINE_PARTITIONS.
 *
 *  This structure provides an array of sizes for each partition of a physical flash
 *  device. The last entry in this array is allowed to be zero, which has the meaning of
 *  _the remaining available memory of the flash device_
 *
 *  The non - specialized structure provides always a single entry array with a zero
 *    value. That means taking the whole flash device for a single partition.
 *
 *  Specializations of this structure will provide a different array with more partitions.
 *  Those specializations are defined by invoking the macro #FLASH_DEFINE_PARTITIONS.
 */
template<enum bapi_E_FlashDevice flashDeviceIndex> struct _FlashPartitionsDef {
  static const _flash_partition_size_t value[];
};

/*
 *  The partition sizes array for the non - specialized _FlashPartitionsDef structure.
 */
template<enum bapi_E_FlashDevice flashDeviceIndex>
  const _flash_partition_size_t _FlashPartitionsDef<flashDeviceIndex>::value[] = {
    0  /* Default implementation just takes the whole device as one
        * partition. A 0 as the last array item indicates the rest of
        * the flash device. If there is only a 0, the rest is the same
        * as the whole flash device. */
};

/**
 * \ingroup _cmsis_driver_flash
 *
 * A macro providing the size of the partition size array for a particular physical
 * flash device.
 */
#define _FLASH_PARTITION_COUNT(flashDeviceIndex) \
  ARRAY_SIZE(_FlashPartitionsDef<flashDeviceIndex>::value)


/* ENABLE_FLASH_PARTITIONING may be set to 0 or 1 by product_config.h */
#if ENABLE_FLASH_PARTITIONING > 0
  /* The following header file must be provided by the product in the product
   * configuration folder. */
  #define  FLASH_GET_PARTITIONS_LAYOUT 1
  #include "flash_partitions.h"
#endif /* #if ENABLE_FLASH_PARTITIONING > 0 */


/* Recursive structure to build the flash partition array for a particular flash device. */
template<enum bapi_E_FlashDevice flashDeviceIndex, unsigned partitionIndex> struct _FlashPartitionArray {
  enum {size = _FLASH_PARTITION_COUNT(flashDeviceIndex)}; /* The array size -> number of partitions. */
  _FlashPartitionArray<flashDeviceIndex, partitionIndex - 1> m_previous;
  Driver_FLASH::FlashPartition m_item;
  _FlashPartitionArray() : m_item(flashDeviceIndex, partitionIndex, _FlashPartitionsDef<flashDeviceIndex>::value) {
  }
};

/* Specialization for flashDeviceIndex = 0 */
template<enum bapi_E_FlashDevice flashDeviceIndex> struct _FlashPartitionArray<flashDeviceIndex, 0> {
  enum {size = 1}; /* The array size -> number of partitions. */
  Driver_FLASH::FlashPartition m_item;
  _FlashPartitionArray() : m_item(flashDeviceIndex, 0, _FlashPartitionsDef<flashDeviceIndex>::value) {
  }
};


#if (BAPI_HAS_FLASH_DEVICE > 0)
STATIC struct _FlashPartitionArray<bapi_E_FlashDev0, _FLASH_PARTITION_COUNT(bapi_E_FlashDev0)-1> s_flashPartitionArray0;
#endif
#if (BAPI_HAS_FLASH_DEVICE > 1)
STATIC struct _FlashPartitionArray<bapi_E_FlashDev1, _FLASH_PARTITION_COUNT(bapi_E_FlashDev1)-1> s_flashPartitionArray1;
#endif
#if (BAPI_HAS_FLASH_DEVICE > 2)
STATIC struct _FlashPartitionArray<bapi_E_FlashDev2, _FLASH_PARTITION_COUNT(bapi_E_FlashDev2)-1> s_flashPartitionArray2;
#endif
#if (BAPI_HAS_FLASH_DEVICE > 3)
STATIC struct _FlashPartitionArray<bapi_E_FlashDev3, _FLASH_PARTITION_COUNT(bapi_E_FlashDev3)-1> s_flashPartitionArray3;
#endif
#if (BAPI_HAS_FLASH_DEVICE > 4)
#error "Too many flash devices defined, please enhance to the scheme above"
#endif

STATIC Driver_FLASH::FlashPartition* const s_flashDevicePartitions[bapi_E_FlashDevCount] = {
   R_CAST(struct Driver_FLASH::FlashPartition*, &s_flashPartitionArray0)
#if (BAPI_HAS_FLASH_DEVICE > 1)
  ,R_CAST(struct Driver_FLASH::FlashPartition*, &s_flashPartitionArray1)
#endif
#if (BAPI_HAS_FLASH_DEVICE > 2)
  ,R_CAST(struct Driver_FLASH::FlashPartition*, &s_flashPartitionArray2)
#endif
#if (BAPI_HAS_FLASH_DEVICE > 3)
  ,R_CAST(struct Driver_FLASH::FlashPartition*, &s_flashPartitionArray3)
#endif
#if (BAPI_HAS_FLASH_DEVICE > 4)
#error "Too many flash devices defined, please enhance to the scheme above"
#endif
};


/**
 * \ingroup ARM_FLASH_GetVersion
 * \brief Get ARM driver version function
 */
STATIC ARM_DRIVER_VERSION ARM_FLASH_GetVersion(void)
{
  return Driver_FLASH::driverVersion;
}

/**
 * \ingroup _cmsis_driver_flash
 * \brief template class for FLASH API with flash device index and partition index (within the flash device context)
 */
template<enum bapi_E_FlashDevice flashDeviceIndex, unsigned flashPartitionIndex> struct ARM_FLASH
{
  static ARM_FLASH_CAPABILITIES GetCapabilities(void)
    {
    return s_flashDevicePartitions[flashDeviceIndex][flashPartitionIndex].GetCapabilities();
  }

  static int32_t Initialize(ARM_Flash_SignalEvent_t cb_event)
    {
    return s_flashDevicePartitions[flashDeviceIndex][flashPartitionIndex].Initialize(cb_event);
  }

  static int32_t Uninitialize(void)
    {
    return s_flashDevicePartitions[flashDeviceIndex][flashPartitionIndex].Uninitialize();
  }

  static int32_t PowerControl(ARM_POWER_STATE state)
    {
    return s_flashDevicePartitions[flashDeviceIndex][flashPartitionIndex].PowerControl(state);
  }

  static int32_t ReadData(uint32_t addr, void *data, uint32_t cnt)
    {
    return s_flashDevicePartitions[flashDeviceIndex][flashPartitionIndex].ReadData(addr, data, cnt);
  }

  static int32_t ProgramData(uint32_t addr, const void *data, uint32_t cnt)
    {
    return s_flashDevicePartitions[flashDeviceIndex][flashPartitionIndex].ProgramData(addr, data, cnt);
  }

  static int32_t EraseBlock(uint32_t addr)
    {
    return s_flashDevicePartitions[flashDeviceIndex][flashPartitionIndex].EraseBlock(addr);
  }

  static int32_t EraseSector(uint32_t addr)
    {
    return s_flashDevicePartitions[flashDeviceIndex][flashPartitionIndex].EraseSector(addr);
  }

  static int32_t EraseChip(void)
    {
    return s_flashDevicePartitions[flashDeviceIndex][flashPartitionIndex].EraseChip();
  }

  static ARM_FLASH_STATUS GetStatus(void)
    {
    return s_flashDevicePartitions[flashDeviceIndex][flashPartitionIndex].GetStatus();
  }

  static ARM_FLASH_INFO *GetInfo(void)
    {
    return s_flashDevicePartitions[flashDeviceIndex][flashPartitionIndex].GetInfo();
  }

  static enum bapi_E_FlashDevice GetFlashDeviceIndex(void) {
    return s_flashDevicePartitions[flashDeviceIndex][flashPartitionIndex].GetFlashDeviceIndex();
  }

  static uint32_t BaseAddress(void) {
    return s_flashDevicePartitions[flashDeviceIndex][flashPartitionIndex].GetBaseAddress();
  }

  static int32_t EnableProtection(void) {
    return s_flashDevicePartitions[flashDeviceIndex][flashPartitionIndex].EnableProtection();
  }

  static int32_t DisableProtection(void) {
    return s_flashDevicePartitions[flashDeviceIndex][flashPartitionIndex].DisableProtection();
  }

  static int32_t ReadSectorLockdown(uint8_t* buffer, unsigned bufferSize) {
    return s_flashDevicePartitions[flashDeviceIndex][flashPartitionIndex].ReadSectorLockdown(buffer, bufferSize);
  }

  static int32_t ReadSectorProtection(uint8_t* buffer, unsigned bufferSize) {
    return s_flashDevicePartitions[flashDeviceIndex][flashPartitionIndex].ReadSectorProtection(buffer, bufferSize);
  }

  static int32_t FreezeSectorLockdown() {
    return s_flashDevicePartitions[flashDeviceIndex][flashPartitionIndex].FreezeSectorLockdown();
  }
};


/**
 * \ingroup _cmsis_driver_flash
 * \brief The list of driver interface functions in the sequence as they appear in
 * struct _ARM_FLASH_DRIVER.
 */
#define _FLASH_DRIVER_VALUE(flashIndex, partitionIndex) \
     ARM_FLASH_GetVersion \
  ,  ARM_FLASH<flashIndex, partitionIndex>::GetCapabilities \
  ,  ARM_FLASH<flashIndex, partitionIndex>::Initialize \
  ,  ARM_FLASH<flashIndex, partitionIndex>::Uninitialize \
  ,  ARM_FLASH<flashIndex, partitionIndex>::PowerControl \
  ,  ARM_FLASH<flashIndex, partitionIndex>::ReadData \
  ,  ARM_FLASH<flashIndex, partitionIndex>::ProgramData \
  ,  ARM_FLASH<flashIndex, partitionIndex>::EraseBlock \
  ,  ARM_FLASH<flashIndex, partitionIndex>::EraseSector \
  ,  ARM_FLASH<flashIndex, partitionIndex>::EraseChip \
  ,  ARM_FLASH<flashIndex, partitionIndex>::GetStatus \
  ,  ARM_FLASH<flashIndex, partitionIndex>::GetInfo \
  ,  ARM_FLASH<flashIndex, partitionIndex>::GetFlashDeviceIndex \
  ,  ARM_FLASH<flashIndex, partitionIndex>::BaseAddress \
  ,  ARM_FLASH<flashIndex, partitionIndex>::EnableProtection \
  ,  ARM_FLASH<flashIndex, partitionIndex>::DisableProtection \
  ,  ARM_FLASH<flashIndex, partitionIndex>::ReadSectorLockdown \
  ,  ARM_FLASH<flashIndex, partitionIndex>::ReadSectorProtection \
  ,  ARM_FLASH<flashIndex, partitionIndex>::FreezeSectorLockdown



/* Recursive structure to build the flash partition array for a particular flash device. */
template<enum bapi_E_FlashDevice flashDeviceIndex, unsigned partitionIndex> struct _PartitionsFlashDrivers {

  enum {size = _FLASH_PARTITION_COUNT(flashDeviceIndex)}; /* The array size -> number of partitions. */
  static const struct _ARM_DRIVER_FLASH ms_item;

  _PartitionsFlashDrivers<flashDeviceIndex, partitionIndex - 1> m_previous;
  const struct _ARM_DRIVER_FLASH* const m_item;
  _PartitionsFlashDrivers() : m_item(&ms_item) {
  }
};

template<enum bapi_E_FlashDevice flashDeviceIndex, unsigned partitionIndex> const struct _ARM_DRIVER_FLASH
  _PartitionsFlashDrivers<flashDeviceIndex, partitionIndex>::ms_item = {
    _FLASH_DRIVER_VALUE(flashDeviceIndex, partitionIndex)
};

/* Specialization for flashDeviceIndex = 0 */
template<enum bapi_E_FlashDevice flashDeviceIndex> struct _PartitionsFlashDrivers<flashDeviceIndex, 0> {

  enum {size = _FLASH_PARTITION_COUNT(flashDeviceIndex)}; /* The array size -> number of partitions. */
  static const struct _ARM_DRIVER_FLASH ms_item;

  const struct _ARM_DRIVER_FLASH* const m_item;
  _PartitionsFlashDrivers() : m_item(&ms_item) {
  }
};

template<enum bapi_E_FlashDevice flashDeviceIndex> const struct _ARM_DRIVER_FLASH
  _PartitionsFlashDrivers<flashDeviceIndex, 0>::ms_item = {
    _FLASH_DRIVER_VALUE(flashDeviceIndex, 0)
};

#if (BAPI_HAS_FLASH_DEVICE > 0)
STATIC const struct _PartitionsFlashDrivers<bapi_E_FlashDev0, _FLASH_PARTITION_COUNT(bapi_E_FlashDev0)-1> s_partitionFlashDrivers0;
#endif
#if (BAPI_HAS_FLASH_DEVICE > 1)
STATIC const struct _PartitionsFlashDrivers<bapi_E_FlashDev1, _FLASH_PARTITION_COUNT(bapi_E_FlashDev1)-1> s_partitionFlashDrivers1;
#endif
#if (BAPI_HAS_FLASH_DEVICE > 2)
STATIC const struct _PartitionsFlashDrivers<bapi_E_FlashDev2, _FLASH_PARTITION_COUNT(bapi_E_FlashDev2)-1> s_partitionFlashDrivers2;
#endif
#if (BAPI_HAS_FLASH_DEVICE > 3)
STATIC const struct _PartitionsFlashDrivers<bapi_E_FlashDev3, _FLASH_PARTITION_COUNT(bapi_E_FlashDev3)-1> s_partitionFlashDrivers3;
#endif
#if (BAPI_HAS_FLASH_DEVICE > 4)
#error "Too many flash devices defined, please enhance to the scheme above"
#endif

struct _FlashDriverArray {
  const struct _ARM_DRIVER_FLASH* const* m_driver;
  const unsigned m_numDrivers;
};

static const struct _FlashDriverArray s_devicesFlashDrivers[] = {
#if (BAPI_HAS_FLASH_DEVICE > 0)
   { (&s_partitionFlashDrivers0.m_item) - (s_partitionFlashDrivers0.size-1), s_partitionFlashDrivers0.size }
#endif
#if (BAPI_HAS_FLASH_DEVICE > 1)
  ,{ (&s_partitionFlashDrivers1.m_item) - (s_partitionFlashDrivers1.size-1), s_partitionFlashDrivers1.size }
#endif
#if (BAPI_HAS_FLASH_DEVICE > 2)
  ,{ (&s_partitionFlashDrivers2.m_item) - (s_partitionFlashDrivers2.size-1), s_partitionFlashDrivers2.size }
#endif
#if (BAPI_HAS_FLASH_DEVICE > 3)
  ,{ (&s_partitionFlashDrivers3.m_item) - (s_partitionFlashDrivers3.size-1), s_partitionFlashDrivers3.size }
#endif
#if (BAPI_HAS_FLASH_DEVICE > 4)
#error "Too many flash devices defined, please enhance to the scheme above"
#endif
};

const struct _ARM_DRIVER_FLASH* driver_flash_getDriver(enum bapi_E_FlashDevice flashDeviceIndex, unsigned partitionIndex) {
  if(flashDeviceIndex < bapi_E_FlashDevCount) {
    if(partitionIndex < s_devicesFlashDrivers[flashDeviceIndex].m_numDrivers ) {
      return s_devicesFlashDrivers[flashDeviceIndex].m_driver[partitionIndex];
    }
  }

  return 0;
}

unsigned driver_flash_getPartitionCount(enum bapi_E_FlashDevice flashDeviceIndex) {
  if(flashDeviceIndex < bapi_E_FlashDevCount) {
    return s_devicesFlashDrivers[flashDeviceIndex].m_numDrivers;
  }
  return 0;
}



#endif /* (BAPI_HAS_FLASH_PARTITIONS > 0) */
