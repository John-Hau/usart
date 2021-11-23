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
 * $Date:        24. Nov 2014
 * $Revision:    V2.02
 *
 * Project:      USART (Universal Synchronous Asynchronous Receiver Transmitter)
 *               Driver definitions
 * -------------------------------------------------------------------------- */

/* History:
 *  Version 2.02
 *    Corrected ARM_USART_CPOL_Pos and ARM_USART_CPHA_Pos definitions
 *  Version 2.01
 *    Removed optional argument parameter from Signal Event
 *  Version 2.00
 *    New simplified driver:
 *      complexity moved to upper layer (especially data handling)
 *      more unified API for different communication interfaces
 *      renamed driver UART -> USART (Asynchronous & Synchronous)
 *    Added modes:
 *      Synchronous
 *      Single-wire
 *      IrDA
 *      Smart Card
 *    Changed prefix ARM_DRV -> ARM_DRIVER
 *  Version 1.10
 *    Namespace prefix ARM_ added
 *  Version 1.01
 *    Added events:
 *      ARM_UART_EVENT_TX_EMPTY,     ARM_UART_EVENT_RX_TIMEOUT
 *      ARM_UART_EVENT_TX_THRESHOLD, ARM_UART_EVENT_RX_THRESHOLD
 *    Added functions: SetTxThreshold, SetRxThreshold
 *    Added "rx_timeout_event" to capabilities
 *  Version 1.00
 *    Initial release
 */


#ifndef _CMSIS_driver_DriverUsart_H_
#define _CMSIS_driver_DriverUsart_H_

#include <string.h>

/** Forward to the original */
#include "baseplate.h"
#include "boards/board-api/bapi_uart.h"
#include "boards/cmsis/Driver_USART.h"


/**
 * \file
 * \brief
 * This file provides the standard CMSIS USART API as well as ARM_DRIVER_USART
 * supplementary functionality. The API is provided by the include of
 * the header file boards/cmsis/Driver_USART.h
 */



/**
 * \addtogroup cmsis_driver_usart
 */
/**@{*/

/* 09/Oct./2015 WSC: Added additional parameter bapi_E_UartIndex_ to the callback function type ARM_USART_SignalEvent_t.
 * This was the only way to integrate the ARM_DRIVER_USART API easily into honeycomb. All other tried solution became
 * too complicated.
 */
typedef void (*ARM_USART_SignalEvent_t) (enum bapi_E_UartIndex_ uartIndex, uint32_t event);  ///< Pointer to \ref ARM_USART_SignalEvent : Signal USART Event.

/**
 * \brief Signal USART Events.
 *
 * @param [in] uartIndex the USART on which the event(s) occurred.
 * @param [in] event \ref cmsis_driver_usart_std_events notification mask.
 *
 * @return none.
 *
 * This function declaration is just for documentation. The real callback is
 * defined by the client application and may have a different name.
 *
 * The function ARM_USART_SignalEventenum bapi_E_UartIndex_, uint32_t) notifies
 * the application of the USART Events and it is registered by the function
 * ARM_USART::Initialize(ARM_USART_SignalEvent_t). The function
 * ARM_USART::GetCapabilities() returns information about the implemented
 * optional events in a driver.
 * The argument event represents the notification mask of the events. Each event
 * is coded in a separate bit and therefore it is possible to signal multiple
 * events in the event call back function. The following call back notifications
 * are generated:
 *
 * Bit| Event                                 | Description
 * -- | ------------------------------------- | -----------------------------------------
 *  0 | ARM_USART_EVENT_SEND_COMPLETE         | Occurs after call to \ref ARM_USART::Send to indicate that all the data to be sent was processed by the driver. All the data might have been already transmitted or parts of it is still queued in transmit buffers. The driver is ready for the next call to \ref ARM_USART::Send; however USART may still transmit data.
 *  1 | ARM_USART_EVENT_RECEIVE_COMPLETE      | Occurs after call to \ref ARM_USART::Receive to indicate that all the data has been received. The driver is ready for the next call to \ref ARM_USART::Receive.
 *  2 | ARM_USART_EVENT_TRANSFER_COMPLETE     | Occurs after call to \ref ARM_USART::Transfer to indicate that all the data has been transferred. The driver is ready for the next call to \ref ARM_USART::Transfer.
 *  3 | ARM_USART_EVENT_TX_COMPLETE (optional)| Occurs after #ARM_USART_EVENT_SEND_COMPLETE event when all data has been physically transmitted.
 *  4 | ARM_USART_EVENT_TX_UNDERFLOW          | Occurs in synchronous slave mode when data is requested by the master but send/receive/transfer operation has not been started.
 *  5 | ARM_USART_EVENT_RX_OVERFLOW           | Occurs when data is lost during receive/transfer operation or when data is lost because receive operation in asynchronous mode or receive/send/transfer operation in synchronous slave mode has not been started.
 *  6 | ARM_USART_EVENT_RX_TIMEOUT (optional) | Occurs during receive when idle time is detected between consecutive characters (idle time is hardware dependent).
 *  7 | ARM_USART_EVENT_RX_BREAK              | Occurs when break is detected during receive.
 *  8 | ARM_USART_EVENT_RX_FRAMING_ERROR      | Occurs when framing error is detected during receive.
 *  9 | ARM_USART_EVENT_RX_PARITY_ERROR       | Occurs when parity error is detected during receive.
 * 10 | ARM_USART_EVENT_CTS (optional)        | Indicates that CTS modem line state has changed.
 * 11 | ARM_USART_EVENT_DSR (optional)        | Indicates that DSR modem line state has changed.
 * 12 | ARM_USART_EVENT_DCD (optional)        | Indicates that DCD modem line state has changed.
 * 13 | ARM_USART_EVENT_RI (optional)         | Indicates that RI modem line state has changed.
 */
