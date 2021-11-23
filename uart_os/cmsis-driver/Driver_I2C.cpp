/*
 *  $HeadURL: $
 *
 *  $Date: $ 01-Sept-2017
 *  $Author: $e876372
 */


/**
 * /file
 * /brief This file Implements the cmsis-driver API for I2C and implements the functions
 * that provide instances of ARM_DRIVER_I2C structures for each I2C that is
 * defined by the board API (refer to bapi_i2c.h).
 */

#include "baseplate.h"

#include "boards/board-api/bapi_i2c.h"
#include "cmsis-driver/Driver_I2C.h"
#include "fsl_i2c.h"
//#include "fsl_i2c_master_driver.h" //  We will use only I2c master in all the boards supported by BSP
#include "fsl_i2c_edma.h"


#define ARM_I2C_DRV_VERSION    ARM_DRIVER_VERSION_MAJOR_MINOR(2, 0)  /* driver version */


namespace Driver_I2C {

/**
 * \ingroup _cmsis_driver_i2c
 *  Driver Version
 */
  STATIC const ARM_DRIVER_VERSION driverVersion = {
      ARM_I2C_API_VERSION,/*CMSIS API version*/
      ARM_I2C_DRV_VERSION
  };
};


/**
 * \ingroup _cmsis_driver_i2c
 * \brief namespace for ARM I2C driver corresponding interface functions
 * with an additional I2C index parameter.
 * This namespace is similar to access struct of CMSIS
 */
