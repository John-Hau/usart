/*
 *  $HeadURL: $
 *
 *  $Date: $
 *  $Author: $
 */

#ifndef BAPI_UART_H_
#define BAPI_UART_H_
/**
 * \file
 * \brief
 * This file declares the UART related board API interface functions.
 * */
#include <stddef.h>
#include <stdint.h>

#include "baseplate.h"
#include "utils/utils.h"
#include "boards/board-api/bapi_atomic.h"


/**
 * \ingroup bapi_uart
 * \brief
 * The transmission mode in which the UART is operating. For driving a console terminal, the
 * bapi_uart_E_TxMode_CRLF mode may be the right choice. Otherwise use bapi_uart_E_TxMode_Transparent.
 */
typedef enum bapi_uart_E_TxMode_ {
   bapi_uart_E_TxMode_Transparent = 0 /**< The message is transmitted unmodified. */
  ,bapi_uart_E_TxMode_CRLF = 1        /**< For each CR character in the message, an extra LF character
                                         is sent afterwards. For each LF character in the message,
                                         an CR character is sent up front. */
  ,bapi_uart_E_TxMode_TxOff = 2
} bapi_uart_E_TxMode;


/**
 * \ingroup bapi_uart
 * \def BAPI_HAS_USART
 * \brief This macro can be used to check at compile time how many USARTs the board has.
 * If the value of the macro is 0, the board doesn't support USARTs.
 * */

/**
 * \enum bapi_E_UartIndex_
 * \ingroup bapi_uart
 * \brief
 * This enumeration abstracts the identification of the UARTs that are supported by the board.
 * All board API functions/structures that need to refer to an UART will only deal with this enumeration type.
 *
 * Type safety is an important criteria to avoid bugs. So the decision was taken to use an enumeration type
 * rather than a simple integer data type to identify a UART.
 * The definition of this enumeration is board specific, because different boards have different numbers of UARTs
 * and different designators. The implementation of this enumeration will have generic designators for the UARTs
 * as well as vendor specific ones. The generic designators follow the scheme bapi_E_UartX where X starts
 * from 0 to (number of usable UARTs minus 1). The enumeration also provides the number of usable UARTs by the
 * designator bapi_E_UartCount
*/
#if defined (EM_DK3750)
	#include "boards/EM_DK3750/bapi_uart_EM_DK3750.h"
#elif defined (FS_IRMFCU)
  #include "boards/FS_IRMFCU/bapi_uart_FS_IRMFCU.h"
#elif defined (FS_IRMLC)
  #include "boards/FS_IRMLC/bapi_uart_FS_IRMLC.h"
#elif defined (FS_IRMCT)
  #include "boards/FS_IRMCT/bapi_uart_FS_IRMCT.h"
#elif defined (FS_IRM_BIO)
  #include "boards/FS_IRM_BIO/bapi_uart_FS_IRM_BIO.h"
#elif defined (FS_IRMLC_KL17Z)
  #include "boards/FS_IRMLC-KL17Z/bapi_uart_FS_IRMLC-KL17Z.h"
#elif defined (FS_FRDM_KL46Z)
  #include "boards/FS_FRDM-KL46Z/bapi_uart_FS_FRDM-KL46Z.h"
#elif defined (FS_FRDM_K64F)
  #include "boards/FS_FRDM-K64F/bapi_uart_FS_FRDM-K64F.h"
#elif defined (FS_FRDM_K66F)
  #include "boards/FS_FRDM-K66F/bapi_uart_FS_FRDM-K66F.h"
#elif defined (FS_IRMCT)
  #include "boards/FS_IRMCT/bapi_uart_FS_IRMCT.h"
#elif defined (CCS_CVAHU)
  #include "boards/CCS_CVAHU/bapi_uart_CCS_CVAHU.h"
#elif defined (FS_IRMFCU_BL)
  #include "boards/FS_IRMFCU_BL/bapi_uart_FS_IRMFCU_BL.h"
#elif defined (FS_IRMVAV)
  #include "boards/FS_IRMVAV/bapi_uart_FS_IRMVAV.h"
#elif defined (FS_IMXRTEVAL)
	#include "boards/FS_IMXRTEVAL/bapi_uart_FS_IMXRTEVAL.h"
#elif defined (FS_BEATS_IO)
	#include "boards/FS_BEATS_IO/bapi_uart_FS_BEATS_IO.h"
#elif defined (FS_IMXRT_TSTAT)
  #include "boards/FS_IMXRT_TSTAT/bapi_uart_FS_IMXRT_TSTAT.h"
#elif defined (FS_IPVAV)
	#include "boards/FS_IPVAV/bapi_uart_FS_IPVAV.h"
#elif defined (FS_SNAP_ON_IO)
	#include "boards/FS_SNAP_ON_IO/bapi_uart_FS_SNAP_ON_IO.h"
#else
	#error "Fatal Error: Unknown hardware board."
#endif

#include "boards/cmsis/Driver_USART.h"

/**
 * \ingroup bapi_uart
 * \brief The CMSIS Driver USART modes as type safe enumeration.
 */