void ARM_USART_SignalEvent(enum bapi_E_UartIndex_ uartIndex, uint32_t event);

/**
 * \brief Provides USART interface functions. The interface functions of the USART driver are
 *  accessed by function pointers exposed by this structure.
 */
struct _ARM_DRIVER_USART {
  /**
   * \brief Get driver version.
   *
   * Returns version information of the driver implementation in ARM_DRIVER_VERSION
   *   - API version is the version of the CMSIS-Driver specification used to implement this driver.
   *   - Driver version is source code version of the actual driver implementation.
   *
   * @return The API Version and Driver Version in an _ARM_DRIVER_VERSION structure.
   *
   Example:
   ~~~~~~~~{.c}

   void setup_usart (void) {
     ARM_DRIVER_USART *drv = driver_flash_getDriver(bapi_E_Uart0) ;
     ARM_DRIVER_VERSION  version = drv->GetVersion();
     if (version.api < 0x10A) {      // requires at minimum API version 1.10 or higher
       // error handling
       return;
     }
   }
   ~~~~~~~~
   */
  ARM_DRIVER_VERSION     (*GetVersion)      (void);

  /**
    * \brief Get driver capabilities.
    * @return \ref _ARM_USART_CAPABILITIES
    *
    * Retrieves information about capabilities in this driver implementation.
    * The bitfield members of the struct _ARM_USART_CAPABILITIES encode various
    * capabilities, for example supported modes, if a hardware is capable to
    * create signal events using the \ref ARM_USART_SignalEvent callback
    * function ...
    *
   Example:
   ~~~~~~~~{.c}
   void read_capabilities (void)  {
     ARM_DRIVER_USART *drv = driver_flash_getDriver(bapi_E_Uart0) ;
     ARM_USART_CAPABILITIES drv_capabilities = drv->GetCapabilities();
     // interrogate capabilities

   }
   ~~~~~~~~
    *
    *
    */
  ARM_USART_CAPABILITIES (*GetCapabilities) (void);

  /**
   * \brief Initialize USART Interface.
   *
   * The function initializes the USART interface. It is called when the
   * middleware component starts operation.
   *
   * @param [in] cb_event Pointer to a callback function with the signature of
   * \ref ARM_USART_SignalEvent
   *
   * @return #ARM_DRIVER_OK upon success. #ARM_DRIVER_ERROR, if the driver
   * is already initialized with a different cb_event.
   *
   * The function performs the following operations:
   *  - Initializes the resources needed for the USART interface.
   *  - Registers the ARM_USART_SignalEvent callback function.
   *
   * The parameter cb_event is a pointer to the callback function with the
   * signature of \ref ARM_USART_SignalEvent; use a NULL pointer when no
   * callback signals are required.
   *
   */
  int32_t                (*Initialize)      (ARM_USART_SignalEvent_t cb_event);

