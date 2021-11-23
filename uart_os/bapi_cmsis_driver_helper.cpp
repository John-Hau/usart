/*
 * bapi_irq.h
 *
 *  Created on: 07.04.2013
 *      Author: Wolfgang
 */


/**
 *
 * \file bapi_cmsis_driver_helper.cpp
 * \brief
 * This file declares and implements functions to support CMSIS drivers on bapi layer.
 * */
#include "baseplate.h"
#include "Driver_Flash.h"

#include <limits>


bool ARM_Flash_hasUniformSectors(const ARM_FLASH_INFO *flashInfo) {
  return flashInfo->sector_info == nullptr;
}

static const struct _ARM_FLASH_SECTOR * _getSectorAt(const ARM_FLASH_INFO *flashInfo, uint32_t index) {

  if((flashInfo->sector_info != NULL) && flashInfo->sector_info_count) {

    if(index > flashInfo->sector_count) {
      /* We are beyond the last flash sector. So return the error code. */
      return nullptr;
    }

    /* At this point we are sure to have at least one sector info */
    if(index < flashInfo->sector_info_count) {
      return &flashInfo->sector_info[index];
    }

    /*
     * Return the last sector info which is equivalent for subsequent sectors
     * in terms of size and block_size_in_pages.
     */
    return &flashInfo->sector_info[flashInfo->sector_info_count-1];

  }

  /* Somebody asked for a non existing sector info. This is not allowed.
   * First check for sector info existence via:
   *
   * if( ARM_Flash_hasUniformSectors(..) )
   *
   * ...and use uniform sector_size if ARM_Flash_hasUniformSectors(..)
   * returns true.
   */
  ASSERT(false);
  return nullptr;
}

uint32_t ARM_Flash_getSectorSizeAt(const ARM_FLASH_INFO *flashInfo, uint32_t index) {
  const ARM_FLASH_SECTOR * sectorInfo = _getSectorAt(flashInfo, index);
  const uint32_t retval = sectorInfo->sectorSize();
  return retval;
}

uint8_t ARM_Flash_getSectorBlockSizeInPagesAt(const ARM_FLASH_INFO *flashInfo, uint32_t index) {
  const ARM_FLASH_SECTOR * sectorInfo = _getSectorAt(flashInfo, index);
  return sectorInfo->block_size_in_pages;
}


uint32_t ARM_Flash_getSectorInfoAt(struct _ARM_FLASH_SECTOR *result, const struct _ARM_FLASH_INFO *flashInfo
  , uint32_t index) {

  if((flashInfo->sector_info != NULL) && flashInfo->sector_info_count) {

    if(index > flashInfo->sector_count) {
      /* We are beyond the last flash sector. So return the error code. */
      return std::numeric_limits<uint32_t>::max();
    }

    /* At this point we are sure to have at least one sector info */
    if(index < flashInfo->sector_info_count) {
      *result = flashInfo->sector_info[index];
      return index;
    }

    /* Create the result from the last sector info that we have. */
    *result = flashInfo->sector_info[flashInfo->sector_info_count-1];

    /* remainingIndexes is the number of indexes that our requested index
     * is higher than the maximum available index
     * (which is flashInfo->sector_info_count-1).
     */
    const uint32_t remainingIndexes = index - (flashInfo->sector_info_count-1);

    /* All following sectors have the same size as the last one. Hence
     * we pick this size ...*/
    const uint32_t lastSectorSize = result->sectorSize();

    /* ...and add it indexOffset times to the result's start and end info. */
    result->start += remainingIndexes * lastSectorSize;
    result->end += remainingIndexes * lastSectorSize;

    /* Return the sector index that we have used to generate the result. */
    return flashInfo->sector_info_count-1;
  }

  /* Somebody asked for a non existing sector info. This is not allowed.
   * First check for sector info existence via:
   *
   * if( ARM_Flash_hasUniformSectors(..) )
   *
   * ...and use uniform sector_size if ARM_Flash_hasUniformSectors(..)
   * returns true.
   */
  ASSERT(false);
  return std::numeric_limits<uint32_t>::max();
}
