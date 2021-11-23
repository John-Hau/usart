/*
 * bapi_uart_MCU_VENDOR_NXP.cpp
 *
 *  Created on: 07.11.2019
 *      Author: e578153
 */


/**
 * \file
 * \brief This file implements the USART Board API for Freescale MCUs.
 */

#include "baseplate.h"

#include "boards/board-api/bapi_uart.h"

#include "boards/board-api/bapi_irq.h"
#include "board_uart_cfg_MCU_VENDOR_NXP.h"

#include "board_uart_MCU_VENDOR_NXP.inc"


#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

#if FSL_FEATURE_SOC_UART_COUNT > 0
  #include "fsl_device_registers.h"
  #include "fsl_uart.h"
#endif

#if LPUART_INSTANCE_COUNT > 0
  #include "fsl_lpuart.h"
#endif /* LPUART_INSTANCE_COUNT > 0 */

#if UART0_INSTANCE_COUNT > 0
  #include "fsl_lpsci.h"
#endif /* UART0_INSTANCE_COUNT > 0 */

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif
 #include "fsl_gpio.h"

/********************* Callbacks *********************************************/

C_FUNC struct bapi_uart_Tx_ISRCallbacks
_bapi_uart_setMsgTransmission_ISRCallbacks(
  enum bapi_E_UartIndex_ uartIndex
  , bapi_uart_msgTransmissionComplete_ISRCallback_t msgTransmitted_ISRCallback
  , bapi_uart_getTransmissionState_ISRCallback_t getTransmissionState
  ) {

  bapi_irq_enterCritical();
  struct bapi_uart_Tx_ISRCallbacks retval = _uart_callbacks[uartIndex].m_txCallbacks;

  if(_uart_callbacks[uartIndex].m_txCallbacks.m_getTransmissionState != getTransmissionState) {
    _uart_callbacks[uartIndex].m_txCallbacks.m_getTransmissionState = getTransmissionState;
  }

  if(_uart_callbacks[uartIndex].m_txCallbacks.m_msgTransmissionCompleteHandler != msgTransmitted_ISRCallback) {
    _uart_callbacks[uartIndex].m_txCallbacks.m_msgTransmissionCompleteHandler = msgTransmitted_ISRCallback;
  }
  bapi_irq_exitCritical();
  return retval;
}

C_FUNC bapi_uart_dataReceived_ISRCallback_t
  _bapi_uart_setDataReceived_ISRCallback(enum bapi_E_UartIndex_ uartIndex,
    bapi_uart_dataReceived_ISRCallback_t rxIrqHandler) {
  bapi_irq_enterCritical();
  bapi_uart_dataReceived_ISRCallback_t retval = _uart_callbacks[uartIndex].m_rxIrqHandler;
  if(_uart_callbacks[uartIndex].m_rxIrqHandler != rxIrqHandler) {
    _uart_callbacks[uartIndex].m_rxIrqHandler = rxIrqHandler;
  }
  bapi_irq_exitCritical();
  return retval;
}



/********************* Interface flags ***************************************/

C_INLINE void __setInterfaceFlag(const enum bapi_E_UartIndex_ uartIndex, uint32_t value) {

  GPIO_Type* gpioPort = _uartFlagProperties[uartIndex].m_portGpio;
  uint32_t m_portPin = _uartFlagProperties[uartIndex].m_portPin;

  if (value) {
	  GPIO_PinWrite(gpioPort, m_portPin, 1);
  } else {
	  GPIO_PinWrite(gpioPort, m_portPin, 0);

  }
}

C_INLINE uint32_t __getInterfaceFlag(const enum bapi_E_UartIndex_ uartIndex) {
  GPIO_Type* port = ((GPIO_Type*)_uartFlagProperties[uartIndex].m_portGpio);
  uint32_t mask = 1ul << _uartFlagProperties[uartIndex].m_portPin;

  //uint32_t pdor = port->PDOR;
  //return ((pdor & mask) ? 1 : 0);
  //TODO
  return 0;
}

STATIC bool _bapi_uart_setInterfaceFlag(
  const enum bapi_E_UartIndex_ uartIndex      /**< [in] The UART to be configured */
  , enum bapi_E_InterfaceFlag_ interfaceFlag  /**< [in] The Interface Flag to be set */
  , uint32_t value
  ) {
  if (_uartFlagProperties[uartIndex].m_interfaceFlag == interfaceFlag) {
    __setInterfaceFlag(uartIndex, value);
    return true;
  }
  return false;
}

