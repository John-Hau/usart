/*
 *  $HeadURL: $
 *
 *  $Date: $
 *  $Author: $
 */

#ifndef bapi_mcu_clock_H_
#define bapi_mcu_clock_H_
/**
 * \file
 * \brief
 * This file declares the clock frequency of the MCU
 * */

#include "hardware-board.h"

#if defined (EM_DK3750)
	#include "boards/vendors/MCU_VENDOR_SILABS/bapi_mcu_clock_MCU_VENDOR_SILABS.h"
#elif defined (FS_IRMFCU)
  #include "boards/vendors/MCU_VENDOR_FREESCALE/bapi_mcu_clock_MCU_VENDOR_FREESCALE.h"
#elif defined (FS_IRMLC)
  #include "boards/vendors/MCU_VENDOR_FREESCALE/bapi_mcu_clock_MCU_VENDOR_FREESCALE.h"
#elif defined (FS_IRMCT)
  #include "boards/vendors/MCU_VENDOR_FREESCALE/bapi_mcu_clock_MCU_VENDOR_FREESCALE.h"
#elif defined (FS_IRM_BIO)
  #include "boards/vendors/MCU_VENDOR_FREESCALE/bapi_mcu_clock_MCU_VENDOR_FREESCALE.h"
#elif defined (FS_FRDM_KL46Z)
  #include "boards/vendors/MCU_VENDOR_FREESCALE/bapi_mcu_clock_MCU_VENDOR_FREESCALE.h"
#elif defined (FS_FRDM_K64F)
  #include "boards/vendors/MCU_VENDOR_FREESCALE/bapi_mcu_clock_MCU_VENDOR_FREESCALE.h"
#elif defined (FS_FRDM_K66F)
  #include "boards/vendors/MCU_VENDOR_FREESCALE/bapi_mcu_clock_MCU_VENDOR_FREESCALE.h"
#elif defined (FS_IRMCT)
  #include "boards/vendors/MCU_VENDOR_FREESCALE/bapi_mcu_clock_MCU_VENDOR_FREESCALE.h"    
#elif defined (FS_IRMLC_KL17Z)
  #include "boards/vendors/MCU_VENDOR_FREESCALE/bapi_mcu_clock_MCU_VENDOR_FREESCALE.h"
#elif defined (CCS_CVAHU)
  #include "boards/vendors/MCU_VENDOR_FREESCALE/bapi_mcu_clock_MCU_VENDOR_FREESCALE.h"
#elif defined (FS_IRMFCU_BL)
  #include "boards/vendors/MCU_VENDOR_FREESCALE/bapi_mcu_clock_MCU_VENDOR_FREESCALE.h"
#elif defined (FS_IRMVAV)
  #include "boards/vendors/MCU_VENDOR_FREESCALE/bapi_mcu_clock_MCU_VENDOR_FREESCALE.h"
#elif defined (FS_IMXRTEVAL)
  #include "boards/vendors/MCU_VENDOR_NXP/bapi_mcu_clock_MCU_VENDOR_NXP.h"
#elif defined (FS_BEATS_IO)
  #include "boards/vendors/MCU_VENDOR_NXP/bapi_mcu_clock_MCU_VENDOR_NXP.h"
#elif defined (FS_IMXRT_TSTAT)
  #include "boards/vendors/MCU_VENDOR_NXP/bapi_mcu_clock_MCU_VENDOR_NXP.h"
#elif defined (FS_IPVAV)
  #include "boards/vendors/MCU_VENDOR_NXP/bapi_mcu_clock_MCU_VENDOR_NXP.h"
#elif defined (FS_SNAP_ON_IO)
  #include "boards/vendors/MCU_VENDOR_NXP/bapi_mcu_clock_MCU_VENDOR_NXP.h"
#else
	#error "Fatal Error: Unknown hardware board."
#endif


#endif // #ifndef bapi_mcu_clock_H_
