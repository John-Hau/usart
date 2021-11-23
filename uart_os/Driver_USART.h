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

#ifndef __DRIVER_USART_H
#define __DRIVER_USART_H

#include "Driver_Common.h"



#define ARM_USART_API_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(2,02)  /* API version */

/****** USART Control Codes *****/
/**
 * \addtogroup cmsis_driver_usart_control_codes
 */
/**@{*/

#define ARM_USART_CONTROL_Pos                0                                ///< Bitfield position for __Control Codes: Mode__ and __Control Codes: Miscellaneous Controls__
#define ARM_USART_CONTROL_Msk               (0xFFUL << ARM_USART_CONTROL_Pos) ///< Bitfield mask for __Control Codes: Mode__ and __Control Codes: Miscellaneous Controls__

/*----- USART Control Codes: Mode -----*/
/** \name USART Control Codes: Mode
 * \brief Possible values for the control parameter of function struct _ARM_DRIVER_USART.Control */
/**@{*/
#define ARM_USART_MODE_ASYNCHRONOUS         (0x01UL << ARM_USART_CONTROL_Pos)   ///< UART (Asynchronous); arg = Baudrate
#define ARM_USART_MODE_SYNCHRONOUS_MASTER   (0x02UL << ARM_USART_CONTROL_Pos)   ///< Synchronous Master (generates clock signal); arg = Baudrate
#define ARM_USART_MODE_SYNCHRONOUS_SLAVE    (0x03UL << ARM_USART_CONTROL_Pos)   ///< Synchronous Slave (external clock signal)
#define ARM_USART_MODE_SINGLE_WIRE          (0x04UL << ARM_USART_CONTROL_Pos)   ///< UART Single-wire (half-duplex); arg = Baudrate
#define ARM_USART_MODE_IRDA                 (0x05UL << ARM_USART_CONTROL_Pos)   ///< UART IrDA; arg = Baudrate
#define ARM_USART_MODE_SMART_CARD           (0x06UL << ARM_USART_CONTROL_Pos)   ///< UART Smart Card; arg = Baudrate
/**@} USART Control Codes: Mode */

/*----- USART Control Codes: Miscellaneous Controls  -----*/
/** \name USART Control Codes: Miscellaneous Controls */
/**@{*/
#define ARM_USART_SET_DEFAULT_TX_VALUE      (0x10UL << ARM_USART_CONTROL_Pos)   ///< Set default Transmit value (Synchronous Receive only); arg = value
#define ARM_USART_SET_IRDA_PULSE            (0x11UL << ARM_USART_CONTROL_Pos)   ///< Set IrDA Pulse in ns; arg: 0=3/16 of bit period
#define ARM_USART_SET_SMART_CARD_GUARD_TIME (0x12UL << ARM_USART_CONTROL_Pos)   ///< Set Smart Card Guard Time; arg = number of bit periods
#define ARM_USART_SET_SMART_CARD_CLOCK      (0x13UL << ARM_USART_CONTROL_Pos)   ///< Set Smart Card Clock in Hz; arg: 0=Clock not generated
#define ARM_USART_CONTROL_SMART_CARD_NACK   (0x14UL << ARM_USART_CONTROL_Pos)   ///< Smart Card NACK generation; arg: 0=disabled, 1=enabled
#define ARM_USART_CONTROL_TX                (0x15UL << ARM_USART_CONTROL_Pos)   ///< Transmitter; arg: 0=disabled, 1=enabled
#define ARM_USART_CONTROL_RX                (0x16UL << ARM_USART_CONTROL_Pos)   ///< Receiver; arg: 0=disabled, 1=enabled
#define ARM_USART_CONTROL_BREAK             (0x17UL << ARM_USART_CONTROL_Pos)   ///< Continuous Break transmission; arg: 0=disabled, 1=enabled
#define ARM_USART_ABORT_SEND                (0x18UL << ARM_USART_CONTROL_Pos)   ///< Abort \ref ARM_USART::Send
#define ARM_USART_ABORT_RECEIVE             (0x19UL << ARM_USART_CONTROL_Pos)   ///< Abort \ref ARM_USART::Receive
#define ARM_USART_ABORT_TRANSFER            (0x1AUL << ARM_USART_CONTROL_Pos)   ///< Abort \ref ARM_USART::Transfer
/**@} USART Control Codes: Miscellaneous Controls */

