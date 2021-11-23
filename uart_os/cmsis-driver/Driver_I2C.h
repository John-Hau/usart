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
 * $Date:        9. May 2014
 * $Revision:    V2.02
 *
 * Project:      I2C (Inter-Integrated Circuit) Driver definitions
 * -------------------------------------------------------------------------- */

/* History:
 *  Version 2.02
 *    Removed function ARM_I2C_MasterTransfer in order to simplify drivers
 *      and added back parameter "xfer_pending" to functions
 *      ARM_I2C_MasterTransmit and ARM_I2C_MasterReceive
 *  Version 2.01
 *    Added function ARM_I2C_MasterTransfer and removed parameter "xfer_pending"
 *      from functions ARM_I2C_MasterTransmit and ARM_I2C_MasterReceive
 *    Added function ARM_I2C_GetDataCount
 *    Removed flag "address_nack" from ARM_I2C_STATUS
 *    Replaced events ARM_I2C_EVENT_MASTER_DONE and ARM_I2C_EVENT_SLAVE_DONE
 *      with event ARM_I2C_EVENT_TRANSFER_DONE
 *    Added event ARM_I2C_EVENT_TRANSFER_INCOMPLETE
 *    Removed parameter "arg" from function ARM_I2C_SignalEvent
 *  Version 2.00
 *    New simplified driver:
 *      complexity moved to upper layer (especially data handling)
 *      more unified API for different communication interfaces
 *    Added:
 *      Slave Mode
 *    Changed prefix ARM_DRV -> ARM_DRIVER
 *  Version 1.10
 *    Namespace prefix ARM_ added
 *  Version 1.00
 *    Initial release
 */

#ifndef __CMSIS_DRIVER_I2C_H
#define __CMSIS_DRIVER_I2C_H

#include "baseplate.h"
#include "boards/board-api/bapi_i2c.h"
#include "boards/cmsis/Driver_I2C.h"



/**
 * \ingroup cmsis_driver_i2c
 * \brief
 * Obtain the CMSIS_ARM_DRIVER_I2C access structure for a particular I2C
 * by I2C index.
 *
 * The CMSIS Driver specifications defines just fixed names for the I2C
 * driver instances. This does badly support access to drivers at runtime
 * via I2C index. This supplementary function closes this gap.
 *
 * \return Pointer to the I2C Driver structure.
 */

//C_FUNC ARM_DRIVER_I2C* driver_i2c_getDriver(bapi_E_I2cIndex_ I2cIndex /**< [in] The I2C index for which to obtain the driver. */);
  
//typedef void (*ARM_I2C_SignalEvent_t) (enum bapi_E_I2cIndex_ I2cIndex,uint32_t event);  ///< Pointer to \ref ARM_I2C_SignalEvent : Signal I2C Event.

void ARM_I2C_SignalEvent(enum bapi_E_I2cIndex_ I2cIndex, uint32_t event);

/**
\brief Access structure of the I2C Driver.
*/
struct _ARM_DRIVER_I2C {
  ARM_DRIVER_VERSION   (*GetVersion)     (void);                                                                ///< Pointer to \ref ARM_I2C_GetVersion : Get driver version.
  ARM_I2C_CAPABILITIES (*GetCapabilities)(void);                                                                ///< Pointer to \ref ARM_I2C_GetCapabilities : Get driver capabilities.
  int32_t              (*Initialize)     (ARM_I2C_SignalEvent_t cb_event);                                      ///< Pointer to \ref ARM_I2C_Initialize : Initialize I2C Interface.
  int32_t              (*Uninitialize)   (void);                                                                ///< Pointer to \ref ARM_I2C_Uninitialize : De-initialize I2C Interface.
  int32_t              (*PowerControl)   (ARM_POWER_STATE state);                                               ///< Pointer to \ref ARM_I2C_PowerControl : Control I2C Interface Power.
  int32_t              (*MasterTransmit) (uint32_t addr, uint8_t *data, uint32_t num, bool xfer_pending); ///< Pointer to \ref ARM_I2C_MasterTransmit : Start transmitting data as I2C Master.
  int32_t              (*MasterReceive)  (uint32_t addr, uint8_t *data, uint32_t num, bool xfer_pending); ///< Pointer to \ref ARM_I2C_MasterReceive : Start receiving data as I2C Master.
  int32_t              (*SlaveTransmit)  (               uint8_t *data, uint32_t num);                    ///< Pointer to \ref ARM_I2C_SlaveTransmit : Start transmitting data as I2C Slave.
  int32_t              (*SlaveReceive)   (               uint8_t *data, uint32_t num);                    ///< Pointer to \ref ARM_I2C_SlaveReceive : Start receiving data as I2C Slave.
  int32_t              (*GetDataCount)   (void);                                                                ///< Pointer to \ref ARM_I2C_GetDataCount : Get transferred data count.
  int32_t              (*Control)        (uint32_t control, uint32_t arg);                                      ///< Pointer to \ref ARM_I2C_Control : Control I2C Interface.
  ARM_I2C_STATUS       (*GetStatus)      (void);                                                                ///< Pointer to \ref ARM_I2C_GetStatus : Get I2C status.
  //void                 (*SignalEvent)    (uint32_t event);                                                      ///< Pointer to \ref ARM_I2C_SignalEvent : This is callback function registered by Initialize function.
};

#if defined (FS_BEATS_IO)
//#if (BAPI_HAS_I2C > 0)
//  #define Driver_I2C0 (*driver_I2C_getDriver(bapi_E_I2c0))
//#endif
#else 
#if (BAPI_HAS_I2C > 0)
  #define Driver_I2C0 (*driver_I2C_getDriver(bapi_E_I2c0))
#endif
#endif 

///** CMSIS I2C 1 Driver Instance Name. */
//#if (BAPI_HAS_I2C > 1)
//  #define Driver_I2C1 (*driver_I2C_getDriver(bapi_E_I2c1))
//#endif
///** CMSIS I2C 2 Driver Instance Name. */
//#if (BAPI_HAS_I2C > 2)
//  #define Driver_I2C2 (*driver_I2C_getDriver(bapi_E_I2c2))
//#endif

typedef const struct _ARM_DRIVER_I2C ARM_DRIVER_I2C;


/**
 * \ingroup cmsis_driver_i2c
 * \brief
 * Obtain the CMSIS _ARM_DRIVER_I2C access structure for a particular I2C
 * by I2C index.
 *
 * The CMSIS Driver specifications defines just fixed names for the I2C
 * driver instances. This does badly support access to drivers at runtime
 * via I2C index. This supplementary function closes this gap.
 *
 * @return Pointer to the I2C Driver structure.
 */
C_FUNC ARM_DRIVER_I2C* driver_i2c_getDriver(
  bapi_E_I2cIndex i2cIndex  /**< [in] The I2C index for which to obtain the driver. */
  );


/**@}*/


#endif /* __CMSIS_DRIVER_I2C_H */