typedef enum bapi_E_UartMode_ {
  arm_USART_MODE_UNINITIALIZED = 0,
  arm_USART_MODE_ASYNCHRONOUS = ARM_USART_MODE_ASYNCHRONOUS,
  arm_USART_MODE_SYNCHRONOUS_MASTER = ARM_USART_MODE_SYNCHRONOUS_MASTER,
  arm_USART_MODE_SYNCHRONOUS_SLAVE = ARM_USART_MODE_SYNCHRONOUS_SLAVE,
  arm_USART_MODE_SINGLE_WIRE = ARM_USART_MODE_SINGLE_WIRE,
  arm_USART_MODE_IRDA = ARM_USART_MODE_IRDA,
  arm_USART_MODE_SMART_CARD = ARM_USART_MODE_SMART_CARD,

  arm_USART_MODE_NONE

} bapi_E_UartMode;

typedef enum bapi_E_InterfaceFlag_ {
   bapi_E_InvalidInterfaceFlag = -1
  ,bapi_E_EnableRts               /**< Switch the RTS manually. */
  ,bapi_E_RS485_EnableTransmitter /**< Switch the Transmitter for UARTS with RS485 hardware. */
  ,bapi_E_BLE_EnablePower         /**< Switch the Power for BLE Module */
  ,bapi_E_Sylk_EnablePower        /**< Switch the Power for UARTS with Sylk hardware. */
} bapi_E_InterfaceFlag;


/**
 * \ingroup bapi_uart
 * \brief
 * GET TRANSMISSION STATE callback function type that the UART Transmission ISR calls during
 * transmission to get the next character to send. May be NULL.
 *
 * */
typedef struct bapi_uart_TransmissionState* (*bapi_uart_getTransmissionState_ISRCallback_t)(
  const enum bapi_E_UartIndex_ uartIndex /** The source UART that has invoked the ISR. */
  );

/**
 * \ingroup bapi_uart
 * \brief
 * TRANSMISSION COMPLETE callback function type that the UART Transmission calls when
 *  transmission of a message is completed. May be NULL.
 *
 *  The ISR that calls this function should not evaluate the return value and always assume instead,
 *  that there might be a new transmission state setup after this call, look at it and continue
 *  sending if this is the case.
 *
 *
 * \return N/A.
 * */
typedef void (*bapi_uart_msgTransmissionComplete_ISRCallback_t)(
  struct bapi_uart_TransmissionState* transmissionState, /**< The transmission state to be passed
                                                          * back to the UART transmission ISR. */
  uint32_t event /**< ARM_USART_EVENT_SEND_COMPLETE in case that all bytes have been moved to the
                  * UART send register or UART FIFO. ARM_USART_EVENT_TX_COMPLETE in case that the
                  * last byte was physically transmitted. */
  );

/**
 * \ingroup bapi_uart
 * brief\
 * The two UART Transmission callback functions: TRANSMISSION COMPLETE and GET TRANSMISSION STATE
 * can be carried in this structure.
 *
 * Introduced for the return value of function
 *  bapi_uart_setMsgTransmission_ISRCallbacks(bapi_uart_msgTransmissionComplete_ISRCallback_t, bapi_uart_getTransmissionState_ISRCallback_t)
 */
struct bapi_uart_Tx_ISRCallbacks {
  bapi_uart_getTransmissionState_ISRCallback_t   m_getTransmissionState;
  bapi_uart_msgTransmissionComplete_ISRCallback_t m_msgTransmissionCompleteHandler;
};

typedef uint16_t bapi_uart_MaxFrameSize_t;

/**
 * \ingroup bapi_uart
 * \brief
 * DATA RECEIVED callback function type that the RX BUFFER FULL ISR or RX ERROR ISR calls when:
 *   1) It has received zero or more character(s). In that case the errorEvent
 *     parameter is zero, and the rx_chars and count parameter give info about the received chars.
 *   2) One or multiple Rx receive error(s) occured. In that case the errorEvent
 *     parameter represents the notification mask of the error events. Each error event
 *     is coded in a separate bit and therefore it is possible to signal multiple
 *     error events in the event call back function.
 *
 * @param errorEvents[in] Zero, in case that the function is called by the RX BUFFER FULL ISR. Otherwise
 *     multiple error flags as follows:
 *   -) #ARM_USART_EVENT_RX_OVERFLOW
 *   -) #ARM_USART_EVENT_RX_TIMEOUT
 *   -) #ARM_USART_EVENT_RX_BREAK
 *   -) #ARM_USART_EVENT_RX_FRAMING_ERROR
 *   -) #ARM_USART_EVENT_RX_PARITY_ERROR
 *
 * @return The number of characters that the callback function did process.
 */
typedef bapi_uart_MaxFrameSize_t (*bapi_uart_dataReceived_ISRCallback_t)(
  const enum bapi_E_UartIndex_ uartIndex,  /**< The source UART of the received character(s). */
  const uint32_t errorEvents,
  const uint8_t rx_chars[],                /**< The received character(s) */
  bapi_uart_MaxFrameSize_t count           /**< The number of received characters */
  );


