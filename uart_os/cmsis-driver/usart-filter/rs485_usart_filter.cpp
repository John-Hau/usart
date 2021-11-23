
#include "baseplate.h"

#include "boards/board-api/bapi_uart.h"
#include "cmsis-driver/Driver_USART.h"
#include "rs485_usart_filter.h"

/**
 * \file
 * @brief This file implements the Console USART filter.
 */

/**
 * \defgroup _rs485_usart_filter Internals
 * @ingroup rs485_usart_filter
 * @brief rs485 USART Filter Internals (non-exposed functions, types,
 * variables).
 */

/**
 * \addtogroup _rs485_usart_filter
 */
/*@{*/
/**
 * @brief Name space for buffer filter internals (non-exposed functions,
 * types, variables).
 */
namespace _rs485UsartFilter {

/**
 * @ingroup _rs485_usart_filter
 * @brief
 * The data type to store the original hook functions.
 */
typedef struct replacedHooks {
  UninitializeFunction_t Uninitialize;
  InitializeFunction_t Initialize;
  SendFunction_t Send;
} replacedHooks_t;


/**
 * \ingroup _rs485_usart_filter
 * \brief This structure declares all data that the filter needs to operate.
 */
struct UsartFilterData {
  _driver_usartDriverHookSignalEvent_t m_signalEventCallback;
  replacedHooks_t m_replacedHookFunctions;
};

/**
 * \ingroup _rs485_usart_filter
 * \brief Variable storing the FilterData for all USARTs.
 */
STATIC struct UsartFilterData _usartFilterData[bapi_E_UartCount];


/**
 * @ingroup _rs485_usart_filter
 * @brief This hook function handles the special console send filtering.
 *
 * The specialty of the console send filter is, that it sends with
 * \ref bapi_uart_E_TxMode_CRLF mode.
 */
STATIC int32_t Send(const enum bapi_E_UartIndex_ uartIndex, const void *data, uint32_t num) {
  ASSERT(data);
  ASSERT(num);

  if(!data || !num) {
    return ARM_DRIVER_ERROR_PARAMETER;
  }

  bapi_uart_TransmissionState* transmissionState = driver_usart_getTransmissionState(uartIndex);

  int32_t retval = ARM_DRIVER_ERROR_BUSY;

  bapi_irq_enterCritical();

  if( !bapi_uart_isInUse(transmissionState) ) {
    bapi_uart_init_TransmissionState(transmissionState, data, num, bapi_uart_E_TxMode_CRLF);

    /* We switch off the UART receiver, flush the FIFO and switch
     * on the RS485 Transmitter when Transfer is complete. */
    bapi_uart_disableReceiver(uartIndex);
    bapi_uart_flushRxFifo(uartIndex);
    bapi_uart_setInterfaceFlag(uartIndex, bapi_E_RS485_EnableTransmitter, true);

    bapi_uart_startTx(uartIndex);
    retval = ARM_DRIVER_OK;
  }

  bapi_irq_exitCritical();

  return retval;
}

/*------------------------------------------------------------------------*//**
 * \ingroup _rs485_usart_filter
 * \brief Our Signal Event callback
 */
STATIC void driver_usart_onEvent(const enum bapi_E_UartIndex_ uartIndex, uint32_t event) {

  if(event == ARM_USART_EVENT_TX_COMPLETE) {

    /* We switch off the RS485 Transmitter and switch on the UART
     * receiver when Transfer is complete. */
    bapi_uart_setInterfaceFlag(uartIndex, bapi_E_RS485_EnableTransmitter, false);
    bapi_uart_enableReceiver(uartIndex);
  }

  if(_usartFilterData[uartIndex].m_signalEventCallback) {

    /* We just call the original Signal Event callback. */
    (*_usartFilterData[uartIndex].m_signalEventCallback)(uartIndex, event);
  }
}

/*------------------------------------------------------------------------*//**
 * \ingroup _rs485_usart_filter
 * \brief Test if driver is already initialized for a USART.
 * @return true, if the driver is already initialized, otherwise false.
 */
C_INLINE bool isInitialized(
  const enum bapi_E_UartIndex_ uartIndex /**< [in] The USART to obtain the initialization state from. */
  ) {
  return (_usartFilterData[uartIndex].m_signalEventCallback != 0);
}

/*------------------------------------------------------------------------*//**
 * \ingroup _rs485_usart_filter
 * \brief Our Initialize hook function
 */
STATIC int32_t Initialize(const enum bapi_E_UartIndex_ uartIndex, _driver_usartDriverHookSignalEvent_t driverHook_cb_event) {

  /* Here we could do some own additional pre - initialization. */

  int32_t retval = ARM_DRIVER_ERROR;

  /* Assert that Initialize isn't called a second time, when we are already initialized. */
  ASSERT(!isInitialized(uartIndex));

  if(!isInitialized(uartIndex)) {
    if(_usartFilterData[uartIndex].m_replacedHookFunctions.Initialize){
      /* Here we hook our own Signal Event callback */
      retval = (*_usartFilterData[uartIndex].m_replacedHookFunctions.Initialize)(uartIndex, driver_usart_onEvent);
    }

    if(retval == ARM_DRIVER_OK) {
      /* Our initialize function saves the original Signal Event callback that was passed to us. */
      _usartFilterData[uartIndex].m_signalEventCallback = driverHook_cb_event;
    }
  }
  return retval;
}

/*-------------------------------------------------------------------------*//*
 * \ingroup _rs485_usart_filter
 * \brief Our Uninitialize hook function
 */
STATIC int32_t Uninitialize(const enum bapi_E_UartIndex_ uartIndex) {

  /* Here we could do some own additional pre - uninitialization. */

  int32_t retval = ARM_DRIVER_ERROR;

  if (_usartFilterData[uartIndex].m_replacedHookFunctions.Uninitialize) {

    /* Call the original Uninitialize function. */
    retval = (*_usartFilterData[uartIndex].m_replacedHookFunctions.Uninitialize)(uartIndex);

    if (retval == ARM_DRIVER_OK) {
      /* Our Signal Event call back is de-installed, now we can forget the original
       * Signal Event callback. */
      _usartFilterData[uartIndex].m_signalEventCallback = 0;
    }
  }

  return retval;
}

/**
 * \ingroup _rs485_usart_filter
 * \brief Get buffer driver hooks.
 *
 * Provide the hook functions of this filter in a \ref driver_usart_Hooks
 * structure.
 * Hook functions that are not part of this filter, will be null in this
 * returned structure.
 */
STATIC const struct driver_usart_Hooks _hookFunctions = {
  Initialize,
  Uninitialize,
  NULL,  /* We don't hook an own Control(..) hook function. */
  NULL,
  Send,
  NULL
};


} /* namespace _consoleUsartFilter */