STATIC bool _bapi_uart_getInterfaceFlag(
  const enum bapi_E_UartIndex_ uartIndex      /**< [in] The UART to be configured */
  , enum bapi_E_InterfaceFlag_ interfaceFlag  /**< [in] The Interface Flag to be set */
  , uint32_t* value
  ) {
  if (_uartFlagProperties[uartIndex].m_interfaceFlag == interfaceFlag) {
    *value = __getInterfaceFlag(uartIndex);
    return true;
  }
  return false;
}

//#ifdef _DEBUG
///* Validate that the _fslUartInterfaces array is uart type index enumeration are setup consistently. */
//static void _validate_fslUartInterfaces();
//#endif

/**
 * \brief implements the Uart Interface for the Freescale standard UART type,
 *    Low Power UART type and UART0 (Lpsci) type, depending on the template
 *    parameter:
 *      -) Uart_Hal_Normalizer   (standard)
 *      -) Lpuart_Hal_Normalizer (low power)
 *      -) Lpsci_Hal_Normalizer  (lpsci, UART0)
 */
template<typename Hal_Normalizer> struct _fslUart {

  static uint32_t configure(const enum bapi_E_UartIndex_ uartIndex, uint32_t baudrate, uint32_t armUsartControl) {
    return HalSerial<Hal_Normalizer>::configure(uartIndex, baudrate, armUsartControl);
  }

  static void unconfigure(const enum bapi_E_UartIndex_ uartIndex) {
    HalSerial<Hal_Normalizer>::unconfigure(uartIndex);
  }

  static enum bapi_E_UartMode_ getMode(const enum bapi_E_UartIndex_ uartIndex){
    return HalSerial<Hal_Normalizer>::uartMode(uartIndex);
  }

  static void uart_enterCritical(const enum bapi_E_UartIndex_ uartIndex, enum bapi_uart_E_UartIrqType irqType){
    HalSerial<Hal_Normalizer>::enterCritical(uartIndex, irqType);
  }

  static void uart_exitCritical(const enum bapi_E_UartIndex_ uartIndex, enum bapi_uart_E_UartIrqType irqType){
    HalSerial<Hal_Normalizer>::exitCritical(uartIndex, irqType);
  }

  static void startTx(const enum bapi_E_UartIndex_ uartIndex){
    HalSerial<Hal_Normalizer>::startTx(uartIndex);
  }

  static bool setLoopCmd(const enum bapi_E_UartIndex_ uartIndex, bool bEnable){
    return Hal_Normalizer::HAL_SetLoopCmd(_uart_getUartAddress<typename Hal_Normalizer::_SERIAL_BaseAddr_type>(uartIndex), bEnable);
  }

  static struct bapi_uart_Tx_ISRCallbacks setMsgTransmission_ISRCallbacks(const enum bapi_E_UartIndex_ uartIndex,
      bapi_uart_msgTransmissionComplete_ISRCallback_t msgTransmitted_ISRCallback
    , bapi_uart_getTransmissionState_ISRCallback_t    getTransmissionState
    ) {
    return _bapi_uart_setMsgTransmission_ISRCallbacks(uartIndex, msgTransmitted_ISRCallback, getTransmissionState);
  }

  static bapi_uart_dataReceived_ISRCallback_t setDataReceived_ISRCallback(const enum bapi_E_UartIndex_ uartIndex,
    bapi_uart_dataReceived_ISRCallback_t rxIrqHandler) {
    return _bapi_uart_setDataReceived_ISRCallback(uartIndex, rxIrqHandler);
  }

  static bool setInterfaceFlag(const enum bapi_E_UartIndex_ uartIndex, enum bapi_E_InterfaceFlag_ interfaceFlag, uint32_t value) {
    return _bapi_uart_setInterfaceFlag(uartIndex, interfaceFlag, value);
  }

  static uint32_t getInterfaceFlag(const enum bapi_E_UartIndex_ uartIndex, enum bapi_E_InterfaceFlag_ interfaceFlag, uint32_t* value ) {
    return _bapi_uart_getInterfaceFlag(uartIndex, interfaceFlag, value);
  }

  static uint32_t enableTransmitter(const enum bapi_E_UartIndex_ uartIndex) {
    return HalSerial<Hal_Normalizer>::enableTransmitter(uartIndex);
  }

  static uint32_t disableTransmitter(const enum bapi_E_UartIndex_ uartIndex) {
    return HalSerial<Hal_Normalizer>::disableTransmitter(uartIndex);
  }

  static uint32_t enableReceiver(const enum bapi_E_UartIndex_ uartIndex) {
    return HalSerial<Hal_Normalizer>::enableReceiver(uartIndex);
  }

  static uint32_t disableReceiver(const enum bapi_E_UartIndex_ uartIndex) {
    return HalSerial<Hal_Normalizer>::disableReceiver(uartIndex);
  }

  static void flushTxFifo(const enum bapi_E_UartIndex_ uartIndex) {
    HalSerial<Hal_Normalizer>::flushTxFifo(uartIndex);
  }

  static void flushRxFifo(const enum bapi_E_UartIndex_ uartIndex) {
    HalSerial<Hal_Normalizer>::flushRxFifo(uartIndex);
  }

  static bapi_uart_fifo_size_t setTxFifo(const enum bapi_E_UartIndex_ uartIndex, bapi_uart_fifo_size_t fifoSize) {
    return HalSerial<Hal_Normalizer>::setTxFifo(uartIndex, fifoSize);
  }

  static bapi_uart_fifo_size_t setRxFifo(const enum bapi_E_UartIndex_ uartIndex, bapi_uart_fifo_size_t fifoSize) {
    return HalSerial<Hal_Normalizer>::setRxFifo(uartIndex, fifoSize);
  }

  static uint32_t getBaudrate(const enum bapi_E_UartIndex_ uartIndex) {
    return HalSerial<Hal_Normalizer>::getBaudrate(uartIndex);
  }

  static bool setBaudrate(const enum bapi_E_UartIndex_ uartIndex, uint32_t baudRate) {
    return HalSerial<Hal_Normalizer>::setBaudrate(uartIndex, baudRate);
  }

};