/**
 * \ingroup bapi_uart
 * \brief
 * Identifies an UART Interrupt type. Will be used to enable disable particular UART interrupts.
 */
enum bapi_uart_E_UartIrqType {
   bapi_uart_IRQT_RX = 0      /**< RECEIVE BUFFER FULL and RECEIVE ERROR */
  ,bapi_uart_IRQT_TX          /**< TRANSMISSION BUFFER EMPTY */
//#ifndef BAPI_DISABLE_UART_ERROR_HANDLING /* Error handling can be disabled by defining BAPI_DISABLE_UART_ERROR_HANDLING */
//  ,bapi_uart_IRQT_TX_COMPLETE /**< Bits are physically transmitted */
//#endif
  ,bapi_uart_IRQT_Count
};

/**
 * \ingroup bapi_uart
 * \brief The data type that will carry a FIFO size in data words. The size of a data word
 *   dependens on the UART setup. A data word can be 8 or 9 or more bits of size.
 */
typedef uint16_t bapi_uart_fifo_size_t;


/**
 * \ingroup bapi_uart
 * \brief Access structure of the bapi UART API. This structure allows to implement the bapi_uart api for
 *   different UART types and even virtual UARTs as for the USB CDC ACM device class.
 *
 * \warning Don't change the order of the members carelessly, because there are const static variables, that
 *   assume exactly this order.
 */
struct _bapi_uart_interface {

  /** \copydoc   bapi_uart_setMsgTransmission_ISRCallbacks(enum bapi_E_UartIndex_ uartIndex,
   *    bapi_uart_msgTransmissionComplete_ISRCallback_t, bapi_uart_getTransmissionState_ISRCallback_t);
   */
  struct bapi_uart_Tx_ISRCallbacks (*setMsgTransmission_ISRCallbacks)(const enum bapi_E_UartIndex_ uartIndex
    , bapi_uart_msgTransmissionComplete_ISRCallback_t msgTransmitted_ISRCallback
    , bapi_uart_getTransmissionState_ISRCallback_t getTransmissionState
    );

  /**
   * \copydoc bapi_uart_setDataReceived_ISRCallback(enum bapi_E_UartIndex_,
   *    bapi_uart_dataReceived_ISRCallback_t)
   */
  bapi_uart_dataReceived_ISRCallback_t (*setDataReceived_ISRCallback)(const enum bapi_E_UartIndex_ uartIndex
    , bapi_uart_dataReceived_ISRCallback_t rxIrqHandler);

  /** \copydoc bapi_uart_configure(const enum bapi_E_UartIndex_, uint32_t,  uint32_t) */
  uint32_t (*configure)(const enum bapi_E_UartIndex_ uartIndex, uint32_t baudrate, uint32_t armUsartControl);

  /** \copydoc bapi_uart_unconfigure(const enum bapi_E_UartIndex_) */
  void (*unconfigure)(const enum bapi_E_UartIndex_ uartIndex);

  /** \copydoc bapi_uart_getMode(const enum bapi_E_UartIndex_) */
  enum bapi_E_UartMode_ (*getMode)(const enum bapi_E_UartIndex_ uartIndex);

  /** \copydoc bapi_uart_enterCritical(const enum bapi_E_UartIndex_, enum bapi_uart_E_UartIrqType irq_type) */
  void (*uart_enterCritical)(const enum bapi_E_UartIndex_ uartIndex, enum bapi_uart_E_UartIrqType irqType);

  /** \copydoc bapi_uart_exitCritical(const enum bapi_E_UartIndex_, enum bapi_uart_E_UartIrqType ) */
  void (*uart_exitCritical)(const enum bapi_E_UartIndex_ uartIndex, enum bapi_uart_E_UartIrqType irqType);

  /** \copydoc bapi_uart_startTx(const enum bapi_E_UartIndex_) */
  void (*startTx)(const enum bapi_E_UartIndex_ uartIndex);

  /** \copydoc bapi_uart_setInterfaceFlag(const enum bapi_E_UartIndex_, enum bapi_E_InterfaceFlag_, uint32_t) */
  bool (*setInterfaceFlag)(const enum bapi_E_UartIndex_ uartIndex, enum bapi_E_InterfaceFlag_ interfaceFlag
    , uint32_t value);

  /** \copydoc bapi_uart_getInterfaceFlag(const enum bapi_E_UartIndex_, enum bapi_E_InterfaceFlag_, uint32_t* value) */
  uint32_t (*getInterfaceFlag)(const enum bapi_E_UartIndex_ uartIndex, enum bapi_E_InterfaceFlag_ interfaceFlag
    , uint32_t* value );

  /** \copydoc bapi_uart_setLoopCmd(const enum bapi_E_UartIndex_, bool bEnable) */
  bool (*setLoopCmd)(const enum bapi_E_UartIndex_ uartIndex, bool bEnable);

  /** \copydoc bapi_uart_enableTransmitter(const enum bapi_E_UartIndex_) */
  uint32_t (*enableTransmitter)(const enum bapi_E_UartIndex_ uartIndex);

  /** \copydoc bapi_uart_disableTransmitter(const enum bapi_E_UartIndex_) */
  uint32_t (*disableTransmitter)(const enum bapi_E_UartIndex_ uartIndex);