/*@} addtogroup _rs485_usart_filter*/


/* This is the hook function pointer that we use to indicate that the driver is hooked or under transition from
 * hooked to unhooked or vice versa. */
typedef UninitializeFunction_t HookedIndicatorFunction_t;

C_INLINE bool isHookedOrTransitioning(enum bapi_E_UartIndex_ uartIndex) {
  return _rs485UsartFilter::_usartFilterData[uartIndex].m_replacedHookFunctions.Uninitialize != 0;
}

C_INLINE HookedIndicatorFunction_t setTransitioning(enum bapi_E_UartIndex_ uartIndex) {
  HookedIndicatorFunction_t retval = _rs485UsartFilter::_usartFilterData[uartIndex].m_replacedHookFunctions.Uninitialize;
  _rs485UsartFilter::_usartFilterData[uartIndex].m_replacedHookFunctions.Uninitialize = _rs485UsartFilter::Uninitialize;
  return retval;
}

C_INLINE bool isTransitioning(enum bapi_E_UartIndex_ uartIndex) {
  return _rs485UsartFilter::_usartFilterData[uartIndex].m_replacedHookFunctions.Uninitialize == _rs485UsartFilter::Uninitialize;
}

C_INLINE void setUnhooked(enum bapi_E_UartIndex_ uartIndex) {
  bapi_irq_enterCritical();
  _rs485UsartFilter::_usartFilterData[uartIndex].m_replacedHookFunctions.Send = 0;
  _rs485UsartFilter::_usartFilterData[uartIndex].m_replacedHookFunctions.Initialize = 0;
  _rs485UsartFilter::_usartFilterData[uartIndex].m_replacedHookFunctions.Uninitialize = 0;
  bapi_irq_exitCritical();
}