  /**
   * \brief De-initialize USART Interface.
   * @return Common \ref cmsis_driver_general_return_codes "Status Error Codes"
   *
   * The function de-initializes the resources of
   * USART interface. It is called when the middleware component stops
   * operation and releases the software resources used by the interface.
   */
  int32_t                (*Uninitialize)    (void);

  /**
    * \brief Control USART Interface Power.
    * @param [in] state Power state
    * @return \ref cmsis_driver_general_return_codes "Status Error Codes"
    *
    * Allows you to control the power modes of the USART interface.
    */
  int32_t                (*PowerControl)    (ARM_POWER_STATE state);

  /**
   * \brief Start sending data to USART transmitter.
   *
   * @param [in]  data  Pointer to buffer with data to send to USART transmitter
   * @param [in]  num Number of data items to send
   *
   * @return \ref cmsis_driver_general_return_codes "Status Error Codes"
   *
   * This functions is used in asynchronous mode to send data to the USART
   * transmitter. It can also be used in synchronous mode when sending data
   * only (received data is ignored).
   *
   * Transmitter needs to be enabled by calling \ref ARM_USART::Control with
   * #ARM_USART_CONTROL_TX as the control parameter and 1 as argument.
   *
   * The function parameters specify the buffer with data and the number of
   * items to send. The item size is defined by the data type which depends on
   * the configured number of data bits.
   *
   * Data type is:
   *
   *   - uint8_t when configured for 5..8 data bits
   *   - uint16_t when configured for 9 data bits
   *
   * Calling the function \ref ARM_USART::Send only starts the send operation.
   * The function is non-blocking and returns as soon as the driver has started
   * the operation (driver typically configures DMA or the interrupt system for
   * continuous transfer). When in synchronous slave mode the operation is only
   * registered and started when the master starts the transfer. During the
   * operation it is not allowed to call this function again or any other data
   * transfer function when in synchronous mode. Also the data buffer must stay
   * allocated and the contents of unsent data must not be modified. When send
   * operation is completed (requested number of items sent) the
   * #ARM_USART_EVENT_SEND_COMPLETE event is generated. Progress of send
   * operation can also be monitored by reading the number of items already sent
   * by calling \ref ARM_USART::GetTxCount.
   *
   * After send operation has completed there might still be some data left in
   * the driver's hardware buffer which is still being transmitted. When all
   * data has been physically transmitted the #ARM_USART_EVENT_TX_COMPLETE
   * event is generated (if supported and reported by event_tx_complete in
   * _ARM_USART_CAPABILITIES). At that point also the tx_busy flag in
   * _ARM_USART_STATUS is cleared.
   *
   * Status of the transmitter can be monitored by calling the
   * \ref ARM_USART::GetStatus and checking the tx_busy flag which indicates if
   * transmission is still in progress.
   *
   * When in synchronous slave mode and transmitter is enabled but
   * send/receive/transfer operation is not started and data is requested by
   * the master then the #ARM_USART_EVENT_TX_UNDERFLOW event is generated.
   *
   * Send operation can be aborted by calling \ref ARM_USART::Control with
   * #ARM_USART_ABORT_SEND as the control parameter.
   */
  int32_t                (*Send)            (const void *data, uint32_t num);

