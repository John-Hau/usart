
#ifndef _HalfduplexDriver_DriverUsart_H_
#define _HalfduplexDriver_DriverUsart_H_

#include "baseplate.h"
#include "cmsis-driver/Driver_USART.h"

/**
 * \file
 * \brief This file declares the API of Console the USART filter that can be
 * hooked in the CMSIS USART Driver.
 *
 */

/**
 * \defgroup rs485_usart_filter Halfduplex USART Filter
 * \ingroup usart_filters
 *
 * \brief This USART filter automatically switch of the UART receiver
 *   and switch on the RS485 transmitter before sending. It disables
 *   the RS485 transmitter and enables the receiver, when the last
 *   data word has been transmitted physically.
 *
 * \copydetails _rs485UsartFilter::Receive
 * \copydetails _rs485UsartFilter::Send
 *
 * \sa cmsis_driver_usart_ext_hook
 */

/**
 * \ingroup rs485_usart_filter
 * \brief This function hooks the rs485 USART filter into the CMSIS USART
 * driver.
 *
 * \warning This function can only be called when the rs485 driver for the
 * targeted USART is uninitialized!
 *
 * \return #ARM_DRIVER_OK if successful. #ARM_DRIVER_ERROR_BUSY if the driver
 * is initialized. ARM_DRIVER_ERROR if the driver is already hooked.
 */
C_FUNC int32_t rs485_driver_hookDriver(
	enum bapi_E_UartIndex_ uartIndex
);


C_FUNC int32_t rs485_driver_unhookDriver(
    enum bapi_E_UartIndex_ uartIndex
  );

#endif /* _HalfduplexDriver_DriverUsart_H_ */