#define _BAPI_FSL_USART_INIT(Hal_Normalizer) \
  _fslUart<Hal_Normalizer>::setMsgTransmission_ISRCallbacks, \
  _fslUart<Hal_Normalizer>::setDataReceived_ISRCallback, \
  _fslUart<Hal_Normalizer>::configure, \
  _fslUart<Hal_Normalizer>::unconfigure, \
  _fslUart<Hal_Normalizer>::getMode, \
  _fslUart<Hal_Normalizer>::uart_enterCritical, \
  _fslUart<Hal_Normalizer>::uart_exitCritical, \
  _fslUart<Hal_Normalizer>::startTx, \
  _fslUart<Hal_Normalizer>::setInterfaceFlag, \
  _fslUart<Hal_Normalizer>::getInterfaceFlag, \
  _fslUart<Hal_Normalizer>::setLoopCmd, \
  _fslUart<Hal_Normalizer>::enableTransmitter, \
  _fslUart<Hal_Normalizer>::disableTransmitter, \
  _fslUart<Hal_Normalizer>::enableReceiver, \
  _fslUart<Hal_Normalizer>::disableReceiver, \
  _fslUart<Hal_Normalizer>::flushTxFifo, \
  _fslUart<Hal_Normalizer>::flushRxFifo, \
  _fslUart<Hal_Normalizer>::setTxFifo, \
  _fslUart<Hal_Normalizer>::setRxFifo, \
  _fslUart<Hal_Normalizer>::getBaudrate, \
  _fslUart<Hal_Normalizer>::setBaudrate


#if UART_INSTANCE_COUNT > 0
STATIC  const struct _bapi_uart_interface _fslUartInterface   = { _BAPI_FSL_USART_INIT(Uart_Hal_Normalizer) };
#endif /* UART_INSTANCE_COUNT > 0 */

#if (LPUART_INSTANCE_COUNT > 0) && (_BAPI_NO_FS_LPUART_USAGE == 0)
STATIC  const struct _bapi_uart_interface _fslLpuartInterface = { _BAPI_FSL_USART_INIT(Lpuart_Hal_Normalizer) };
#endif /* LPUART_INSTANCE_COUNT > 0 */

#if UART0_INSTANCE_COUNT > 0
STATIC  const struct _bapi_uart_interface  _fslLpsciInterface = { _BAPI_FSL_USART_INIT(Lpsci_Hal_Normalizer) };
#endif /* UART0_INSTANCE_COUNT > 0 */