  /**
   * \brief Start receiving data from USART receiver.
   *
   * @param [out] data  Pointer to buffer for data to receive from USART receiver.
   * @param [in]  num Number of data items to receive.
   *
   * @return \ref cmsis_driver_general_return_codes "Status Error Codes"
   *
   * This functions is used in asynchronous mode to receive data from the USART
   * receiver. It can also be used in synchronous mode when receiving data only
   * (transmits the default value as specified by \ref ARM_USART::Control with
   * #ARM_USART_SET_DEFAULT_TX_VALUE as control parameter).
   *
   * Receiver needs to be enabled by calling ARM_USART_Control with
   * #ARM_USART_CONTROL_RX as the control parameter and 1 as argument.
   *
   * The function parameters specify the buffer for data and the number of
   * items to receive. The item size is defined by the data type which depends
   * on the configured number of data bits.
   *
   * Data type is:
   *
   *   - uint8_t when configured for 5..8 data bits
   *   - uint16_t when configured for 9 data bits
   *
   * Calling the function ARM_USART_Receive only starts the receive operation.
   * The function is non-blocking and returns as soon as the driver has started
   * the operation (driver typically configures DMA or the interrupt system for
   * continuous transfer). When in synchronous slave mode the operation is only
   * registered and started when the master starts the transfer. During the
   * operation it is not allowed to call this function again or any other data
   * transfer function when in synchronous mode. Also the data buffer must stay
   * allocated. When receive operation is completed (requested number of items
   * received) the ARM_USART_EVENT_RECEIVE_COMPLETE event is generated.
   * Progress of receive operation can also be monitored by reading the number
   * of items already received by calling \ref ARM_USART::GetRxCount.
   *
   * Status of the receiver can be monitored by calling the
   * ARM_USART::GetStatus and checking the rx_busy flag which indicates if
   * reception is still in progress.
   *
   * During reception the following events can be generated (in asynchronous
   * mode):
   *
   *   - #ARM_USART_EVENT_RX_TIMEOUT : Receive timeout between consecutive
   *      characters detected (optional)
   *   - #ARM_USART_EVENT_RX_BREAK : Break detected (Framing error is not
   *      generated for Break condition)
   *   - #ARM_USART_EVENT_RX_FRAMING_ERROR : Framing error detected
   *   - #ARM_USART_EVENT_RX_PARITY_ERROR : Parity error detected
   *   - #ARM_USART_EVENT_RX_OVERFLOW : Data overflow detected (also
   *      in synchronous slave mode)
   *
   * #ARM_USART_EVENT_RX_OVERFLOW event is also generated when receiver is
   * enabled but data is lost because receive operation in asynchronous mode or
   * receive/send/transfer operation in synchronous slave mode has not been started.
   *
   * Receive operation can be aborted by calling ARM_USART::Control with
   * #ARM_USART_ABORT_RECEIVE as the control parameter.
   *
   */
  int32_t                (*Receive)         (      void *data, uint32_t num);

  /**
    * \brief Start sending/receiving data to/from USART transmitter/receiver.
    *
    * @param [in]  data_out Pointer to buffer with data to send to USART transmitter
    * @param [out] data_in Pointer to buffer for data to receive from USART receiver
    * @param [in]  num Number of data items to transfer
    *
    * @return \ref cmsis_driver_general_return_codes "Status Error Codes"
    *
    * This functions is used in synchronous mode to transfer data via USART.
    * It synchronously sends data to the USART transmitter and receives data
    * from the USART receiver.
    *
    * Transmitter needs to be enabled by calling ARM_USART::Control with
    * #ARM_USART_CONTROL_TX as the control parameter and 1 as argument.
    * Receiver needs to be enabled by calling ARM_USART_Control with
    * #ARM_USART_CONTROL_RX as the control parameter and 1 as argument.
    *
    * The function parameters specify the buffer with data to send, the buffer
    * for data to receive and the number of items to transfer. The item size
    * is defined by the data type which depends on the configured number of
    * data bits.
    *
    * Data type is:
    *
    *   - uint8_t when configured for 5..8 data bits
    *   - uint16_t when configured for 9 data bits
    *
    * Calling the function ARM_USART_Transfer only starts the transfer operation.
    * The function is non-blocking and returns as soon as the driver has started
    * the operation (driver typically configures DMA or the interrupt system for
    * continuous transfer). When in synchronous slave mode the operation is only
    * registered and started when the master starts the transfer. During the
    * operation it is not allowed to call this function or any other data
    * transfer function again. Also the data buffers must stay allocated and the
    * contents of unsent data must not be modified. When transfer operation is
    * completed (requested number of items transferred) the
    * #ARM_USART_EVENT_TRANSFER_COMPLETE event is generated. Progress of transfer
    * operation can also be monitored by reading the number of items already
    * transferred by calling ARM_USART_GetTxCount or ARM_USART_GetRxCount.
    *
    * Status of the transmitter or receiver can be monitored by calling the
    * ARM_USART::GetStatus and checking the tx_busy or rx_busy flag.
    *
    * When in synchronous slave mode also the following events can be generated:
    *
    *   - #ARM_USART_EVENT_TX_UNDERFLOW : transmitter is enabled but transfer
    *       operation is not started and data is requested by the master
    *   - #ARM_USART_EVENT_RX_OVERFLOW : data lost during transfer or because
    *       receiver is enabled but transfer operation has not been started
    *
    * Transfer operation can also be aborted by calling ARM_USART::Control with
    * #ARM_USART_ABORT_TRANSFER as the control parameter.
    *
    */
  int32_t                (*Transfer)        (const void *data_out,
                                                   void *data_in,
                                             uint32_t    num);
  /**
    * \brief Get transmitted data count.
    * @return number of data items transmitted
    *
    * Returns the number of the currently transmitted data items during
    * \ref ARM_USART::Send and \ref ARM_USART::Transfer operation.
    */
  uint32_t               (*GetTxCount)      (void);