/*----- USART Control Codes: Mode Parameters: Data Bits -----*/
/** \name USART Control Codes:  Mode Parameters: Data Bits */
/**@{*/
#define ARM_USART_DATA_BITS_Pos              8                                  ///< Bitfield position for __Control Codes:  Mode Parameters: Data Bits__
#define ARM_USART_DATA_BITS_Msk             (7UL << ARM_USART_DATA_BITS_Pos)    ///< Bitfield mask for __Control Codes:  Mode Parameters: Data Bits__
#define ARM_USART_DATA_BITS_5               (5UL << ARM_USART_DATA_BITS_Pos)    ///< 5 Data bits
#define ARM_USART_DATA_BITS_6               (6UL << ARM_USART_DATA_BITS_Pos)    ///< 6 Data bit
#define ARM_USART_DATA_BITS_7               (7UL << ARM_USART_DATA_BITS_Pos)    ///< 7 Data bits
#define ARM_USART_DATA_BITS_8               (0UL << ARM_USART_DATA_BITS_Pos)    ///< 8 Data bits (default)
#define ARM_USART_DATA_BITS_9               (1UL << ARM_USART_DATA_BITS_Pos)    ///< 9 Data bits
/**@} Mode Parameters: Data Bits */

/*----- USART Control Codes: Mode Parameters: Parity -----*/
/** \name USART Control Codes: Mode Parameters: Parity */
/**@{*/
#define ARM_USART_PARITY_Pos                 12                                 ///< Bitfield position for __Control Codes: Mode Parameters: Parity__
#define ARM_USART_PARITY_Msk                (3UL << ARM_USART_PARITY_Pos)       ///< Bitfield mask for __Control Codes: Mode Parameters: Parity__
#define ARM_USART_PARITY_NONE               (0UL << ARM_USART_PARITY_Pos)       ///< No Parity (default)
#define ARM_USART_PARITY_EVEN               (1UL << ARM_USART_PARITY_Pos)       ///< Even Parity
#define ARM_USART_PARITY_ODD                (2UL << ARM_USART_PARITY_Pos)       ///< Odd Parity
/**@}*/

/*----- USART Control Codes: Mode Parameters: Stop Bits -----*/
/** \name USART Control Codes:  Mode Parameters: Stop Bits */
/**@{*/
#define ARM_USART_STOP_BITS_Pos              14                                 ///< Bitfield position for __Control Codes: Mode Parameters: Stop Bits__
#define ARM_USART_STOP_BITS_Msk             (3UL << ARM_USART_STOP_BITS_Pos)    ///< Bitfield mask for __Control Codes: Mode Parameters: Stop Bits__
#define ARM_USART_STOP_BITS_1               (0UL << ARM_USART_STOP_BITS_Pos)    ///< 1 Stop bit (default)
#define ARM_USART_STOP_BITS_2               (1UL << ARM_USART_STOP_BITS_Pos)    ///< 2 Stop bits
#define ARM_USART_STOP_BITS_1_5             (2UL << ARM_USART_STOP_BITS_Pos)    ///< 1.5 Stop bits
#define ARM_USART_STOP_BITS_0_5             (3UL << ARM_USART_STOP_BITS_Pos)    ///< 0.5 Stop bits
/**@}*/

/*----- USART Control Codes: Mode Parameters: Flow Control -----*/
/** \name USART Control Codes:  Mode Parameters: Flow Control */
/**@{*/
#define ARM_USART_FLOW_CONTROL_Pos           16                                 ///< Bitfield position for __Control Codes: Mode Parameters: Flow Control__
#define ARM_USART_FLOW_CONTROL_Msk          (3UL << ARM_USART_FLOW_CONTROL_Pos) ///< Bitfield mask for __Control Codes: Mode Parameters: Flow Control__
#define ARM_USART_FLOW_CONTROL_NONE         (0UL << ARM_USART_FLOW_CONTROL_Pos) ///< No Flow Control (default)
#define ARM_USART_FLOW_CONTROL_RTS          (1UL << ARM_USART_FLOW_CONTROL_Pos) ///< RTS Flow Control
#define ARM_USART_FLOW_CONTROL_CTS          (2UL << ARM_USART_FLOW_CONTROL_Pos) ///< CTS Flow Control
#define ARM_USART_FLOW_CONTROL_RTS_CTS      (3UL << ARM_USART_FLOW_CONTROL_Pos) ///< RTS/CTS Flow Control
/**@}*/

/*----- USART Control Codes: Mode Parameters: Clock Polarity (Synchronous mode) -----*/
/** \name USART Control Codes:  Mode Parameters: Clock Polarity (Synchronous mode) */
/**@{*/
#define ARM_USART_CPOL_Pos                   18                                 ///< Bitfield position for __Control Codes: Mode Parameters: Clock Polarity__
#define ARM_USART_CPOL_Msk                  (1UL << ARM_USART_CPOL_Pos)         ///< Bitfield mask for __Control Codes: Mode Parameters: Clock Polarity__
#define ARM_USART_CPOL0                     (0UL << ARM_USART_CPOL_Pos)         ///< CPOL = 0 (default)
#define ARM_USART_CPOL1                     (1UL << ARM_USART_CPOL_Pos)         ///< CPOL = 1
/**@}*/

