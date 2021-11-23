/*
 * osCom.h
 *
 *  Created on: 07.04.2013
 *      Author: Wolfgang
 */

#ifndef osExUIO_H_
#define osExUIO_H_


#include "baseplate.h"
#include "boards/board-api/bapi_uart.h"
#include "cmsis-driver/Driver_USART.h"

#ifdef __IAR_SYSTEMS_ICC__
  #include <LowLevelIOInterface.h>
#else
  #include <unistd.h>
#endif

#include "boards/board-api/bapi_io.h"
#include "rtos/cmsis-rtos/cmsis_os_redirect.h"

void osExUioMutexCreat(void);
uint16_t osExUioGetAIValue(uint8_t uioPinIndex);
bool osExUioGetBIValue(uint8_t uioPinIndex);
bool osExUioSetAOValue(uint8_t uioPinIndex,uint32_t value);


bool osExUioCongigAiChanel(uint8_t uioPinIndex,IO_PIN_TYPE_t pintype);
bool osExUioCongigBiChanel(uint8_t uioPinIndex,IO_PIN_TYPE_t pintype);
bool osExUioCongigAoChanel(uint8_t uioPinIndex,IO_PIN_TYPE_t pintype);
bool osExUioConfigChanel(uint8_t uioPinIndex,IO_PIN_TYPE_t pintype);
uint8_t osExUioGet4BIValueinOneUIO(spi_bus_idx_t uio_SPIbusIndex,spi_dev_idx_t uio_deviceIndex);
uint8_t osExUioGetAlertStatus(void);


/**
 * \file
 * This file declares the ARM CMSIS RTOS ADC Extension API.
 */



#endif /* osIoAdc_H_ */