  /**
    * \brief Get received data count.
    * @return number of data items received
    *
    * Returns the number of the currently received data items during
    * \ref ARM_USART::Receive and \ref ARM_USART::Transfer operation.
    *
    */
  uint32_t               (*GetRxCount)      (void);

  /**
   * \brief Control USART Interface.
   *
   * @param control [in] Operation
   * @param arg [in] Argument of Operation
   *
   * @return Common \ref cmsis_driver_general_return_codes "Status Error Codes"
   * and driver specific \ref cmsis_driver_usart_return_codes "Status Error Codes"
   *
   * Control the USART interface settings and execute various operations.
   * The parameter _control_ is a bit mask that specifies various operations
   * (see tables below). The control bits of the various groups can be combined
   * to a control code as shown in the following example. Depending on the
   * control bits, the parameter _arg_ provides additional information, for
   * example the baudrate.
   *
   Example:
   ~~~~~~~~{.c}
   extern ARM_DRIVER_USART Driver_USART0;
   // configure to UART mode: 8 bits, no parity, 1 stop bit, no flow control, 9600 bps
   status = Driver_USART0.Control(ARM_USART_MODE_ASYNCHRONOUS |
                                  ARM_USART_DATA_BITS_8 |
                                  ARM_USART_PARITY_NONE |
                                  ARM_USART_STOP_BITS_1 |
                                  ARM_USART_FLOW_CONTROL_NONE, 9600);
   // identical with above settings (default settings removed)
   // configure to UART mode: 8 bits, no parity, 1 stop bit, flow control, 9600 bps
   status = Driver_USART0.Control(ARM_USART_MODE_ASYNCHRONOUS, 9600);
   // enable TX output
   status = Driver_USART0.Control(ARM_USART_CONTROL_TX, 1);
   // disable RX output
   status = Driver_USART0.Control(ARM_USART_CONTROL_RX, 0);
   ~~~~~~~~
   *
   * __ARM_USART_MODE_xxx__ control bits specify the USART mode:
   *
   * Mode Control Bits                  | Description
   * ---------------------------------- | ------------------------------------------------------------------------------------
   * #ARM_USART_MODE_ASYNCHRONOUS       | Set to asynchronous UART mode. arg specifies baudrate.
   * #ARM_USART_MODE_SYNCHRONOUS_MASTER | Set to synchronous master mode with clock signal generation. arg specifies baudrate.
   * #ARM_USART_MODE_SYNCHRONOUS_SLAVE  | Set to synchronous slave mode with external clock signal.
   * #ARM_USART_MODE_SINGLE_WIRE        | Set to single-wire (half-duplex) mode. arg specifies baudrate.
   * #ARM_USART_MODE_IRDA               | Set to Infrared data mode. arg specifies baudrate.
   * #ARM_USART_MODE_SMART_CARD         | Set to Smart Card mode. arg specifies baudrate.
   *
   * The Mode Control bits can be combined with Mode Control Parameters to
   * specify data bits, parity, stop bits, flow control, clock polarity, and clock phase.
   *
   * __ARM_USART_DATA_xxx__ control bits specify the number of data bits:
   *
   * Mode Parameters: Data Bits  | Description
   * --------------------------- | ----------------------------
   * #ARM_USART_DATA_BITS_5      | Set to 5 data bits
   * #ARM_USART_DATA_BITS_6      | Set to 6 data bits
   * #ARM_USART_DATA_BITS_7      | Set to 7 data bits
   * #ARM_USART_DATA_BITS_8      | Set to 8 data bits (default)
   * #ARM_USART_DATA_BITS_9      | Set to 9 data bits
   *
   * __ARM_USART_PARITY_xxx__ control bits specify the parity bit:
   *
   * Mode Parameters: Parity | Description
   * ----------------------- | ----------------------------
   * ARM_USART_PARITY_EVEN   | Set to Even Parity
   * ARM_USART_PARITY_NONE   | Set to No Parity (default)
   * ARM_USART_PARITY_ODD    | Set to Odd Parity
   *
   * __ARM_USART_STOP_BITS_xxx__ control bits specify the number of stop bits:
   *
   * Mode Parameters: Stop Bits|  Description
   * ------------------------- | ----------------------------
   * ARM_USART_STOP_BITS_1     | Set to 1 Stop bit (default)
   * ARM_USART_STOP_BITS_2     | Set to 2 Stop bits
   * ARM_USART_STOP_BITS_1_5   | Set to 1.5 Stop bits
   * ARM_USART_STOP_BITS_0_5   | Set to 0.5 Stop bits
   *
   * __ARM_USART_FLOW_CONTROL_xxx__ control bits specify the RTS/CTS flow control:
   *
   * Mode Parameters: Flow Control  | Description
   * ------------------------------ | ----------------------------------------------
   * ARM_USART_FLOW_CONTROL_NONE    | No flow control signal (default)
   * ARM_USART_FLOW_CONTROL_CTS     | Set to use the CTS flow control signal
   * ARM_USART_FLOW_CONTROL_RTS     | Set to use the RTS flow control signal
   * ARM_USART_FLOW_CONTROL_RTS_CTS | Set to use the RTS and CTS flow control signal
   *
   * __ARM_USART_CPOLx__ define the clock polarity for synchronous mode:
   *
   * Mode Parameters: Clock Polarity | Description
   * ------------------------------- | --------------------------------------------------------------------------
   * ARM_USART_CPOL0                 | CPOL=0 (default) : data are captured on rising edge (low->high transition)
   * ARM_USART_CPOL1                 | CPOL=1 : data are captured on falling edge (high->lowh transition)
   *
   * __ARM_USART_CPHAx__ define the clock phase for synchronous mode:
   *
   * Mode Parameters: Clock Phase    | Description
   * ------------------------------- | --------------------------------------------------
   * ARM_USART_CPHA0                 | CPHA=0 (default) : sample on first (leading) edge
   * ARM_USART_CPHA1                 | CPHA=1 : sample on second (trailing) edge
   *
   * __Miscellaneous Controls__ execute various operations:
   *
   * Miscellaneous Controls               | Description
   * ------------------------------------ | ------------------------------------------------------------------------------------
   * #ARM_USART_ABORT_RECEIVE             | Abort receive operation (see also: ARM_USART::Receive)
   * #ARM_USART_ABORT_SEND                | Abort send operation (see also: ARM_USART::Send)
   * #ARM_USART_ABORT_TRANSFER            | Abort transfer operation (see also: ARM_USART::Transfer)
   * #ARM_USART_CONTROL_BREAK             | Enable or disable continuous Break transmission; arg : 0=disabled; 1=enabled
   * #ARM_USART_CONTROL_RX                | Enable or disable receiver; arg : 0=disabled; 1=enabled (see also: ARM_USART::Receive; ARM_USART::Transfer)
   * #ARM_USART_CONTROL_SMART_CARD_NACK   | Enable or disable Smart Card NACK generation; arg : 0=disabled; 1=enabled
   * #ARM_USART_CONTROL_TX                | Enable or disable transmitter; arg : 0=disabled; 1=enabled (see also: ARM_USART::Send; ARM_USART::Transfer)
   * #ARM_USART_SET_DEFAULT_TX_VALUE      | Set the default transmit value (synchronous receive only); arg specifies the value. (see also: ARM_USART::Receive)
   * #ARM_USART_SET_IRDA_PULSE            | Set the IrDA pulse value in ns; arg : 0=3/16 of bit period
   * #ARM_USART_SET_SMART_CARD_CLOCK      | Set the Smart Card Clock in Hz; arg : 0=Clock not set
   * #ARM_USART_SET_SMART_CARD_GUARD_TIME | Set the Smart Card guard time; arg = number of bit periods
   *
   */
  int32_t                (*Control)         (uint32_t control, uint32_t arg);