STATIC const struct _bapi_uart_interface* const _fslUartInterfaces[fslUartTypeIndexCount] = {
  /* Interface for standard UARTs */
#if UART_INSTANCE_COUNT > 0
  &_fslUartInterface,
#else
  0,
#endif /* UART_INSTANCE_COUNT > 0 */

  /* Interface for standard LPUARTs */
#if (LPUART_INSTANCE_COUNT > 0) && (_BAPI_NO_FS_LPUART_USAGE == 0)
  &_fslLpuartInterface,
#else
  0,
#endif /* LPUART_INSTANCE_COUNT > 0 */

  /* Interface for standard LPSCIs (UART0) */
#if UART0_INSTANCE_COUNT > 0
  &_fslLpsciInterface,
#else
  0,
#endif /* UART0_INSTANCE_COUNT > 0 */
};

const _bapi_uart_interface* _bapi_uart_getUartInterface(enum bapi_E_UartIndex_ uartIndex) {
  const enum _fsl_E_uart_type uartType = _fslUartType(uartIndex);

  /* Pick the right interface for the UART type */
  ASSERT(uartType < ARRAY_SIZE(_fslUartInterfaces));

  const _bapi_uart_interface* retval = _fslUartInterfaces[uartType];
  ASSERT(retval);

  return retval;
}


/********************* Low Power UART IRQ Handler ****************************/
#if (LPUART_INSTANCE_COUNT) > 0 && (_BAPI_NO_FS_LPUART_USAGE == 0)
/**
 * \ingroup _bapi_uart
 * \brief The common LPSCI Rx/Tx ISR that will be called by all LPSCI_IRQ handlers.
 *
 * */
C_FUNC void LPUART_DRV_IRQHandler(uint32_t instance) {
  typedef typename Lpuart_Hal_Normalizer::_SERIAL_BaseAddr_type _SERIAL_BaseAddr_type;
  typedef Lpuart_Hal_Normalizer HalNormalizer;

  const bapi_E_UartIndex uartIndex = _fslLpuartInstance2UartIndex(instance);

  const _SERIAL_BaseAddr_type baseAddr = _uart_getUartAddress<_SERIAL_BaseAddr_type>(uartIndex);

  uint32_t errorEvents =
#ifndef BAPI_DISABLE_UART_ERROR_HANDLING /* Error handling can be disabled by defining BAPI_DISABLE_UART_ERROR_HANDLING */
    HalSerial<HalNormalizer>::getUartErrorFlags(baseAddr);
#else
   0;
#endif


  /* Handle receive data register full interrupt */
  if((HalNormalizer::HAL_GetIntMode(baseAddr, HalNormalizer::kIntRxDataRegFull))
    && (HalNormalizer::HAL_GetStatusFlag(baseAddr, HalNormalizer::kRxDataRegFull)))
  {

#if FSL_FEATURE_LPUART_HAS_FIFO
    /* Read out all data from RX FIFO */
	uint8_t countBytes = ((uint8_t)((baseAddr->WATER & LPUART_WATER_RXCOUNT_MASK) >> LPUART_WATER_RXCOUNT_SHIFT));
    while(countBytes>0)
    {
#endif

	  /* Get data and put into receive buffer */
      uint8_t rxChar = HalNormalizer::HAL_serialGetChar(baseAddr);

      /* Invoke callback if there is one */
      if (_uart_callbacks[uartIndex].m_rxIrqHandler) {
        (*_uart_callbacks[uartIndex].m_rxIrqHandler)(uartIndex, errorEvents, &rxChar, 1);
      }
#if BAPI_TRACE_UART_IRQ_HANDLER
      uartHandlerTrace[uartIndex].RDRF_Handled++;
#endif

      countBytes--;
#if FSL_FEATURE_LPUART_HAS_FIFO
    }
#endif

  } else {
#ifndef BAPI_DISABLE_UART_ERROR_HANDLING /* Error handling can be disabled by defining BAPI_DISABLE_UART_ERROR_HANDLING */
    /* In case of any error events, invoke callback if there is one */
    if(errorEvents && _uart_callbacks[uartIndex].m_rxIrqHandler) {
      (*_uart_callbacks[uartIndex].m_rxIrqHandler)(uartIndex, errorEvents, 0, 0);
    }
#endif
  }

#if BAPI_DISABLE_TX_COMPLETE_HANDLING < 1
  /* Handle transmission complete interrupt */
  if ((HalNormalizer::HAL_GetIntMode(baseAddr, HalNormalizer::kIntTxComplete))
    && (HalNormalizer::HAL_GetStatusFlag(baseAddr, HalNormalizer::kTxComplete)))
  {
    HalSerial<HalNormalizer>::serialHandleTxComplete(uartIndex);
  }
#endif

  /* Handle transmit data register empty interrupt */
  if ((HalNormalizer::HAL_GetIntMode(baseAddr, HalNormalizer::kIntTxDataRegEmpty))
    &&(HalNormalizer::HAL_GetStatusFlag(baseAddr, HalNormalizer::kTxDataRegEmpty )))
  {
    HalSerial<HalNormalizer>::serialHandleTransmission(uartIndex);
  }

  /* Handle receive overrun interrupt */
  if ((HalNormalizer::HAL_GetStatusFlag(baseAddr, HalNormalizer::kRxOverrun ))) {
    /* Clear the flag, OR the rxDataRegFull will not be set any more */
	  HalNormalizer::HAL_ClearStatusFlag(baseAddr, HalNormalizer::kRxOverrun);
  }
}