namespace _driver_ARM_I2C {

/**
 * \name _driver_ARM_I2C Functions.
 * ARM I2C driver corresponding interface functions with an additional
 * I2C index parameter.
 */
/**@{*/

/**
 * \ingroup _cmsis_driver_i2c
 * \brief Implements the functionality of CMSIS Driver API function
 * ARM_I2C::Control(uint32_t control, uint32_t arg)
 * ,but with an additional parameter <em>I2CIndex</em>, in order to decide on
 * which I2C the function should be carried out.
 * \warning This function must not be called from within an ISR context.
 */
STATIC int32_t Control(const enum bapi_E_I2cIndex_ I2CIndex, uint32_t control, uint32_t arg) {
	//ASSERT(!bapi_isIsrRunning())  // Shiva.
	ASSERT_DEBUG(!bapi_irq_isInterruptContext()) // nishant
	int32_t retval = ARM_DRIVER_ERROR_UNSUPPORTED;
  uint32_t i2c_Speed_Hz=0;
	switch(control & ARM_I2C_CONTROL_Msk)
	{

	case ARM_I2C_OWN_ADDRESS:
		// TBD : arg is the slave address which should be allocated as slave address of I2C interface pointed by I2CIndex
		// we may not use this as we only use MASTER mode of I2C in this microcontroller
	  retval = ARM_DRIVER_ERROR_UNSUPPORTED;
		break;

	case ARM_I2C_BUS_SPEED:
	  switch (arg)
	  {
	  case ARM_I2C_BUS_SPEED_STANDARD:
	    i2c_Speed_Hz = 100000;
	    break;
	  case ARM_I2C_BUS_SPEED_FAST:
	    i2c_Speed_Hz = 400000;
	    break;
	  case ARM_I2C_BUS_SPEED_FAST_PLUS:
	    i2c_Speed_Hz = 1000000;
	    break;
	  case ARM_I2C_BUS_SPEED_HIGH:
	    i2c_Speed_Hz = 3400000;
	    break;
	  default:
	    return ARM_DRIVER_ERROR_UNSUPPORTED;
	  }
    _bapi_I2cSetSpeed(I2CIndex,i2c_Speed_Hz); // call bapi layer to configure I2C port
		retval = ARM_DRIVER_OK;
		break;

	case ARM_I2C_BUS_CLEAR:
		// TBD : send 9 clock pulses on BUS
		retval = ARM_DRIVER_OK;
		break;

	case ARM_I2C_ABORT_TRANSFER:
    _bapi_I2cAbortTransfer(I2CIndex);

		retval = ARM_DRIVER_OK;
		break;
	}

  return retval; 
}


/**
 * \ingroup _cmsis_driver_i2c
 * \brief Implements the functionality of CMSIS Driver API function
 * ARM_I2C::Initialize(ARM_I2C_SignalEvent_t)
 * ,but with an additional parameter <em>I2CIndex</em>, in order to decide on
 * which I2C the function should be carried out.
 */
STATIC int32_t Initialize(const enum bapi_E_I2cIndex_ I2CIndex, ARM_I2C_SignalEvent_t cb_event) {
 // bapi_irq_enterCritical();

  int32_t retval = ARM_DRIVER_OK;
  _bapi_I2cInitCallback(I2CIndex,cb_event);

 // TBD
  
  /* Driver is already initialized and must be uninitialized first. */
 // bapi_irq_exitCritical();

  return retval;
}

/**
 * \ingroup _cmsis_driver_i2c
 * \brief Implements the functionality of CMSIS Driver API function
 * ARM_I2C::Uninitialize()
 * ,but with an additional parameter <em>I2CIndex</em>, in order to decide on
 * which I2C the function should be carried out.
 */
STATIC int32_t Uninitialize(const enum bapi_E_I2cIndex_ I2CIndex) {
	int32_t abortResult = ARM_DRIVER_ERROR;
 // TODO: implement
  return abortResult;
}


/**
 * \ingroup _cmsis_driver_i2c
 * \brief Implements the functionality of CMSIS Driver API function
 * ARM_I2C::Send(const void *, uint32_t)
 * ,but with an additional parameter <em>I2CIndex</em>, in order to decide on
 * which I2C the function should be carried out.
 */
STATIC int32_t MasterTransmit(const enum bapi_E_I2cIndex_ I2CIndex, uint32_t addr, uint8_t *data, uint32_t num, bool xfer_pending)
{
  int32_t retval;
  if(!data || !num) {
    return ARM_DRIVER_ERROR_PARAMETER;
  }
  bapi_irq_enterCritical();
  retval = (_bapi_I2cWriteData(I2CIndex,addr,data,num,xfer_pending));
  bapi_irq_exitCritical();
  return retval;
}

/**
 * \ingroup _cmsis_driver_i2c
 * \brief Implements the functionality of CMSIS Driver API function
 * ARM_I2C::MasterReceive(uint32_t,void *, uint32_t,bool)
 * ,but with an additional parameter <em>I2CIndex</em>, in order to decide on
 * which I2C the function should be carried out.
 */
STATIC int32_t MasterReceive(const enum bapi_E_I2cIndex_ I2CIndex, uint32_t addr, uint8_t *data, uint32_t num, bool xfer_pending)
{
  int32_t retval;
  if(!data || !num) {
    return ARM_DRIVER_ERROR_PARAMETER;
  }

  retval = ARM_DRIVER_ERROR_BUSY;

  bapi_irq_enterCritical();
  retval = (_bapi_I2cReadData(I2CIndex,addr,data,num,xfer_pending));
  bapi_irq_exitCritical();

  return retval;
}

/**
 * \ingroup _cmsis_driver_i2c
 * \brief Implements the functionality of CMSIS Driver API function
 * ARM_I2C::SlaveTransmit(void *, uint32_t)
 * @param I2CIndex
 * @param data
 * @param num
 * @return
 */
STATIC int32_t SlaveTransmit(const enum bapi_E_I2cIndex_ I2CIndex, uint8_t *data, uint32_t num)
{
  return (ARM_DRIVER_ERROR_UNSUPPORTED);
}

/**
 * \ingroup _cmsis_driver_i2c
 * \brief Implements the functionality of CMSIS Driver API function
 * ARM_I2C::SlaveReceive(void *, uint32_t)
 * @param I2CIndex
 * @param data Pointer to buffer with data to transmit to I2C Master
 * @param num Number of data bytes to transmit
 * @return Status Error Codes
 */
STATIC int32_t SlaveReceive(const enum bapi_E_I2cIndex_ I2CIndex, uint8_t *data, uint32_t num)
{
  return (ARM_DRIVER_ERROR_UNSUPPORTED);
}



/**
 * \ingroup _cmsis_driver_i2c
 * \brief Implements the functionality of CMSIS Driver API function
 * ARM_I2C::GetStatus(void)
 * ,but with an additional parameter <em>I2CIndex</em>, in order to decide on
 * which I2C the function should be carried out.
 */
STATIC ARM_I2C_STATUS GetStatus(const enum bapi_E_I2cIndex_ I2CIndex) {
  ARM_I2C_STATUS status = {0};

  // TODO: support all status flags.
  return status;
}

/**
 * \ingroup _cmsis_driver_i2c
 * \brief Implements the functionality of CMSIS Driver API function
 * ARM_I2C::GetDataCount(void)
 * ,but with an additional parameter <em>I2CIndex</em>, in order to decide on
 * which I2C the function should be carried out.
 */
STATIC int32_t GetDataCount(const enum bapi_E_I2cIndex_ I2CIndex)
{
  return ARM_DRIVER_ERROR_UNSUPPORTED;
}

/**
 * \ingroup _cmsis_driver_i2c
 * \brief Implements the functionality of CMSIS Driver API function
 * ARM_I2C::PowerControl(ARM_POWER_STATE state)
 * ,but with an additional parameter <em>I2CIndex</em>, in order to decide on
 * which I2C the function should be carried out.
 */
STATIC int32_t PowerControl(const enum bapi_E_I2cIndex_ I2CIndex, ARM_POWER_STATE state)
{
  int32_t retval = ARM_DRIVER_ERROR_UNSUPPORTED;
    switch (state)
    {
    case ARM_POWER_OFF:
      _bapi_I2cDisable(I2CIndex);
      retval =  ARM_DRIVER_OK;
        break;
    case ARM_POWER_LOW:
        break;
    case ARM_POWER_FULL:
      _bapi_I2cEnable(I2CIndex); /*enable : I2C peripheral and I2C clock*/
      retval =  ARM_DRIVER_OK;
        break;
    default:
      retval =  ARM_DRIVER_ERROR_UNSUPPORTED;
    }
    return retval;
}


/**@} _driver_ARM_I2C*/

} /* namespace _driver_ARM_I2C */