   /**
     * \brief Get USART status.
     * \retval status \ref _ARM_USART_STATUS.
     */
  ARM_USART_STATUS (*GetStatus)       (void);

  /**
    * \brief Set USART Modem Control line state.
    * @param [in] control \ref _ARM_USART_MODEM_CONTROL
    * @return \ref cmsis_driver_general_return_codes "Status Error Codes"
    *
    * Activate or deactivate the selected USART modem control line.
    * The function ARM_USART::GetModemStatus returns information about status
    * of the modem lines.
    */
  int32_t                (*SetModemControl) (ARM_USART_MODEM_CONTROL control);

  /**
    * \brief Get USART Modem Status lines state.
    * @return modem status ref _ARM_USART_MODEM_STATUS
    *
    * Get the current USART Modem Status lines state. The function
    * \ref ARM_USART::SetModemControl sets the modem control lines
    * of the USART.
    */
  ARM_USART_MODEM_STATUS (*GetModemStatus)  (void);
};


/**@} cmsis_driver_usart */

/**
 * \addtogroup cmsis_driver_usart
 */
/**@{*/
typedef const struct _ARM_DRIVER_USART ARM_DRIVER_USART;
/**@} cmsis_driver_usart */


/**
 * \ingroup cmsis_driver_usart
 * \brief
 * Obtain the CMSIS _ARM_DRIVER_USART access structure for a particular USART
 * by USART index.
 *
 * The CMSIS Driver specifications defines just fixed names for the USART
 * driver instances. This does badly support access to drivers at runtime
 * via USART index. This supplementary function closes this gap.
 *
 * @return Pointer to the USART Driver structure.
 */