#endif /* #if LPUART_INSTANCE_COUNT > 0 */


/********************* Lpsci UART IRQ Handler ********************************/
#if UART0_INSTANCE_COUNT > 0
/**
 * \ingroup _bapi_uart
 * \brief The common LPSCI Rx/Tx ISR that will be called by all LPSCI_IRQ handlers.
 *
 * */
C_FUNC void LPSCI_DRV_IRQHandler(uint32_t instance) {
  typedef typename Lpsci_Hal_Normalizer::_SERIAL_BaseAddr_type _SERIAL_BaseAddr_type;
  typedef Lpsci_Hal_Normalizer HalNormalizer;

  const bapi_E_UartIndex uartIndex = _fslUartInstance2UartIndex(instance);

  const _SERIAL_BaseAddr_type baseAddr = _uart_getUartAddress<_SERIAL_BaseAddr_type>(uartIndex);

  uint32_t errorEvents =
#ifndef BAPI_DISABLE_UART_ERROR_HANDLING /* Error handling can be disabled by defining BAPI_DISABLE_UART_ERROR_HANDLING */
    HalSerial<HalNormalizer>::getUartErrorFlags(baseAddr);
#else
   0;
#endif


  /* Handle receive data register full interrupt */
  if((HalNormalizer::HAL_GetIntMode(baseAddr, HalNormalizer::kIntRxDataRegFull))
    && (HalNormalizer::HAL_GetStatusFlag(baseAddr, HalNormalizer::kRxDataRegFull)))
  {

#if FSL_FEATURE_LPSCI_HAS_FIFO
    /* Read out all data from RX FIFO */
    while(UART0_HAL_GetRxDatawordCountInFifo(baseAddr))
    {
#endif

      /* Get data and put into receive buffer */
      uint8_t rxChar = HalNormalizer::HAL_serialGetChar(baseAddr);

      /* Invoke callback if there is one */
      if (_uart_callbacks[uartIndex].m_rxIrqHandler) {
        (*_uart_callbacks[uartIndex].m_rxIrqHandler)(uartIndex, errorEvents, &rxChar, 1);
      }

#if FSL_FEATURE_LPSCI_HAS_FIFO
    }
#endif

  } else {
#ifndef BAPI_DISABLE_UART_ERROR_HANDLING /* Error handling can be disabled by defining BAPI_DISABLE_UART_ERROR_HANDLING */
    /* In case of any error events, invoke callback if there is one */
    if(errorEvents && _uart_callbacks[uartIndex].m_rxIrqHandler) {
      (*_uart_callbacks[uartIndex].m_rxIrqHandler)(uartIndex, errorEvents, 0, 0);
    }
#endif
  }

#if BAPI_DISABLE_TX_COMPLETE_HANDLING < 1
  /* Handle transmission complete interrupt */
  if ((HalNormalizer::HAL_GetIntMode(baseAddr, HalNormalizer::kIntTxComplete))
    && (HalNormalizer::HAL_GetStatusFlag(baseAddr, HalNormalizer::kTxComplete)))
  {
    HalSerial<HalNormalizer>::serialHandleTxComplete(uartIndex);
  }
#endif

  /* Handle transmit data register empty interrupt */
  if ((HalNormalizer::HAL_GetIntMode(baseAddr, HalNormalizer::kIntTxDataRegEmpty))
    &&(HalNormalizer::HAL_GetStatusFlag(baseAddr, HalNormalizer::kTxDataRegEmpty )))
  {
    HalSerial<HalNormalizer>::serialHandleTransmission(uartIndex);
  }

  /* Handle receive overrun interrupt */
  if (LPSCI_HAL_GetStatusFlag(baseAddr, kLpsciRxOverrun)) {
    /* Clear the flag, OR the rxDataRegFull will not be set any more */
    LPSCI_HAL_ClearStatusFlag(baseAddr, kLpsciRxOverrun);
  }
}