/******************************************************************************
 * ARM_DRIVER_I2C instances
 *****************************************************************************/
/**
 * \ingroup cmsis_driver_i2c
 * \brief Get driver version.
 *
 * The function initializes the I2C interface. It is called when the
 * middleware component starts operation.
 *
 * Returns version information of the driver implementation in ARM_DRIVER_VERSION
 *   - API version is the version of the CMSIS-Driver specification used to implement this driver.
 *   - Driver version is source code version of the actual driver implementation.
 *
 * \return The API Version and Driver Version in an _ARM_DRIVER_VERSION structure.
 *
 Example:
 ~~~~~~~~{.c}
 extern ARM_DRIVER_I2C Driver_I2C0;
 ARM_DRIVER_I2C *drv_info;

 void setup_i2c (void) {
   ARM_DRIVER_VERSION  version;

   drv_info = &Driver_I2C0;
   version = drv_info->GetVersion ();
   if (version.api < 0x10A) {      // requires at minimum API version 1.10 or higher
     // error handling
     return;
   }
 }
 ~~~~~~~~
 */
STATIC ARM_DRIVER_VERSION ARM_I2C_GetVersion(void)
{
  return Driver_I2C::driverVersion;
}


/**
 * \ingroup cmsis_driver_i2c
 * \brief Implementation of the I2C driver interface functions for all I2CS.
 *
 * Refer to \ref cmsis_driver_i2c_instance_section and struct i2cArray__
 *
 */
