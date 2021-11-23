/*
 * osCom.h
 *
 *  Created on: 29.11.2016
 *      Author: Wolfgang
 */

#ifndef osComBacnet_H_
#define osComBacnet_H_


#include "baseplate.h"
#include "rtos/cmsis-rtos-ext/osCom.h"
#include "cmsis-driver/usart-filter/bacnetMSTP_usart_filter.h"



/**
 * \ingroup cmsis_os_ext_com
 * \brief
 * Establish the bacnet MSTP driver for a particular UART. Initialize the UART with MSTP default parameters.
 * Assign a file descriptor and create transmission and receive queues.
 * \note Works also in a Non RTOS environment.
 * \param[in] uartIndex The UART to initialize for BACnet communication.
 * \param[in] fd The file descriptor to be assigned to this UART.
 * \param[in] baudRate The baudrate that shall be used for the UART.
 * @return Either \code ARM_DRIVER_OK \endcode or any ARM_DRIVER_ERROR_* code.
 *
 * \note __For proper operation__, the bacnet MSTP driver __requires__ frequent calls of function
 *    \ref bacnetMSTP_driver_executeBacApp. In and Non RTOS environment it requires as well frequent
 *    calls of function \ref bacnetMSTP_driver_bgnd_timerpoll. In an RTOS environment it requires
 *    every 5 milliseconds a call of function \ref bacnetMSTP_driver_executeStateMachine.
 *
 */

C_INLINE int32_t osComBacnetInitialize(enum bapi_E_UartIndex_ uartIndex, int fd, uint32_t baudRate)
{
  enum{ MSTP_USART_QUEUE_SIZE = 8 };

	int32_t retVal = ARM_DRIVER_ERROR;
	bacnetMSTP_driver_hookDriver(uartIndex);
	retVal = osComUsartInitialize(uartIndex, fd, MSTP_USART_QUEUE_SIZE);
	if(retVal == ARM_DRIVER_OK)
	{
	  ARM_DRIVER_USART* driver = osComArmDriverUsartGetFromFd(fd);
		ASSERT(driver);
		(*driver->Control)(
         ARM_USART_MODE_ASYNCHRONOUS |
         ARM_USART_DATA_BITS_8 |
         ARM_USART_PARITY_NONE |
         ARM_USART_STOP_BITS_1
        ,baudRate
      );
	}
  return retVal;
}

#endif /* osComBacnet_H_ */