#endif /* #if UART0_INSTANCE_COUNT > 0 */


/* */
#if defined (_DEBUG) && ! defined(BAPI_DEBUG_UART_IRQ)
#define BAPI_DEBUG_UART_IRQ 0
#endif

#if BAPI_DEBUG_UART_IRQ

struct uart_registers_image {
  uint8_t rd_s1;
  uint8_t rd_s2;
  #if FSL_FEATURE_UART_HAS_EXTENDED_DATA_REGISTER_FLAGS
  uint8_t rd_ed;
  #endif
  #if FSL_FEATURE_UART_HAS_FIFO
  uint8_t rd_sfifo;
  #endif

  uint8_t bdh;
  uint8_t c2;
  uint8_t c3;
  #if FSL_FEATURE_UART_HAS_FIFO
  uint8_t rd_cfifo;
  #endif
};

/**
 * UART Registers are copied to here, so that it is possible to see registers values
 * before the IRQ handlers breakpoint was reached.
 */
STATIC struct uart_registers_image uartRegistersImage[bapi_E_UartCount];

#endif /* #if BAPI_DEBUG_UART_IRQ */

#if defined (_DEBUG) && ! defined(BAPI_TRACE_UART_IRQ_HANDLER)
#define BAPI_TRACE_UART_IRQ_HANDLER 0
#endif


#if BAPI_TRACE_UART_IRQ_HANDLER

struct uart_handler_trace {
  unsigned RDRF_Handled;
  unsigned ERR_Handled;
  unsigned TDRE_Handled;
  unsigned TC_Handled;
};

STATIC struct uart_handler_trace uartHandlerTrace[bapi_E_UartCount];

#endif /* BAPI_TRACE_UART_IRQ_HANDLER */

/********************* Normal UART IRQ Handler *******************************/
#if UART_INSTANCE_COUNT > 0
/**
 * \ingroup _bapi_uart
 * \brief The common UART Rx/Tx ISR that will be called by all UART_IRQ handlers.
 *
 */
/*
 * Important !!!:
 *
 *    B e   c a r e f u l   w i t h   d e b u g g i n g.
 *    B e   c a r e f u l   w i t h   d e b u g g i n g.
 *    B e   c a r e f u l   w i t h   d e b u g g i n g.
 *
 * Please read the comments about debugging within the body of this function
 * in order to draw not the wrong conclusions. Watch for "!!! Debugging"
 * tags.
 */