template<enum bapi_E_I2cIndex_ I2cIndex> class ARM_I2C {
public:

  /* -------- hook-able functions -------- */

  /**
   * \ingroup cmsis_driver_i2c
   * \brief Initialize I2C Interface.
   *
   * \param [in] cb_event Pointer to a callback function with the signature of
   * \ref ARM_I2C_SignalEvent
   *
   * \return #ARM_DRIVER_OK upon success. #ARM_DRIVER_ERROR, if the driver
   * is already initialized with a different cb_event.
   *
   * The function initializes the I2C interface. It is called when the
   * middleware component starts operation.
   *
   * The function performs the following operations:
   *  - Initializes the resources needed for the I2C interface.
   *  - Registers the ARM_I2C_SignalEvent callback function.
   *
   * The parameter cb_event is a pointer to the callback function with the
   * signature of \ref ARM_I2C_SignalEvent; use a NULL pointer when no
   * callback signals are required.
   *
   */
  static int32_t Initialize(ARM_I2C_SignalEvent_t cb_event)
  {
    return _driver_ARM_I2C::Initialize(I2cIndex, cb_event);
  }

  /**
   * \ingroup cmsis_driver_i2c
   * \brief De-initialize I2C Interface.
   * \return Common \ref cmsis_driver_general_return_codes "Status Error Codes"
   *
   * The function ARM_I2C::Uninitialize de-initializes the resources of
   * I2C interface. It is called when the middleware component stops
   * operation and releases the software resources used by the interface.
   */
  static int32_t Uninitialize()
  {
    return _driver_ARM_I2C::Uninitialize(I2cIndex);
  }

  /**
   * \ingroup cmsis_driver_i2c
   * \brief Control I2C Interface.
   * \param control [in] Operation
   * \param arg [in] Argument of Operation
   *
   * \return Common \ref cmsis_driver_general_return_codes "Status Error Codes"
   * and driver specific \ref cmsis_driver_i2c_return_codes "Status Error Codes"
   *
   * Control the I2C interface settings and execute various operations.
   * The parameter _control_ is a bit mask that specifies various operations
   * (see tables below). The control bits of the various groups can be combined
   * to a control code as shown in the following example. Depending on the
   * control bits, the parameter _arg_ provides additional information
   *
   Example:
   ~~~~~~~~{.c}
   extern ARM_DRIVER_I2C Driver_I2C0;
   // configure to I2C mode: I2C speed 400kHz
   status = Driver_I2C0.Control(ARM_I2C_BUS_SPEED \
                               ,ARM_I2C_BUS_SPEED_FAST);
      // Disable slave mode of I2C
   status = Driver_I2C0.Control(ARM_I2C_OWN_ADDRESS, 0); // setting own slave address 0 disables slave mode
   ~~~~~~~~
   *
   * __ARM_I2C_MODE_xxx__ control Byte specify the I2C mode:
   *
   * Mode Control Byte                | Description
   * ---------------------------------|------------------------------------------------------------------------------------
   * #ARM_I2C_OWN_ADDRESS             | Set Own Slave Address; arg = slave address
   * #ARM_I2C_BUS_SPEED               | Set Bus Speed; arg = bus speed
   * #ARM_I2C_BUS_CLEAR               | Clear the bus by sending nine clock pulses
   * #ARM_I2C_ABORT_TRANSFER          | Aborts the data transfer between Master and Slave for Transmit or Receive
   *
   * The Mode Control Byte can be combined with Mode Control Parameters to specify operation
   *
   * For Mode Control Byte (ARM_I2C_OWN_ADDRESS) :
   * Control Parameter (arg)          | Description
   * ---------------------------------|------------------------------------------------------------------------------------
   * ARM_I2C_ADDRESS_10BIT            | 10-bit address
   * ARM_I2C_ADDRESS_GC               | slave accepts a General Call.
   * 0                                | slave address is 0 (SLAVE Mode DISABLED)
   *
   * For Mode Control Byte  (ARM_I2C_BUS_SPEED) :
   * Control Parameter (arg)          | Description
   * ---------------------------------|------------------------------------------------------------------------------------
   * ARM_I2C_BUS_SPEED_STANDARD       | Standard Speed to (100 kHz)
   * ARM_I2C_BUS_SPEED_FAST           | Fast Speed (400kHz)
   * ARM_I2C_BUS_SPEED_FAST_PLUS      | Fast + Speed (1MHz)
   * ARM_I2C_BUS_SPEED_HIGH           | High Speed (3.4MHz)
   *
   * For Mode Control Byte  (ARM_I2C_BUS_CLEAR) : No argument, just Clear the bus by sending nine clock pulses
   * For Mode Control Byte  (ARM_I2C_ABORT_TRANSFER) : No argument, just abort communication
   */
  static int32_t Control(uint32_t control, uint32_t arg) {
    return _driver_ARM_I2C::Control(I2cIndex, control, arg);
  }

  /**
   * \ingroup cmsis_driver_i2c
   * \brief Start sending data to I2C transmitter : MasterTransmit
   *
   * \param [in]  addr  Slave address to which data is to sent
   * \param [in]  data  Pointer to buffer with data to transmit to I2C Slave
   * \param [in]  num   Number of data bytes to transmit
   * \param [in]  xfer_pending   Transfer operation is pending - Stop condition will not be generated
   *
   * \return \ref cmsis_driver_general_return_codes "Status Error Codes"
   *
   *This function ARM_I2C_MasterTransmit transmits data as Master to the selected Slave.

   * The operation consists of:
   *    Master generates START condition
   *    Master addresses the Slave as Master Transmitter
   *    Master transmits data to the addressed Slave
   *    Master generates STOP condition (if xfer_pending is "false")
   *
   * The parameter data and num specify the address of a data buffer and the number of bytes to transmit.
   * Set the parameter xfer_pending to 'true' if another transfer operation follows.
   * With xfer_pending set to 'false' a STOP condition is generated.
   *
   * The function is non-blocking and returns as soon as the driver has started the operation.
   *
   * During the operation it is not allowed to call any Master function again.
   * Also the data buffer must stay allocated and the contents of data must not be modified.
   * When transmit operation has finished the
   *    + ARM_I2C_EVENT_TRANSFER_DONE event is generated.
   * When not all the data is transferred then the
   *    + ARM_I2C_EVENT_TRANSFER_INCOMPLETE flag is set at the same time.
   * Number of data bytes transmitted and acknowledged is returned by the function
   *    + ARM_I2C_GetDataCount during and after the operation has finished.
   * The operation is aborted in the following cases (ARM_I2C_EVENT_TRANSFER_DONE event is generated together with):
   *    + selected slave has not acknowledged the address: ARM_I2C_EVENT_ADDRESS_NACK event
   *    + arbitration has been lost: ARM_I2C_EVENT_ARBITRATION_LOST event
   *    + bus error has been detected: ARM_I2C_EVENT_BUS_ERROR event
   * Status can be monitored by calling the
   *    + ARM_I2C_GetStatus and checking the flags.
   * Transmit operation can be aborted also by calling
   *    + ARM_I2C_Control with the parameter control ARM_I2C_ABORT_TRANSFER.
   */
  static int32_t MasterTransmit(uint32_t addr,uint8_t *data,uint32_t num,bool xfer_pending)
  {
    return _driver_ARM_I2C::MasterTransmit(I2cIndex,addr,data,num,xfer_pending);
  }

  /**
   * \ingroup cmsis_driver_i2c
   * \brief Start receiving data from I2C receiver.
   *
   * \param [in]  addr  Slave address (7-bit or 10-bit)
   * \param [out] data  Pointer to buffer for data to receive from I2C Slave
   * \param [in]  num   Number of data bytes to receive.
   * \prama [in]  xfer_pending  Transfer operation is pending - Stop condition will not be generated
   *
   * \return \ref cmsis_driver_general_return_codes "Status Error Codes"
   *
   * This function ARM_I2C_MasterReceive is used to receive data as Master from the selected Slave.
   * The operation consists of:
   *  + Master generates START condition
   *  + Master addresses the Slave as Master Receiver
   *  + Master receives data from the addressed Slave
   *  + Master generates STOP condition (if xfer_pending is "false")
   *  The parameter addr is the address of the slave to receive the data from.
   *    + The value can be ORed with ARM_I2C_ADDRESS_10BIT to identify a 10-bit address value.
   *  The parameter data and num specify the address of a data buffer and the number of bytes to receive.
   *  Set the parameter xfer_pending to 'true' if another transfer operation follows.
   *    + With xfer_pending set to 'false' a STOP condition is generated.
   *
   *  The function is non-blocking and returns as soon as the driver has started the operation.
   *
   *  During the operation it is not allowed to call any Master function again.
   *  Also the data buffer must stay allocated.
   *  When receive operation has finished the
   *    + ARM_I2C_EVENT_TRANSFER_DONE event is generated.
   *  When not all the data is transferred then the
   *    +  ARM_I2C_EVENT_TRANSFER_INCOMPLETE flag is set at the same time.
   *  Number of data bytes received is returned by the function
   *    + ARM_I2C_GetDataCount during and after the operation has finished.
   *  The operation is aborted in the following cases (ARM_I2C_EVENT_TRANSFER_DONE event is generated together with):
   *    + selected slave has not acknowledged the address: ARM_I2C_EVENT_ADDRESS_NACK event
   *    + arbitration has been lost: ARM_I2C_EVENT_ARBITRATION_LOST event
   *    + bus error has been detected: ARM_I2C_EVENT_BUS_ERROR event
   *  Status can be monitored by calling the ARM_I2C_GetStatus and checking the flags.
   *  Receive operation can be aborted also by calling ARM_I2C_Control with the parameter control = ARM_I2C_ABORT_TRANSFER.
   *
   */
  static int32_t MasterReceive( uint32_t addr,uint8_t *data,uint32_t num,bool xfer_pending)
  {
    return _driver_ARM_I2C::MasterReceive(I2cIndex,addr,data,num,xfer_pending);
  }

  /**
    * \ingroup cmsis_driver_i2c
    * \brief Get I2C status.
    * \retval status \ref _ARM_I2C_STATUS.
    */
  static ARM_I2C_STATUS GetStatus() {
    return _driver_ARM_I2C::GetStatus(I2cIndex);
  }

  /* ------ Non hook-able functions ------ */

  /**
    * \ingroup cmsis_driver_i2c
    * \brief Get driver capabilities.
    * \return \ref _ARM_I2C_CAPABILITIES
    *
    * Retrieves information about capabilities in this driver implementation.
    * The bitfield members of the struct _ARM_I2C_CAPABILITIES encode various
    * capabilities, for example supported modes, if a hardware is capable to
    * create signal events using the \ref ARM_I2C_SignalEvent callback
    * function ...
    *
   Example:
   ~~~~~~~~{.c}
   extern ARM_DRIVER_I2C Driver_I2C0;
   ARM_DRIVER_I2C *drv_info;

   void read_capabilities (void)  {
     ARM_I2C_CAPABILITIES drv_capabilities;

     drv_info = &Driver_I2C0;
     drv_capabilities = drv_info->GetCapabilities ();
     // interrogate capabilities

   }
   ~~~~~~~~
    *
    *
    */
  static ARM_I2C_CAPABILITIES GetCapabilities()
  {
	  return *bapi_i2c_getCapabilities(I2cIndex);

  }

  /**
   * \ingroup cmsis_driver_i2c
   * @param Pointer to buffer with data to transmit to I2C Master
   * @param num Number of data bytes to transmit
   * @return Status Error Codes
   * \brief
   * This function ARM_I2C_SlaveTransmit is used to transmit data as Slave to the Master.
   * The parameter data is a pointer to the data to transmit.
   * The parameter num specifies the number of bytes to transmit.
   * The function is non-blocking and returns as soon as the driver has registered the operation.
   * The actual operation will start after being addressed by the master as a Slave Transmitter.
   * If the operation has not been registered at that point the ARM_I2C_EVENT_SLAVE_TRANSMIT event is generated.
   * The same event is also generated if the operation has finished (specified number of bytes transmitted) but more data is requested by the master.
   * It is not allowed to call this function again if the operation has started until it finishes.
   * Also the data buffer must stay allocated and the contents of data must not be modified.
   * When transmit operation has finished the ARM_I2C_EVENT_TRANSFER_DONE event is generated.
   * When not all the data is transferred then the ARM_I2C_EVENT_TRANSFER_INCOMPLETE flag is set at the same time.
   * Number of data bytes transmitted is returned by the function ARM_I2C_GetDataCount during and after the operation has finished.
   * In case that a General call has been detected the ARM_I2C_EVENT_GENERAL_CALL flag is indicated together with the ARM_I2C_EVENT_TRANSFER_DONE event
   * (also with ARM_I2C_EVENT_SLAVE_TRANSMIT event).
   * In case that bus error has been detected then the operation is aborted and the ARM_I2C_EVENT_BUS_ERROR event is generated together with ARM_I2C_EVENT_TRANSFER_DONE.
   * Slave will only respond to its own address (or General call if enabled) that is specified by calling ARM_I2C_Control with ARM_I2C_OWN_ADDRESS as control parameter. Using address 0 disables the slave.
   * Status can be monitored by calling the ARM_I2C_GetStatus and checking the flags.
   * Transmit operation can be canceled or aborted by calling ARM_I2C_Control with the parameter control = ARM_I2C_ABORT_TRANSFER.
   */
  static int32_t SlaveTransmit(uint8_t *data,uint32_t num)
  {
    return _driver_ARM_I2C::SlaveTransmit(I2cIndex,data,num);
  }

  /**
   * \ingroup cmsis_driver_i2c
   * @param data Pointer to buffer for data to receive from I2C Master
   * @param num Number of data bytes to receive
   * @return Status Error Codes
   *
   * This function ARM_I2C_SlaveReceive receives data as Slave from the Master.
   * The parameter data is a pointer to the data to receive. The parameter num specifies the number of bytes to receive.
   * The function is non-blocking and returns as soon as the driver has registered the operation.
   * The actual operation will start after being addressed by the master as a Slave Receiver.
   * If the operation has not been registered at that point the ARM_I2C_EVENT_SLAVE_RECEIVE event is generated.
   * It is not allowed to call this function again if the operation has started until it finishes.
   * Also the data buffer must stay allocated. When receive operation has finished the ARM_I2C_EVENT_TRANSFER_DONE event is generated.
   * When not all the data is transferred then the ARM_I2C_EVENT_TRANSFER_INCOMPLETE flag is set at the same time.
   * Number of data bytes received and acknowledged is returned by the function ARM_I2C_GetDataCount during and after the operation has finished.
   * In case that a General call has been detected the ARM_I2C_EVENT_GENERAL_CALL flag is indicated together with the ARM_I2C_EVENT_TRANSFER_DONE event (also with ARM_I2C_EVENT_SLAVE_RECEIVE event).
   * In case that bus error has been detected then the operation is aborted and the ARM_I2C_EVENT_BUS_ERROR event is generated together with ARM_I2C_EVENT_TRANSFER_DONE.
   * Slave will only respond to its own address (or General call if enabled) that is specified by calling ARM_I2C_Control with ARM_I2C_OWN_ADDRESS as control parameter.
   * Using address 0 disables the slave.
   * Status can be monitored by calling the ARM_I2C_GetStatus and checking the flags.
   * Receive operation can be canceled or aborted by calling ARM_I2C_Control with the parameter control = ARM_I2C_ABORT_TRANSFER.
   *
   */

  static int32_t SlaveReceive(uint8_t *data,uint32_t num)
  {
    return _driver_ARM_I2C::SlaveReceive(I2cIndex,data,num);
  }


  /**
    * \ingroup cmsis_driver_i2c
    * \brief Get transferred data count..
    * \return number of data bytes transferred; -1 when Slave is not addressed by Master
    *
    * The function ARM_I2C_GetDataCount returns the number of currently transferred data bytes during and after:
    *  + ARM_I2C_MasterTransmit : number of data bytes transmitted and acknowledged
    *  + ARM_I2C_MasterReceive : number of data bytes received
    *  + ARM_I2C_SlaveTransmit : number of data bytes transmitted
    *  + ARM_I2C_SlaveReceive : number of data bytes received and acknowledged
    *
    * When the Slave is not yet addressed by the Master then -1 is returned.
    *
    */
  static int32_t GetDataCount()
  {
    return _driver_ARM_I2C::GetDataCount(I2cIndex);
  }

  /**
    * \ingroup cmsis_driver_i2c
    * \brief Control I2C Interface Power.
    * \param [in] state Power state
    * \return \ref cmsis_driver_general_return_codes "Status Error Codes"
    *
    * Allows you to control the power modes of the I2C interface.
    */
  static int32_t PowerControl(ARM_POWER_STATE state) {
    return _driver_ARM_I2C::PowerControl(I2cIndex, state);
  }
 
};