  /** \copydoc bapi_uart_enableReceiver(const enum bapi_E_UartIndex_) */
  uint32_t (*enableReceiver)(const enum bapi_E_UartIndex_ uartIndex);

  /** \copydoc bapi_uart_disableReceiver(const enum bapi_E_UartIndex_) */
  uint32_t (*disableReceiver)(const enum bapi_E_UartIndex_ uartIndex);

  /** \copydoc bapi_uart_flushTxFifo(const enum bapi_E_UartIndex_) */
  void (*flushTxFifo)(const enum bapi_E_UartIndex_ uartIndex);

  /** \copydoc bapi_uart_flushRxFifo(const enum bapi_E_UartIndex_) */
  void (*flushRxFifo)(const enum bapi_E_UartIndex_ uartIndex);

  /** \copydoc bapi_uart_setTxFifo(const enum bapi_E_UartIndex_, bapi_uart_fifo_size_t fifoSize) */
  bapi_uart_fifo_size_t (*setTxFifo)(const enum bapi_E_UartIndex_ uartIndex, bapi_uart_fifo_size_t fifoSize);

  /** \copydoc bapi_uart_setRxFifo(const enum bapi_E_UartIndex_, bapi_uart_fifo_size_t fifoSize) */
  bapi_uart_fifo_size_t (*setRxFifo)(const enum bapi_E_UartIndex_ uartIndex, bapi_uart_fifo_size_t fifoSize);

  /** \copydoc bapi_uart_getBaudrate(const enum bapi_E_UartIndex_) */
  uint32_t (*getBaudrate)(const enum bapi_E_UartIndex_ uartIndex);

  /** \copydoc bapi_uart_setBaudrate(const enum bapi_E_UartIndex_) */
   bool (*setBaudrate)(const enum bapi_E_UartIndex_ uartIndex, uint32_t baudRate);
};


/**
 * \ingroup _bapi_uart
 * This function needs to be implemented by the MCU vendor specific UART module.
 *  @return pointer to the uart interface that is to be used for the uartIndex.
 */
C_FUNC const struct _bapi_uart_interface* _bapi_uart_getUartInterface(enum bapi_E_UartIndex_ uartIndex);


/**
 * \ingroup bapi_uart
 * \brief In case that the UART uses a transmission FIFO, this function flushes
 *   all characters from the transmission FIFO. Otherwise, it will do nothing.
 */
C_INLINE void bapi_uart_flushTxFifo(const enum bapi_E_UartIndex_ uartIndex) {
  const struct _bapi_uart_interface* uartInterface = _bapi_uart_getUartInterface(uartIndex);
  uartInterface->flushTxFifo(uartIndex);
}

/**
 * \ingroup bapi_uart
 * \brief In case that the UART uses a receive FIFO, this function flushes
 *   all characters from the receive FIFO. Otherwise, it will do nothing.
 */
C_INLINE void bapi_uart_flushRxFifo(const enum bapi_E_UartIndex_ uartIndex) {
  const struct _bapi_uart_interface* uartInterface = _bapi_uart_getUartInterface(uartIndex);
  uartInterface->flushRxFifo(uartIndex);
}

/**
 * \ingroup bapi_uart
 * \brief In case that the UART has a transmission FIFO, this function will set the Tx fifoSize.
 *
 * \warning The real selected FIFO Size may be different to the size required by _fifoSize_ due
 * to the available FIFO size granularity. The function tries to match _fifoSize_ exactly.
 * Otherwise it will pick next higher available FIFO size. The maximum
 * available FIFO size will be selected, if _fifoSize_ exceeds this maximum boundary.
 *
 * @param uartIndex The UART for which to set the FIFO.
 * @param fifoSize  The required FIFO size in data words (Note that a data workd may be different to 8 bit).
 *  Passing a value <= 1 will set the FIFO size to 1.
 *  \note There will always be a single transmit buffer, so the FIFO size cannot be 0.
 * @return The real selected FIFO size.
 *
 */
C_INLINE bapi_uart_fifo_size_t bapi_uart_setTxFifo(const enum bapi_E_UartIndex_ uartIndex, bapi_uart_fifo_size_t fifoSize) {
  const struct _bapi_uart_interface* uartInterface = _bapi_uart_getUartInterface(uartIndex);
  return uartInterface->setTxFifo(uartIndex, fifoSize);
}

/**
 * \ingroup bapi_uart
 * \brief In case that the UART has a receive FIFO, this function will set the Rx fifoSize.
 *
 * \warning The real selected FIFO Size may be different to the size required by _fifoSize_ due
 * to the available FIFO size granularity. The function tries to match _fifoSize_ exactly.
 * Otherwise it will pick next higher available FIFO size. The maximum
 * available FIFO size will be selected, if _fifoSize_ exceeds this maximum boundary.
 *
 * @param uartIndex The UART for which to set the FIFO.
 * @param fifoSize  The required FIFO size in data words (Note that a data workd may be different to 8 bit).
 *  Passing a value <= 1 will set the FIFO size to 1.
 *  \note There will always be a single receive buffer, so the FIFO size cannot be 0.
 * @return The real selected FIFO size.
 *
 */