/*----- USART Control Codes: Mode Parameters: Clock Phase (Synchronous mode) -----*/
/** \name USART Control Codes:  Mode Parameters: Clock Phase (Synchronous mode) */
/**@{*/
#define ARM_USART_CPHA_Pos                   19                                 ///< Bitfield position for __Control Codes: Mode Parameters: Clock Phase__
#define ARM_USART_CPHA_Msk                  (1UL << ARM_USART_CPHA_Pos)         ///< Bitfield mask for __Control Codes: Mode Parameters: Clock Phase__
#define ARM_USART_CPHA0                     (0UL << ARM_USART_CPHA_Pos)         ///< CPHA = 0 (default)
#define ARM_USART_CPHA1                     (1UL << ARM_USART_CPHA_Pos)         ///< CPHA = 1
/**@} Clock Phase*/


/**@} cmsis_driver_usart_control_codes USART Control Codes */


/****** USART specific error codes *****/
/**
 * \addtogroup cmsis_driver_usart_return_codes
 */
/**@{*/
#define ARM_USART_ERROR_MODE                (ARM_DRIVER_ERROR_SPECIFIC - 1)     ///< Specified Mode not supported
#define ARM_USART_ERROR_BAUDRATE            (ARM_DRIVER_ERROR_SPECIFIC - 2)     ///< Specified baudrate not supported
#define ARM_USART_ERROR_DATA_BITS           (ARM_DRIVER_ERROR_SPECIFIC - 3)     ///< Specified number of Data bits not supported
#define ARM_USART_ERROR_PARITY              (ARM_DRIVER_ERROR_SPECIFIC - 4)     ///< Specified Parity not supported
#define ARM_USART_ERROR_STOP_BITS           (ARM_DRIVER_ERROR_SPECIFIC - 5)     ///< Specified number of Stop bits not supported
#define ARM_USART_ERROR_FLOW_CONTROL        (ARM_DRIVER_ERROR_SPECIFIC - 6)     ///< Specified Flow Control not supported
#define ARM_USART_ERROR_CPOL                (ARM_DRIVER_ERROR_SPECIFIC - 7)     ///< Specified Clock Polarity not supported
#define ARM_USART_ERROR_CPHA                (ARM_DRIVER_ERROR_SPECIFIC - 8)     ///< Specified Clock Phase not supported
/**@} cmsis_driver_usart_return_codes USART Return Codes */

/**
 * \addtogroup cmsis_driver_usart
 */
/**@{*/
/**
* \brief USART Status
*/
struct _ARM_USART_STATUS {
  uint32_t tx_busy          : 1;        ///< Transmitter busy flag
  uint32_t rx_busy          : 1;        ///< Receiver busy flag
  uint32_t tx_underflow     : 1;        ///< Transmit data underflow detected (cleared on start of next send operation)
  uint32_t rx_overflow      : 1;        ///< Receive data overflow detected (cleared on start of next receive operation)
  uint32_t rx_break         : 1;        ///< Break detected on receive (cleared on start of next receive operation)
  uint32_t rx_framing_error : 1;        ///< Framing error detected on receive (cleared on start of next receive operation)
  uint32_t rx_parity_error  : 1;        ///< Parity error detected on receive (cleared on start of next receive operation)
};


/**
* \brief USART Modem Control
*/
enum _ARM_USART_MODEM_CONTROL {
  ARM_USART_RTS_CLEAR,                  ///< Deactivate RTS
  ARM_USART_RTS_SET,                    ///< Activate RTS
  ARM_USART_DTR_CLEAR,                  ///< Deactivate DTR
  ARM_USART_DTR_SET                     ///< Activate DTR
};


/**
* \brief USART Modem Status
*/
struct _ARM_USART_MODEM_STATUS {
  uint32_t cts : 1;                     ///< CTS state: 1=Active, 0=Inactive
  uint32_t dsr : 1;                     ///< DSR state: 1=Active, 0=Inactive
  uint32_t dcd : 1;                     ///< DCD state: 1=Active, 0=Inactive
  uint32_t ri  : 1;                     ///< RI  state: 1=Active, 0=Inactive
};


/**@} cmsis_driver_usart */

/**
 * \addtogroup cmsis_driver_usart
 */