/******************************************************************************
 * ARM_DRIVER_I2C: s_i2cDrivers
 *****************************************************************************/

/**
 * \ingroup _cmsis_driver_i2c
 * \brief The list of driver interface functions in the sequence as they appear in
 * struct _ARM_I2C_DRIVER.
 */
#define _ARM_I2C_DRIVER_VALUE_(I2cIndex) { \
   ARM_I2C_GetVersion \
  ,ARM_I2C<I2cIndex>::GetCapabilities \
  ,ARM_I2C<I2cIndex>::Initialize \
  ,ARM_I2C<I2cIndex>::Uninitialize \
  ,ARM_I2C<I2cIndex>::PowerControl \
  ,ARM_I2C<I2cIndex>::MasterTransmit \
  ,ARM_I2C<I2cIndex>::MasterReceive \
  ,ARM_I2C<I2cIndex>::SlaveTransmit \
  ,ARM_I2C<I2cIndex>::SlaveReceive \
  ,ARM_I2C<I2cIndex>::GetDataCount \
  ,ARM_I2C<I2cIndex>::Control \
  ,ARM_I2C<I2cIndex>::GetStatus \
  }

STATIC ARM_DRIVER_I2C s_I2cDrivers[] = {
   _ARM_I2C_DRIVER_VALUE_(bapi_E_I2c0)
#if (BAPI_HAS_I2C > 1)
  ,_ARM_I2C_DRIVER_VALUE_(bapi_E_I2c1)
#endif
#if (BAPI_HAS_I2C > 2)
  ,_ARM_I2C_DRIVER_VALUE_(bapi_E_I2c2)
#endif
#if (BAPI_HAS_I2C > 3)
  #error "More than 3 I2Cs defined. Please enhance according to the scheme above."
#endif
};


/**
 * \ingroup _cmsis_driver_i2c
 * \brief Load the pointer to access struct for the indexed instance of I2C
 * @param i2cindex
 * @return address of access struct for the indexed instance of I2C
 */
ARM_DRIVER_I2C* driver_i2c_getDriver(bapi_E_I2cIndex_ i2cindex)
{
  if(i2cindex != bapi_E_I2c_Invalid && i2cindex < bapi_E_I2cCount)
  {
    return &s_I2cDrivers[i2cindex];
  }
  return 0;
}