C_INLINE bapi_uart_fifo_size_t bapi_uart_setRxFifo(const enum bapi_E_UartIndex_ uartIndex, bapi_uart_fifo_size_t fifoSize) {
  const struct _bapi_uart_interface* uartInterface = _bapi_uart_getUartInterface(uartIndex);
  return uartInterface->setRxFifo(uartIndex, fifoSize);
}

/**
 * \ingroup bapi_uart
 * \brief get the baud rate at which UART interface is configured
 *
 * @param uartIndex The UART for which configured baud rate is queried
 * @return Baud rate.
 *
 */
C_INLINE uint32_t bapi_uart_getBaudrate(const enum bapi_E_UartIndex_ uartIndex) {
  const struct _bapi_uart_interface* uartInterface = _bapi_uart_getUartInterface(uartIndex);
  return uartInterface->getBaudrate(uartIndex);
}

/**
 * \ingroup bapi_uart
 * \brief get the baud rate at which UART interface is configured
 *
 * @param uartIndex The UART for which configured baud rate is queried
 * @param baudRate
 * @return None.
 *
 */
C_INLINE uint32_t bapi_uart_setBaudrate(const enum bapi_E_UartIndex_ uartIndex, uint32_t baudRate) {
  const struct _bapi_uart_interface* uartInterface = _bapi_uart_getUartInterface(uartIndex);
  return uartInterface->setBaudrate(uartIndex, baudRate);
}

/**
 * \ingroup bapi_uart
 * \brief set a UART associated interface Flag
 */
C_INLINE bool bapi_uart_setInterfaceFlag(
   const enum bapi_E_UartIndex_ uartIndex    /**< [in] The UART to be configured */
  ,enum bapi_E_InterfaceFlag_ interfaceFlag  /**< [in] The Interface Flag to be set */
  ,uint32_t value                            /**< [in] The Interface Flag value */
  ) {
  const struct _bapi_uart_interface* uartInterface = _bapi_uart_getUartInterface(uartIndex);
  return uartInterface->setInterfaceFlag(uartIndex, interfaceFlag, value);
}

/**
 * \ingroup bapi_uart
 * \brief retrieve a UART associated interface Flag
 */
C_INLINE bool bapi_uart_getInterfaceFlag(
  const enum bapi_E_UartIndex_ uartIndex      /**< [in]  The UART to be configured */
  , enum bapi_E_InterfaceFlag_ interfaceFlag  /**< [in]  The Interface Flag to get */
  , uint32_t* value                           /**< [out] The Interface Flag value */
  ) {
  const struct _bapi_uart_interface* uartInterface = _bapi_uart_getUartInterface(uartIndex);
  return uartInterface->getInterfaceFlag(uartIndex, interfaceFlag, value);
}


C_DECL const struct _ARM_USART_CAPABILITIES _bapi_uartCapabilities[bapi_E_UartCount];

/**
 *
 */
C_INLINE const struct _ARM_USART_CAPABILITIES* bapi_uart_getCapabilities(
  const enum bapi_E_UartIndex_ uartIndex    /**< [in] The UART to be configured */
  ) {
  ASSERT(uartIndex < bapi_E_UartCount);
  return &_bapi_uartCapabilities[uartIndex];
}

/**
 * \ingroup bapi_uart
 * \brief Internally Connect / Disconnect Rx with Tx.
 * \return true, if successful, false if not supported for this UART.
 * */
C_INLINE bool bapi_uart_setLoopCmd(
  const enum bapi_E_UartIndex_ uartIndex /**< [in] The UART to be configured */
  , bool bEnable
  ) {
  const struct _bapi_uart_interface* uartInterface = _bapi_uart_getUartInterface(uartIndex);
  return uartInterface->setLoopCmd(uartIndex, bEnable);
}

/**
 * \ingroup bapi_uart
 * \brief Will configure an UART to operate in asynchronous mode.
 * Any ongoing transmission will be interrupted immediately.
 *
 * \return ARM_DRIVER_OK if successful, otherwise one of the following:
 *    - ARM_DRIVER_ERROR_PARAMETER   in case a Control Code that is only valid for synchronous mode was given.
 *    - ARM_USART_ERROR_MODE         in case the demanded mode is not supported.
 *    - ARM_USART_ERROR_DATA_BITS    in case the demanded data bits is not supported.
 *    - ARM_USART_ERROR_PARITY       in case the demanded parity mode is not supported.
 *    - ARM_USART_ERROR_STOP_BITS    in case the demanded stop bits is not supported.
 *    - ARM_USART_ERROR_FLOW_CONTROL in case the demanded flow control is not supported.
 * see also __return value __ of function \htmlinclude cmsis_driver_usart_api_ARM_USART_Control.html
 */
