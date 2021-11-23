
#include "baseplate.h"

#include "boards/board-api/bapi_uart.h"
#include "cmsis-driver/Driver_USART.h"
#include "console_usart_filter.h"

/**
 * \file
 * @brief This file implements the Console USART filter.
 */

/**
 * \defgroup _console_usart_filter Internals
 * @ingroup console_usart_filter
 * @brief Console USART Filter Internals (non-exposed functions, types,
 * variables).
 */

/**
 * \addtogroup _console_usart_filter
 */
/*@{*/
/**
 * @brief Name space for buffer filter internals (non-exposed functions,
 * types, variables).
 */
namespace _consoleUsartFilter {

/**
 * @ingroup _console_usart_filter
 * @brief
 * The data type to store the original hook functions.
 */
typedef struct replacedHooks {
  ReceiveFunction_t Receive;
} replacedHooks_t;


/**
 * \ingroup _console_usart_filter
 * \brief This structure declares all data that the filter needs to operate.
 */
struct UsartFilterData {
  replacedHooks_t m_replacedHookFunctions;
};

/**
 * @ingroup _console_usart_filter
 * \brief Variable storing the FilterData for all USARTs.
 */
STATIC struct UsartFilterData _usartFilterData[bapi_E_UartCount];

/**
 * @ingroup _console_usart_filter
 * @brief This hook function handles the special console receive filtering.
 *
 * The specialty of the console receive filter is, that is always
 * receives just 1 byte, even if the client has claimed to wait for more.
 */
STATIC int32_t Receive(const enum bapi_E_UartIndex_ uartIndex, void *data, uint32_t num) {
  (*_usartFilterData[uartIndex].m_replacedHookFunctions.Receive)(uartIndex, data, 1);
  return 0;
}

/**
 * @ingroup _console_usart_filter
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
    bapi_uart_startTx(uartIndex);
    retval = ARM_DRIVER_OK;
  }
  bapi_irq_exitCritical();

  return retval;
}

/**
 * \ingroup _console_usart_filter
 * \brief Get buffer driver hooks.
 *
 * Provide the hook functions of this filter in a \ref driver_usart_Hooks
 * structure.
 * Hook functions that are not part of this filter, will be null in this
 * returned structure.
 */
STATIC const struct driver_usart_Hooks _hookFunctions = {
  NULL,
  NULL,
  NULL,
  Receive,
  Send,
  NULL
};

} /* namespace _consoleUsartFilter */

/*@} addtogroup _console_usart_filter*/

/* This is the hook function pointer that we use to indicate that the driver is hooked or under transition from
 * hooked to unhooked or vice versa. */
typedef ReceiveFunction_t HookedIndicatorFunction_t;

C_INLINE bool isHookedOrTransitioning(enum bapi_E_UartIndex_ uartIndex) {
  return _consoleUsartFilter::_usartFilterData[uartIndex].m_replacedHookFunctions.Receive != 0;
}

C_INLINE HookedIndicatorFunction_t setTransitioning(enum bapi_E_UartIndex_ uartIndex) {
  HookedIndicatorFunction_t retval = _consoleUsartFilter::_usartFilterData[uartIndex].m_replacedHookFunctions.Receive;
  _consoleUsartFilter::_usartFilterData[uartIndex].m_replacedHookFunctions.Receive = _consoleUsartFilter::Receive;
  return retval;
}

C_INLINE void setUnhooked(enum bapi_E_UartIndex_ uartIndex) {
  _consoleUsartFilter::_usartFilterData[uartIndex].m_replacedHookFunctions.Receive = 0;
}

int32_t console_driver_hookDriver(
  enum bapi_E_UartIndex_ uartIndex
) {

  bapi_irq_enterCritical();
  if(!isHookedOrTransitioning(uartIndex)) {

    /* Make the oldReceiveFunction nonzero, so that no other thread can step in here. */
    setTransitioning(uartIndex);
    bapi_irq_exitCritical();


    struct driver_usart_Hooks _replacedHooks_;
    int32_t retval = driver_usart_setHooks(uartIndex, &_consoleUsartFilter::_hookFunctions, &_replacedHooks_);

    if( retval == ARM_DRIVER_OK ) {
      /* We just need the old Receive function, but don't care about the old send function. */
      _consoleUsartFilter::_usartFilterData[uartIndex].m_replacedHookFunctions.Receive = _replacedHooks_.Receive;
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
