
#ifndef _ConsoleDriver_DriverUsart_H_
#define _ConsoleDriver_DriverUsart_H_

#include "baseplate.h"
#include "cmsis-driver/Driver_USART.h"

/**
 * \file
 * \brief This file declares the API of Console the USART filter that can be
 * hooked in the CMSIS USART Driver.
 *
 */

/**
 * \defgroup console_usart_filter Console USART Filter
 * \ingroup usart_filters
 * \brief This USART filter does a special send and receive handling for a
 * console communication.
 *
 * \copydetails _consoleUsartFilter::Receive
 * \copydetails _consoleUsartFilter::Send
 *
 * \sa cmsis_driver_usart_ext_hook
 */

/**
 * \ingroup console_usart_filter
 * \brief This function hooks the console USART filter into the CMSIS USART
 * driver.
 *
 * \warning This function can only be called when the console driver for the
 * targeted USART is uninitialized!
 *
 * \return #ARM_DRIVER_OK if successful. #ARM_DRIVER_ERROR_BUSY if the driver
 * is initialized. ARM_DRIVER_ERROR if the driver is already hooked.
 */
C_FUNC int32_t console_driver_hookDriver(
	enum bapi_E_UartIndex_ uartIndex
);

#endif /* _ConsoleDriver_DriverUsart_H_ */