C_INLINE void abortUnhook(enum bapi_E_UartIndex_ uartIndex, HookedIndicatorFunction_t restore) {
  _rs485UsartFilter::_usartFilterData[uartIndex].m_replacedHookFunctions.Uninitialize = restore;
}



int32_t rs485_driver_hookDriver(
  enum bapi_E_UartIndex_ uartIndex
) {

  bapi_irq_enterCritical();
  if(!isHookedOrTransitioning(uartIndex)) {

    if(isTransitioning(uartIndex)) {
      bapi_irq_exitCritical();
      return ARM_DRIVER_ERROR_BUSY;
    }

    /* Make the _replacedUninitializeHookFunction nonzero, so that no other thread can step in here. */
    setTransitioning(uartIndex);

    bapi_irq_exitCritical();

    struct driver_usart_Hooks _replacedHooks_;
    int32_t retval = driver_usart_setHooks(uartIndex, &_rs485UsartFilter::_hookFunctions, &_replacedHooks_);

    if( retval == ARM_DRIVER_OK ) {
      /* Save the old hook functions. */
      _rs485UsartFilter::_usartFilterData[uartIndex].m_replacedHookFunctions.Uninitialize = _replacedHooks_.Uninitialize;
      _rs485UsartFilter::_usartFilterData[uartIndex].m_replacedHookFunctions.Initialize = _replacedHooks_.Initialize;
      _rs485UsartFilter::_usartFilterData[uartIndex].m_replacedHookFunctions.Send = _replacedHooks_.Send;
    } else {
      /* Error: Indicate that the driver is uninitialized. */
      setUnhooked(uartIndex);
    }

    return retval;
  }

  bapi_irq_exitCritical();

  /* The driver is already hooked. */
  return ARM_DRIVER_ERROR;
}

int32_t rs485_driver_unhookDriver(
    enum bapi_E_UartIndex_ uartIndex
  ) {
  bapi_irq_enterCritical();

  if(isHookedOrTransitioning(uartIndex)) {

    if(isTransitioning(uartIndex)) {
      bapi_irq_exitCritical();
      return ARM_DRIVER_ERROR_BUSY;
    }

    HookedIndicatorFunction_t originalHookFunction = setTransitioning(uartIndex);

    bapi_irq_exitCritical();

    if(_rs485UsartFilter::_usartFilterData[uartIndex].m_signalEventCallback == 0) {
      _rs485UsartFilter::_usartFilterData[uartIndex].m_replacedHookFunctions.Send = 0;
      _rs485UsartFilter::_usartFilterData[uartIndex].m_replacedHookFunctions.Initialize = 0;
      _rs485UsartFilter::_usartFilterData[uartIndex].m_replacedHookFunctions.Uninitialize = 0;
      return ARM_DRIVER_OK;
    }

    /* Driver is still initialized, cannot unhook. Client must Uninitialize before unhook. */
    abortUnhook(uartIndex, originalHookFunction);
    return ARM_DRIVER_ERROR;
  }

  bapi_irq_exitCritical();

  /* Is already unhooked. */
  return ARM_DRIVER_ERROR;
}