C_INLINE uint32_t bapi_uart_configure(
  const enum bapi_E_UartIndex_ uartIndex,   /**< [in] The UART to be configured */
  uint32_t baudrate,                        /**< [in] The baud rate at which the UART shall operate */
  uint32_t armUsartControl                  /**< [in] armUsartControl (see also parameter __control__ of function \htmlinclude cmsis_driver_usart_api_ARM_USART_Control.html)
                                             * A bit combination of:
                                             * - USART Control Codes: Mode
                                             *    + ARM_USART_MODE_ASYNCHRONOUS
                                             *    + ARM_USART_MODE_SYNCHRONOUS_MASTER
                                             *    + ARM_USART_MODE_SYNCHRONOUS_SLAVE
                                             *    + ARM_USART_MODE_SINGLE_WIRE
                                             *    + ARM_USART_MODE_IRDA
                                             *    + ARM_USART_MODE_SMART_CARD
                                             * - USART Control Codes: Mode Parameters: Data Bits, see ARM_USART_FLOW_CONTROL_Msk
                                             * - USART Control Codes: Mode Parameters: Parity, see ARM_USART_PARITY_Msk
                                             * - USART Control Codes: Mode Parameters: Stop Bits, see ARM_USART_STOP_BITS_Msk
                                             * - USART Control Codes: Mode Parameters: Flow Control, see ARM_USART_FLOW_CONTROL_Msk
                                             */
  ){
  const struct _bapi_uart_interface* uartInterface = _bapi_uart_getUartInterface(uartIndex);
  return uartInterface->configure(uartIndex, baudrate, armUsartControl);
}

/**
 * \ingroup bapi_uart
 * \brief Disable Rx and Tx of a UART and sets it into non operational mode.
 *
 * \sa bapi_uart_configure(const enum bapi_E_UartIndex_, uint32_t, uint32_t).
 */
C_INLINE void bapi_uart_unconfigure(
  const enum bapi_E_UartIndex_ uartIndex    /**< [in] The UART to be unconfigured */
  ) {
  const struct _bapi_uart_interface* uartInterface = _bapi_uart_getUartInterface(uartIndex);
  uartInterface->unconfigure(uartIndex);
}

/**
 * \ingroup bapi_uart
 * \brief retrieve the UART mode as per enum bapi_E_UartMode_.
 *
 * \return the current UART mode
 */
C_INLINE enum bapi_E_UartMode_ bapi_uart_getMode(
  const enum bapi_E_UartIndex_ uartIndex    /**< [in] The UART for which to retrieve the information */
  ) {
  const struct _bapi_uart_interface* uartInterface = _bapi_uart_getUartInterface(uartIndex);
  return uartInterface->getMode(uartIndex);
}

/**
 * \ingroup bapi_uart
 * \brief Increments the transmitter-disable-nesting counter and will physically disable
 *   the transmitter when there is a nesting counter transition from 0 to 1.
 *
 * Should only be called when the UART is configured. Unconfiguring a configured UART will set
 * the nesting counter to 1 and physically disable the transmitter.
 * When the function is called for an unconfigured UART, the nesting counter sticks as 1;
 *
 *
 * \return The current nesting counter.
 */
C_INLINE uint8_t bapi_uart_disableTransmitter(
  const enum bapi_E_UartIndex_ uartIndex    /**< [in] The UART for which to retrieve the information */
  ) {
  const struct _bapi_uart_interface* uartInterface = _bapi_uart_getUartInterface(uartIndex);
  return uartInterface->disableTransmitter(uartIndex);
}

/**
 * \ingroup bapi_uart
 * \brief Decrements the disable-transmitter nesting counter and will physically enable
 *   the transmitter when there is a nesting counter transition from 1 to 0.
 *
 * Should only be called when the UART is configured. Configuring an unconfigured UART will set
 * the nesting counter to 0 and physically enable the transmitter.
 * When the function is called for an unconfigured UART, the nesting counter sticks as 1;
 *
 * \return The current nesting counter. If zero, the transmitter is physically enabled.
 */
C_INLINE uint8_t bapi_uart_enableTransmitter(
  const enum bapi_E_UartIndex_ uartIndex    /**< [in] The UART for which to retrieve the information */
  ) {
  const struct _bapi_uart_interface* uartInterface = _bapi_uart_getUartInterface(uartIndex);
  return uartInterface->enableTransmitter(uartIndex);
}

/**
 * \ingroup bapi_uart
 * \brief Increments the receiver-disable-nesting counter and will physically disable
 *   the receiver when there is a nesting counter transition from 0 to 1.
 *
 * Should only be called when the UART is configured. Unconfiguring a configured UART will set
 * the nesting counter to 1 and physically disable the receiver.
 * When the function is called for an unconfigured UART, the nesting counter sticks as 1;
 *
 * \return The current nesting counter.
 */
C_INLINE uint8_t bapi_uart_disableReceiver(
  const enum bapi_E_UartIndex_ uartIndex    /**< [in] The UART for which to retrieve the information */
  ) {
  const struct _bapi_uart_interface* uartInterface = _bapi_uart_getUartInterface(uartIndex);
  return uartInterface->disableReceiver(uartIndex);
}