C_FUNC ARM_DRIVER_USART* driver_usart_getDriver(
    bapi_E_UartIndex uartIndex /**< [in] The USART index for which to obtain the driver. */
  );

/**
 * \addtogroup cmsis_driver_usart_instance_names USART Driver Instance Names
 */
/**@{*/
/** CMSIS USART 0 Driver Instance Name. */
#if (BAPI_HAS_USART > 0)
  #define Driver_USART0 (*driver_usart_getDriver(bapi_E_Uart0))
#endif
/** CMSIS USART 1 Driver Instance Name. */
#if (BAPI_HAS_USART > 1)
  #define Driver_USART1 (*driver_usart_getDriver(bapi_E_Uart1))
#endif
/** CMSIS USART 2 Driver Instance Name. */
#if (BAPI_HAS_USART > 2)
  #define Driver_USART2 (*driver_usart_getDriver(bapi_E_Uart2))
#endif
/** CMSIS USART 3 Driver Instance Name. */
#if (BAPI_HAS_USART > 3)
  #define Driver_USART3 (*driver_usart_getDriver(bapi_E_Uart3))
#endif
/** CMSIS USART 4 Driver Instance Name. */
#if (BAPI_HAS_USART > 4)
  #define Driver_USART4 (*driver_usart_getDriver(bapi_E_Uart4))
#endif
/** CMSIS USART 5 Driver Instance Name. */
#if (BAPI_HAS_USART > 5)
  #define Driver_USART5 (*driver_usart_getDriver(bapi_E_Uart5))
#endif
/** CMSIS USART 6 Driver Instance Name. */
#if (BAPI_HAS_USART > 6)
  #define Driver_USART6 (*driver_usart_getDriver(bapi_E_Uart6))
#endif
/** CMSIS USART 7 Driver Instance Name. */
#if (BAPI_HAS_USART > 7)
  #define Driver_USART7 (*driver_usart_getDriver(bapi_E_Uart7))
#endif
/**@}*/


/**
 * \ingroup cmsis_driver_usart
 * \brief
 * Retrieve the transmission state for a particular USART. A driver hook may need to
 * access this driver data.
 */
C_FUNC struct bapi_uart_TransmissionState* driver_usart_getTransmissionState(
  enum bapi_E_UartIndex_ uartIndex /**< [in] The USART for which to get the transmission state. */
  );

/**
 * \addtogroup cmsis_driver_usart_ext_hook
 */
typedef ARM_USART_SignalEvent_t _driver_usartDriverHookSignalEvent_t;
typedef int32_t (*InitializeFunction_t)(const enum bapi_E_UartIndex_ uartIndex, _driver_usartDriverHookSignalEvent_t driverHook_cb_event);
typedef int32_t (*UninitializeFunction_t)(const enum bapi_E_UartIndex_ uartIndex);
typedef int32_t (*ControlFunction_t)(enum bapi_E_UartIndex_ uartIndex, uint32_t control, uint32_t arg);
typedef int32_t (*ReceiveFunction_t)(enum bapi_E_UartIndex_ uartIndex, void *data, uint32_t num);
typedef int32_t (*SendFunction_t)(enum bapi_E_UartIndex_ uartIndex, const void *data, uint32_t num);
typedef ARM_USART_STATUS (*GetStatusFunction_t)(enum bapi_E_UartIndex_ uartIndex);
/**@}*/