C_FUNC void UART_DRV_IRQHandler(uint32_t instance) {

  typedef typename Uart_Hal_Normalizer::_SERIAL_BaseAddr_type _SERIAL_BaseAddr_type;
  typedef Uart_Hal_Normalizer HalNormalizer;

  const bapi_E_UartIndex uartIndex = _fslUartInstance2UartIndex(instance);
  const _SERIAL_BaseAddr_type baseAddr = _uart_getUartAddress<_SERIAL_BaseAddr_type>(uartIndex);

#if BAPI_DEBUG_UART_IRQ

  uartRegistersImage[uartIndex].rd_s1 = UART_RD_S1(baseAddr);
  uartRegistersImage[uartIndex].rd_s2 = UART_RD_S2(baseAddr);
#if FSL_FEATURE_UART_HAS_EXTENDED_DATA_REGISTER_FLAGS
  uartRegistersImage[uartIndex].rd_ed = UART_RD_ED(baseAddr);
#endif
#if FSL_FEATURE_UART_HAS_FIFO
  uartRegistersImage[uartIndex].rd_sfifo = UART_RD_SFIFO(baseAddr);
#endif

  uartRegistersImage[uartIndex].bdh = UART_RD_BDH(baseAddr);
  uartRegistersImage[uartIndex].c2  = UART_RD_C2(baseAddr);
  uartRegistersImage[uartIndex].c3  = UART_RD_C3(baseAddr);
#if FSL_FEATURE_UART_HAS_FIFO
  uartRegistersImage[uartIndex].rd_cfifo  = UART_RD_CFIFO(baseAddr);
#endif

#endif /* #if BAPI_DEBUG_UART_IRQ */

  unsigned ANYTHING_Handled = 0;

#if  BAPI_TRACE_UART_IRQ_HANDLER
  uartHandlerTrace[uartIndex].RDRF_Handled = 0;
  uartHandlerTrace[uartIndex].ERR_Handled = 0;
  uartHandlerTrace[uartIndex].TDRE_Handled = 0;
  uartHandlerTrace[uartIndex].TC_Handled = 0;
#endif

  /* !!! Debugging: If you set a breakpoint before here, the UART will continue working
   * and might change it's registers in the background.
   * So the uartRegistersImage will not get the values as they would get without
   * having set a break point before here.
   *
   * Hence you rather should set a break point below this line.
   */


#if BAPI_DEBUG_UART_IRQ
  /* if you want to observe a particular UART, you can change the instance
   * to your needs and set a break point at line: int x = 0; */
   if(instance == 3) {
     int x = 0;
   }
#endif

  uint32_t errorEvents =
#ifndef BAPI_DISABLE_UART_ERROR_HANDLING /* Error handling can be disabled by defining BAPI_DISABLE_UART_ERROR_HANDLING */
    HalSerial<HalNormalizer>::getUartErrorFlags(baseAddr);
#else
   0;
#endif


#if BAPI_DEBUG_UART_IRQ
  /* if you want to observe a particular UART, you can change the instance
   * to your needs and set a break point at line: int x = 0; */
   if(instance == 3) {
     int x = 0;
   }
#endif


  /* Handle receive data register full interrupt */
  /* Important: kRxDataRegFull is not asserted in case of an Overrun. */
  if((HalNormalizer::HAL_GetIntMode(baseAddr, HalNormalizer::kIntRxDataRegFull))
    && (HalNormalizer::HAL_GetStatusFlag(baseAddr, HalNormalizer::kRxDataRegFull)))
  {
#if FSL_FEATURE_UART_HAS_FIFO
    /* Read out all data from RX FIFO */
    while(UART_RD_RCFIFO(baseAddr))
    {
#endif
      /* Get data and put into receive buffer */
      uint8_t rxChar = HalNormalizer::HAL_serialGetChar(baseAddr);

      /* Invoke callback if there is one */
      if(_uart_callbacks[uartIndex].m_rxIrqHandler) {
        (*_uart_callbacks[uartIndex].m_rxIrqHandler)(uartIndex, errorEvents, &rxChar, 1);
      }

#if BAPI_TRACE_UART_IRQ_HANDLER
      uartHandlerTrace[uartIndex].RDRF_Handled++;
#endif
    ANYTHING_Handled++;

#if FSL_FEATURE_UART_HAS_FIFO
    }
#endif
  } else {
#ifndef BAPI_DISABLE_UART_ERROR_HANDLING /* Error handling can be disabled by defining BAPI_DISABLE_UART_ERROR_HANDLING */
    /* In case of any error events, invoke callback if there is one */
	//if(errorEvents && _uart_callbacks[uartIndex].m_rxIrqHandler)
    if((errorEvents > 0) && (_uart_callbacks[uartIndex].m_rxIrqHandler)) {
      (*_uart_callbacks[uartIndex].m_rxIrqHandler)(uartIndex, errorEvents, 0, 0);
    }

#if BAPI_TRACE_UART_IRQ_HANDLER
    uartHandlerTrace[uartIndex].ERR_Handled++;
#endif
    ANYTHING_Handled++;

#endif
  }

  /* !!! Debugging: If you set a breakpoint before here, the UART will continue working
   * and might set it's Receiver Overrun (OR) Flag in background. An receiver overrun
   * will also de-assert the Receive Data Register Full Flag (RDRF).
   *
   * So when have reached a break point before here, you will probably impact the program flow
   * of the ISR, because an Receiver Overrun might be detected instead of an
   * Receive Data Register Full event.
   *
   * But anyway, you can see the un-impacted Flags if you did set BAPI_DEBUG_UART_IRQ
   * to 1 and watch the variable uartRegistersImage. Of course you shouldn't have set
   * a breakpoint, before the uart registers have been copied into uartRegistersImage.
   *
   */

#if BAPI_DEBUG_UART_IRQ
  /* if you want to observe a particular UART, you can change the instance
   * to your needs and set a break point at line: int x = 0; */
   if(instance == 3) {
     int x = 0;
   }
#endif

//#ifndef BAPI_DISABLE_TX_COMPLETE_HANDLING < 1
#ifndef BAPI_DISABLE_TX_COMPLETE_HANDLING
  /* Handle transmission complete interrupt */
  if ( (HalNormalizer::HAL_GetIntMode(baseAddr, HalNormalizer::kIntTxComplete))
    && (HalNormalizer::HAL_GetStatusFlag(baseAddr, HalNormalizer::kTxComplete)))
  {
    HalSerial<HalNormalizer>::serialHandleTxComplete(uartIndex);
#if BAPI_TRACE_UART_IRQ_HANDLER
    uartHandlerTrace[uartIndex].TC_Handled++;
#endif
    ANYTHING_Handled++;
  }
#endif

  /* Handle transmit data register empty interrupt */
  if ((HalNormalizer::HAL_GetIntMode(baseAddr, HalNormalizer::kIntTxDataRegEmpty))
    && (HalNormalizer::HAL_GetStatusFlag(baseAddr, HalNormalizer::kTxDataRegEmpty)))
  {
    HalSerial<HalNormalizer>::serialHandleTransmission(uartIndex);
#if BAPI_TRACE_UART_IRQ_HANDLER
    uartHandlerTrace[uartIndex].TDRE_Handled++;
#endif
    ANYTHING_Handled++;
  }

  /* Workaround for a never ending interrupt invocations that was observed on FS_IRMLC Uart 3 */
  //if(!ANYTHING_Handled) {
  if(0 == ANYTHING_Handled) {
    /* Ensure data register to be read. */
    uint8_t c = HalNormalizer::HAL_serialGetChar(baseAddr);
  }

  /* Ensure Overrun Status cleared which might be caused by an above breakpoint that was reached
   * while debugging. */
  if (HalNormalizer::HAL_GetStatusFlag(baseAddr, HalNormalizer::kRxOverrun))
  {
      /* Clear the flag, OR the rxDataRegFull will not be set any more */
      HalNormalizer::HAL_ClearStatusFlag(baseAddr, Uart_Hal_Normalizer::kRxOverrun);
  }
}
#endif