/**
 * \ingroup bapi_uart
 * \brief Decrements the disable-receiver nesting counter and will physically enable
 *   the receiver when there is a nesting counter transition from 1 to 0.
 *
 * Should only be called when the UART is configured. Configuring an unconfigured UART will set
 * the nesting counter to 0 and physically enable the receiver.
 * When the function is called for an unconfigured UART, the nesting counter sticks as 1;
 *
 * \return The current nesting counter. If zero, the receiver is physically enabled.
 */
C_INLINE uint8_t bapi_uart_enableReceiver(
  const enum bapi_E_UartIndex_ uartIndex    /**< [in] The UART for which to retrieve the information */
  ) {
  const struct _bapi_uart_interface* uartInterface = _bapi_uart_getUartInterface(uartIndex);
  return uartInterface->enableReceiver(uartIndex);
}

/**
 * \ingroup bapi_uart
 * \brief
 * A short hand type definition for enum bapi_E_UartIndex_
 * */
typedef enum bapi_E_UartIndex_ bapi_E_UartIndex;


/**
 * \ingroup _bapi_uart
 * \brief
 * This is the internal state of the transmission used to perform the transmission modes of enum bapi_uart_E_TxMode_.
 */
enum _uart_E_TransmissionState
{
   bapi_uart_E_TS_Normal  = 0
  ,bapi_uart_E_TS_ExtraCR = 1
};



/**
 * \ingroup bapi_uart
 * \brief
 * The transmission state of an individual UART.
 *
 */
struct bapi_uart_TransmissionState {
  bapi_E_UartIndex m_uartIndex;         /**< The UART to which this transmission state belongs */
  bapi_uart_MaxFrameSize_t m_remainingBytes;  /**< The number of remaining bytes to send */

  // TODO: Support 9 bit data word transmission
  const char *m_byteToSend;             /**< Pointer to the next byte in the message to send.
                                         * <em><b>Important:</b></em> A NULL value indicates, that there
                                         * is no message under transmission.
                                         * */
  enum _uart_E_TransmissionState state; /**< If next byte to send is an extra LF character */
  bapi_uart_E_TxMode mode;              /**< The transmission mode */
};

/**
 * \ingroup bapi_uart
 * \brief
 * Initializes the transmission state structure so that transmission will start at the first byte of the
 * message.
 *
 */
C_INLINE void bapi_uart_init_TransmissionState(
  struct bapi_uart_TransmissionState* transmissionState,
  const void* mem, bapi_uart_MaxFrameSize_t len, bapi_uart_E_TxMode mode
  ) {
  /* Set "inUse" first */
  atomic_Uint16Set(&transmissionState->m_remainingBytes, len);
  transmissionState->m_byteToSend = (const char*)mem;
  transmissionState->state = bapi_uart_E_TS_Normal;
  transmissionState->mode = mode;
}

/**
 * \ingroup bapi_uart
 * \brief
 * Invalidate the transmission state so that there is no next byte to transmit.
 */
C_INLINE void bapi_uart_invalidateTransmissionState(
  struct bapi_uart_TransmissionState* transmissionState /**< The transmission state to be initialized */
  ) {
  /* Order of setting m_remainingBytes and m_byteToSend is important for synchronization with ISR */
  transmissionState->m_byteToSend = 0;
  transmissionState->state = bapi_uart_E_TS_Normal;

  /* Unset "inUse" last */
  atomic_Uint16Set(&transmissionState->m_remainingBytes, 0);
}

/**
 * \ingroup bapi_uart
 * \brief
 * See whether the transmission state is currently in use for an ongoing transmission.
 */
C_INLINE bool bapi_uart_isInUse(
  const struct bapi_uart_TransmissionState* transmissionState
  ) {
  return atomic_Uint16Get(&transmissionState->m_remainingBytes) != 0;
}

/**
 * \ingroup _bapi_uart_tx
 * \brief
 * Retrieve the next character to send according to the transition state and update the transition state for the next call.
 * This function is suggested to be used by the board/vendor specific ISR (Interrupt Service Routine).
 *
 * \return the next character to send.
 * */
C_INLINE char _bapi_uart_getNextTxCharAndUpdateTransmissionState(struct bapi_uart_TransmissionState* transmissionState)
{
  char c = *transmissionState->m_byteToSend;
  if (transmissionState->mode == bapi_uart_E_TxMode_CRLF) {
    /* Extra CRLF mode */

    if (transmissionState->state == bapi_uart_E_TS_Normal) {
      /* Normal state */
      if (c == '\r') {
        transmissionState->state = bapi_uart_E_TS_ExtraCR;
      } else {
        if (c == '\n') {
          c = '\r';
          transmissionState->state = bapi_uart_E_TS_ExtraCR;
        } else {
          transmissionState->m_byteToSend++;
          transmissionState->m_remainingBytes--;
        }
      }
    }
    else {
      /* Extra CR_LF state */
      transmissionState->state = bapi_uart_E_TS_Normal;
      c = '\n';
      transmissionState->m_byteToSend++;
      transmissionState->m_remainingBytes--;
    }
  } else {
    /* Transparent mode */
    transmissionState->m_byteToSend++;
    transmissionState->m_remainingBytes--;
  }
  return c;
}


/**
 * \ingroup bapi_uart
 * \brief
 * Replaces the DATA RECEIVED callback function by a new one.
 *
 * \return The old DATA RECEIVED callback function that was replaced.
 */