/**
 * \ingroup cmsis_driver_usart_ext_hook
 * \brief
 * This structure is a list of function pointers to functions that can hook into the CMSIS USART driver.
 * <b>All members of this structures must be function pointers only!</b>
 *
 * There are 2 major purposes of this structure:
 *
 * 1) Setting hooks for a particular UART by passing this structure to the function
 *      driver_usart_setHooks(enum bapi_E_UartIndex_, const struct arm_usart_DriverHooks *const, struct arm_usart_DriverHooks *const).
 *
 * 2) Storing the actual hooks for every UART within static variables of the Driver_USART module.
 *      There are static variables of this structure instantiated. Those are:
 *        ARM_USART<uartIndex>::defaultHooks
 *        ARM_USART<uartIndex>::hooks
 *
 *      Those static variables are initialized with valid values (hook functions). So if this structure
 *      is changed or enhanced it must be ensured that the initialization code of those variables are
 *      adjusted accordingly.
 *
 */
struct driver_usart_Hooks {
  /** Pointer to the Initialize(..) function. Provides a hook into the Initialize function. */
  InitializeFunction_t Initialize;

  /** Pointer to the Uninitialize(..) function. Provides a hook into the Uninitialize function. */
  UninitializeFunction_t Uninitialize;

  /** Pointer to the Control(..) function. Provides a hook into the Control function. */
  ControlFunction_t Control;

  /* Pointer to the Receive(..) function. Provides a hook into the Receive function. */
  ReceiveFunction_t Receive;

  /* Pointer to the Send(..) function. Provides a hook into the Send function. */
  SendFunction_t Send;

  /* Pointer to the GetStatus(..) function. Provides a hook into the GetStatus function. */
  GetStatusFunction_t GetStatus;
};

/**
 * \ingroup cmsis_driver_usart_ext_hook
 * \brief Alias for the number of hook functions in struct driver_usart_Hooks.
 */
enum { _driver_usart_HOOK_FUNCTION_COUNT = (sizeof(struct driver_usart_Hooks)/sizeof(void*)) };

/**
 * \ingroup cmsis_driver_usart_ext_hook
 * \brief
 * Set new driver hooks. This function can only be called when the driver for the targeted
 * UART is uninitialized! Refer to \ref struct _ARM_DRIVER_USART.Initialize respectively \ref struct _ARM_DRIVER_USART.Uninitialize.
 *
 * This will set hook functions in the USART driver for a particular USART index. After this function returned, the driver
 * will call the new hook functions instead of the one's that were previously installed.
 *
 * The new hooks are passed in a arm_usart_DriverHooks structure. Hook members which are
 * set to NULL in that structure will be skipped. That means the corresponding current
 * hook function in the driver is kept.
 *
 * @return ARM_DRIVER_OK if successful. ARM_DRIVER_ERROR_BUSY if the driver is initialized.
 */
C_FUNC int32_t driver_usart_setHooks(
  enum bapi_E_UartIndex_ uartIndex,                /**< [in]  The USART for which to set the hooks. */
  const struct driver_usart_Hooks *const hooks,    /**< [in]  The hooks to be set. NULL members are skipped. */
  struct driver_usart_Hooks *const replacedHooks   /**< [out] The old hooks, that have been replaced.
                                                              Hooks that have not been replaced will
                                                              be NULL in this returned structure. */
);

/**
 * \ingropup cmsis_driver_usart_ext_hook
 * \brief
 * Set new driver hooks.This function can only be called when the driver for the targeted
 * UART is uninitialized!
 *
 * This will set hook functions in the USART driver to it's default values.
 *
 * @return ARM_DRIVER_OK if successful. ARM_DRIVER_ERROR_BUSY if the driver is initialized.
 */
C_DECL int32_t driver_usart_resetHooks(
  enum bapi_E_UartIndex_ uartIndex             /**< [in] The USART for which to reset all hooks. */
);

/**
 * \ingroup cmsis_driver_usart_ext_hook
 * \brief
 * Set all members of a arm_usart_DriverHooks structure to zero.
 */
C_INLINE void driver_usart_initHooks(
  struct driver_usart_Hooks* hooks /**< [out] The structure to be initialized */
  ) {
  /* We assume that arm_usart_DriverHooks is a POD */
  MEMSET(hooks, 0, sizeof(*hooks));
}


#endif /* _CMSIS_driver_DriverUsart_H_ */