#if (LPUART_INSTANCE_COUNT > 0)
/* Implementation of UART0 handler named in startup code. */
C_FUNC void LPUART1_IRQHandler(void)
{
	LPUART_DRV_IRQHandler(1);
}
#endif

#if (LPUART_INSTANCE_COUNT > 1)
/* Implementation of UART1 handler named in startup code. */
C_FUNC void LPUART2_IRQHandler(void)
{
	LPUART_DRV_IRQHandler(2);
}
#endif

#if (LPUART_INSTANCE_COUNT > 2)
/* Implementation of UART2 handler named in startup code. */
C_FUNC void LPUART3_IRQHandler(void)
{
	LPUART_DRV_IRQHandler(3);
}
#endif

#if (LPUART_INSTANCE_COUNT > 3)
/* Implementation of UART3 handler named in startup code. */
C_FUNC void LPUART4_IRQHandler(void)
{
	LPUART_DRV_IRQHandler(4);
}
#endif

#if (LPUART_INSTANCE_COUNT > 4)
/* Implementation of UART4 handler named in startup code. */
C_FUNC void LPUART5_IRQHandler(void)
{
	LPUART_DRV_IRQHandler(5);
}
#endif

#if (LPUART_INSTANCE_COUNT > 5)
/* Implementation of UART5 handler named in startup code. */
C_FUNC void LPUART6_IRQHandler(void)
{
	LPUART_DRV_IRQHandler(6);
}
#endif

#if (LPUART_INSTANCE_COUNT > 6)
/* Implementation of UART5 handler named in startup code. */
#ifndef FS_BEATS_IO
C_FUNC void LPUART7_IRQHandler(void)
{
    LPUART_DRV_IRQHandler(7);
}
#endif
#endif

#if (LPUART_INSTANCE_COUNT > 7)
/* Implementation of UART5 handler named in startup code. */
C_FUNC void LPUART8_IRQHandler(void)
{
	LPUART_DRV_IRQHandler(8);
}

#endif