C_INLINE bapi_uart_dataReceived_ISRCallback_t
  bapi_uart_setDataReceived_ISRCallback(
      enum bapi_E_UartIndex_ uartIndex                   /**< [in] The UART for which to set the callback */
    , bapi_uart_dataReceived_ISRCallback_t rxIrqHandler  /**< [in] The new callback function to become valid. */
    ) {
  const struct _bapi_uart_interface* uartInterface = _bapi_uart_getUartInterface(uartIndex);
  return uartInterface->setDataReceived_ISRCallback(uartIndex, rxIrqHandler);
}

/**
 * \ingroup bapi_uart
 * \brief
 * Replaces the TRANSMISSION COMPLETE and GET TRANSMISSION STATE callback functions for a particular UART.
 *
 * <em><b>Important:</b></em> If you are replacing the callback functions, you should check, whether you need to
 * clean up (in particular free up memory) any TRANSMISSION that will not processed by the
 * new callback functions.
 *
 * \param uartIndex The UART for which to set the callbacks.
 *
 * \param msgTransmitted_ISRCallback The TRANSMISSION COMPLETE callback function.
 * When the transmission ISR is invoked it will look into the corresponding transmission
 * state for remaining bytes to send. If so, it will just place next byte(s) into the
 * UART's transmission buffer. Otherwise it will call this callback function.
 * After that, the ISR will immediately asks for a transmission state again,
 * and continue the transmission, if needed.
 *
 * \param getTransmissionState_ISRCallback The GET TRANSMISSION STATE callback function.
 * When the transmission ISR is invoked, it calls this callback function to retrieve the
 * actual transmission state. The transmission state is needed by the ISR in order to decide
 * what to do. In the easiest case it just sends the next byte from the message of the transmission
 * state.
 *
 * \return The old callback functions. */
C_INLINE struct bapi_uart_Tx_ISRCallbacks
  bapi_uart_setMsgTransmission_ISRCallbacks(
    enum bapi_E_UartIndex_ uartIndex,
    bapi_uart_msgTransmissionComplete_ISRCallback_t msgTransmitted_ISRCallback,
    bapi_uart_getTransmissionState_ISRCallback_t getTransmissionState_ISRCallback
  ) {
  const struct _bapi_uart_interface* uartInterface = _bapi_uart_getUartInterface(uartIndex);
  return uartInterface->setMsgTransmission_ISRCallbacks(uartIndex, msgTransmitted_ISRCallback
    , getTransmissionState_ISRCallback);
}



/**
 * \ingroup _bapi_uart
 * \brief
 * This internal structure holds all callback functions that are part of the UART board API
 */
struct _uart_Callbacks {
  bapi_uart_dataReceived_ISRCallback_t m_rxIrqHandler;
  struct bapi_uart_Tx_ISRCallbacks m_txCallbacks;
};


/**
 * \ingroup bapi_uart
 * \brief Will start the the transmission of a message, in case there is not already
 * another message under transmission. Otherwise this function does nothing.
 *
 * <em>Note</em>: If there is a message under transmission, the TRANSMISSION COMPLETE callback function
 * will be called upon completion. This callback can than provide any further message to be
 * sent, and transmission is just being continued.
 */
C_INLINE void bapi_uart_startTx(const enum bapi_E_UartIndex_ uartIndex) {
  const struct _bapi_uart_interface* uartInterface = _bapi_uart_getUartInterface(uartIndex);
  uartInterface->startTx(uartIndex);
}

/**
 * \ingroup bapi_uart
 * \brief
 * Ensures that a particular UART related interrupt does not happen, and that the corresponding ISR callback function
 * is not called. 
 * 
 * Calling this function X times requires also to withdraw the request X times via
 * bapi_uart_exitCritical(const enum bapi_E_UartIndex_, enum bapi_uart_E_UartIrqType), before the interrupt is really
 * physically enabled.
 *
 */
C_INLINE void bapi_uart_enterCritical(
  const enum bapi_E_UartIndex_ uartIndex,  /**< [in] The UART for which the interrupt shall be ensured not to happen. */
  enum bapi_uart_E_UartIrqType irqType     /**< [in] The Interrupt type that shall be ensured not to happen. */
  ) {
  const struct _bapi_uart_interface* uartInterface = _bapi_uart_getUartInterface(uartIndex);
  uartInterface->uart_enterCritical(uartIndex, irqType);
}

/**
 * \ingroup bapi_uart
 * \brief
 * Withdraws the request that an UART related interrupt must not happen.
 *
 */
C_INLINE void bapi_uart_exitCritical(
  const enum bapi_E_UartIndex_ uartIndex,  /**< [in] The UART for which to withdraw the request. */
  enum bapi_uart_E_UartIrqType irqType    /**< [in] The Interrupt type for which to withdraw the request. */
  ) {
  const struct _bapi_uart_interface* uartInterface = _bapi_uart_getUartInterface(uartIndex);
  uartInterface->uart_exitCritical(uartIndex, irqType);
}


#endif /* BAPI_UART_H_ */