/**@{*/
typedef struct _ARM_USART_STATUS ARM_USART_STATUS;
typedef struct _ARM_USART_MODEM_STATUS ARM_USART_MODEM_STATUS;
typedef enum _ARM_USART_MODEM_CONTROL ARM_USART_MODEM_CONTROL;
/**@} cmsis_driver_usart */


/****** USART Event *****/
/**
 * \addtogroup cmsis_driver_usart_std_events USART Event Codes
 */
/**@{*/
#define ARM_USART_EVENT_SEND_COMPLETE       (1UL << 0)  ///< Send completed; however USART may still transmit data
#define ARM_USART_EVENT_RECEIVE_COMPLETE    (1UL << 1)  ///< Receive completed
#define ARM_USART_EVENT_TRANSFER_COMPLETE   (1UL << 2)  ///< Transfer completed
#define ARM_USART_EVENT_TX_COMPLETE         (1UL << 3)  ///< Transmit completed (optional)
#define ARM_USART_EVENT_TX_UNDERFLOW        (1UL << 4)  ///< Transmit data not available (Synchronous Slave)
#define ARM_USART_EVENT_RX_OVERFLOW         (1UL << 5)  ///< Receive data overflow
#define ARM_USART_EVENT_RX_TIMEOUT          (1UL << 6)  ///< Receive character timeout (optional)
#define ARM_USART_EVENT_RX_BREAK            (1UL << 7)  ///< Break detected on receive
#define ARM_USART_EVENT_RX_FRAMING_ERROR    (1UL << 8)  ///< Framing error detected on receive
#define ARM_USART_EVENT_RX_PARITY_ERROR     (1UL << 9)  ///< Parity error detected on receive
#define ARM_USART_EVENT_CTS                 (1UL << 10) ///< CTS state changed (optional)
#define ARM_USART_EVENT_DSR                 (1UL << 11) ///< DSR state changed (optional)
#define ARM_USART_EVENT_DCD                 (1UL << 12) ///< DCD state changed (optional)
#define ARM_USART_EVENT_RI                  (1UL << 13) ///< RI  state changed (optional)
#define ARM_USART_EVENT_RX_NOISE_ERROR      (1UL << 14) ///Noise error
/**@} cmsis_driver_usart_std_events USART Event Codes */



/**
 * \addtogroup cmsis_driver_usart
 */
/**@{*/

/**
 *\brief USART Device Driver Capabilities.
 */
struct _ARM_USART_CAPABILITIES {
  uint32_t asynchronous       : 1;      ///< supports UART (Asynchronous) mode 
  uint32_t synchronous_master : 1;      ///< supports Synchronous Master mode
  uint32_t synchronous_slave  : 1;      ///< supports Synchronous Slave mode
  uint32_t single_wire        : 1;      ///< supports UART Single-wire mode
  uint32_t irda               : 1;      ///< supports UART IrDA mode
  uint32_t smart_card         : 1;      ///< supports UART Smart Card mode
  uint32_t smart_card_clock   : 1;      ///< Smart Card Clock generator available
  uint32_t flow_control_rts   : 1;      ///< RTS Flow Control available
  uint32_t flow_control_cts   : 1;      ///< CTS Flow Control available
  uint32_t event_tx_complete  : 1;      ///< Transmit completed event: \ref ARM_USART_EVENT_TX_COMPLETE
  uint32_t event_rx_timeout   : 1;      ///< Signal receive character timeout event: \ref ARM_USART_EVENT_RX_TIMEOUT
  uint32_t rts                : 1;      ///< RTS Line: 0=not available, 1=available
  uint32_t cts                : 1;      ///< CTS Line: 0=not available, 1=available
  uint32_t dtr                : 1;      ///< DTR Line: 0=not available, 1=available
  uint32_t dsr                : 1;      ///< DSR Line: 0=not available, 1=available
  uint32_t dcd                : 1;      ///< DCD Line: 0=not available, 1=available
  uint32_t ri                 : 1;      ///< RI Line: 0=not available, 1=available
  uint32_t event_cts          : 1;      ///< Signal CTS change event: \ref ARM_USART_EVENT_CTS
  uint32_t event_dsr          : 1;      ///< Signal DSR change event: \ref ARM_USART_EVENT_DSR
  uint32_t event_dcd          : 1;      ///< Signal DCD change event: \ref ARM_USART_EVENT_DCD
  uint32_t event_ri           : 1;      ///< Signal RI change event: \ref ARM_USART_EVENT_RI
};

typedef struct _ARM_USART_CAPABILITIES ARM_USART_CAPABILITIES;

/**@} cmsis_driver_usart */

#endif /* __DRIVER_USART_H */
