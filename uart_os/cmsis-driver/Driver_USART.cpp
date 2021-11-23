/*
 *  $HeadURL: $
 *
 *  $Date: $
 *  $Author: $
 */


/**
 * /file
 * /brief This file Implements the cmsis-driver API for USARTs and implements the functions
 * that provide instances of ARM_DRIVER_USART structures for each USART that is
 * defined by the board API (refer to bapi_uart.h). It also Implements the functions that
 * allow hooking own driver functions into the those USART drivers.
 */


#include "baseplate.h"

#include "cmsis_os2.h"                    // ::CMSIS:RTOS2
#include "boards/board-api/bapi_uart.h"
#include "cmsis-driver/Driver_USART.h"
#include "boards/board-api/bapi_atomic.h"

extern osMessageQueueId_t m_msg_console;

char tttbuf[256]={0};
int tttcnt=0;


#ifndef ARM_USART_CFG_DEBUG_ABORT_RX_TX
  #define ARM_USART_CFG_DEBUG_ABORT_RX_TX 0
#endif

#define ARM_USART_DRV_VERSION    ARM_DRIVER_VERSION_MAJOR_MINOR(2, 0)  /* driver version */

namespace Driver_USART {

/**
 * \ingroup _cmsis_driver_usart
 *  Driver Version
 */
STATIC const ARM_DRIVER_VERSION driverVersion = {
    ARM_USART_API_VERSION,
    ARM_USART_DRV_VERSION
};

/**
 * \ingroup _cmsis_driver_usart
 * \brief
 * Holds the receive state of the message currently being received. This
 * is the number of characters to be received until the receive session
 * is completed as well as the actual received number of characters.
 *
 * \note Members in a structure are sequenced from largest to smallest for optimal memory usage.
 */
struct DriverReceiveState {

  uint8_t* m_buffer; /**< Buffer where the received characters are stored. */

  /** The number of characters to be received until the
   * ARM_USART_EVENT_RECEIVE_COMPLETE signal should be
   * fired. */
  bapi_uart_MaxFrameSize_t m_requestedCount;

  /** The actual number of received characters. */
  bapi_uart_MaxFrameSize_t m_currentCount;

  /**
   * \brief Retrieve whether there is currently a receive session in progress.
   * \return true, if there is a receive session in progress, otherwise false.
   */
  inline bool isInUse()const {
    return atomic_Get(&m_requestedCount) != 0;
  }

  /**
   * \brief Invalidate this receive state.
   *
   * This function will be called upon a completion or abort of a receive
   * session.
   */
  inline void invalidate() {
    /* Setting a pointer to zero should always be atomic. */
    m_buffer = 0;

    /* !!!
     * Note: After an receive session abort, the client might still want to
     * know how many characters have been received. So we don't set
     * m_currentCount to 0 here.
     */

    /* Invalidate the variable that determines the "inUse" state at last. */
    atomic_Set(&m_requestedCount, static_cast<bapi_uart_MaxFrameSize_t>(0));
  }

  /**
   * \brief Initialize this receive state for a new receive session.
   */
  inline void init(void* buffer, bapi_uart_MaxFrameSize_t requestedCount) {
    /* Set "inUse" first */
    atomic_Set(&m_requestedCount, requestedCount);
    m_currentCount = 0;
    m_buffer = static_cast<uint8_t*>(buffer);
  }

};

/**
 * \ingroup _cmsis_driver_usart
 * \brief
 * A structure that a USART driver needs for a single USART to perform send and receive operations.
 */
struct DriverState {
  _driver_usartDriverHookSignalEvent_t m_signalEventCallback;  /**< The event callback to fire USART events. */
  bapi_uart_TransmissionState          m_transmissionState;    /**< Transmission state */
  DriverReceiveState                   m_receiveState;         /**< Receive state */
};


/**
 * \ingroup _cmsis_driver_usart
 * For each USART the runtime data to perform send and receive operations.
 * \note It is assumed that the compiler implicitly initializes the whole
 * array with zeroes.
 */
STATIC struct DriverState driverState[bapi_E_UartCount];


/**
 * \ingroup _cmsis_driver_usart
 * \brief
 * The callback that will be called by the bapi_uart module to obtain the
 * current transmission state for a particular USART.
 */
STATIC bapi_uart_TransmissionState* myBapiGetTxState_ISRCallback(
  const enum bapi_E_UartIndex_ uartIndex /**< [in] The USART that has invoked this ISR callback. */
  ){
  return &driverState[uartIndex].m_transmissionState;
}

/**
 * \ingroup _cmsis_driver_usart
 * \brief
 * The callback that will be called by the bapi_uart module upon a
 * TRANSMISSION COMPLETE event.
 */
STATIC void myBapiTxComplete_ISRCallback(
  struct bapi_uart_TransmissionState* transmissionState, /**< [in] The transmission state that the ISR
                                                          * associated with the callback event. */

  uint32_t event /**< [in] ARM_USART_EVENT_SEND_COMPLETE in case that all bytes have been moved to the
                  * UART send register or UART FIFO. ARM_USART_EVENT_TX_COMPLETE in case that the last
                  * byte was physically transmitted. */
 ){
  ASSERT(transmissionState->m_uartIndex >= 0);
  ASSERT(transmissionState->m_uartIndex < bapi_E_UartCount);

  _driver_usartDriverHookSignalEvent_t driverHook_cb = driverState[transmissionState->m_uartIndex].m_signalEventCallback;
  /** Call the arm driver signal event callback */

  if(driverHook_cb) {
    (driverHook_cb)(transmissionState->m_uartIndex, event);
  }

  if(event == ARM_USART_EVENT_SEND_COMPLETE) {
    /* All bytes are in the UART send register of Fifo now, so we must reset the transmission state.*/
    bapi_uart_invalidateTransmissionState(transmissionState);
  }
}

C_INLINE void dataReceivedComplete(const enum bapi_E_UartIndex_ uartIndex)
  {
  /* We have sufficient data received, so call the arm driver signal event callback */
  _driver_usartDriverHookSignalEvent_t driverHook_cb = driverState[uartIndex].m_signalEventCallback;
  if (driverHook_cb) {
    (driverHook_cb)(uartIndex, ARM_USART_EVENT_RECEIVE_COMPLETE);
  }
  driverState[uartIndex].m_receiveState.invalidate();
}

C_INLINE void signalRxOverflow(const enum bapi_E_UartIndex_ uartIndex)
  {
  _driver_usartDriverHookSignalEvent_t driverHook_cb = driverState[uartIndex].m_signalEventCallback;
  if (driverHook_cb) {
    (driverHook_cb)(uartIndex, ARM_USART_EVENT_RX_OVERFLOW);
  }
}

/**
 * \ingroup _cmsis_driver_usart
 * \brief The callback that will be called by the bapi_uart module to dispose
 * one or more received characters.
 *
 * \return The number of consumed characters.
 */
STATIC bapi_uart_MaxFrameSize_t dataReceived_ISRCallback(
 const enum bapi_E_UartIndex_ uartIndex,   /**< [in] The UART that has the received character(s). */
 const uint32_t errorEvents,
 const uint8_t rx_chars[],                 /**< [in] The received character(s) */
 bapi_uart_MaxFrameSize_t count            /**< [in] The number of received characters */
  ){


#if 1
if(uartIndex==bapi_E_Uart4)
{
	 if(rx_chars) {

		 tttbuf[tttcnt]=rx_chars[0];

		  if(tttbuf[tttcnt] == '\n')
		  {
			  tttbuf[tttcnt + 1] = '\0';
			  osMessageQueuePut(m_msg_console, tttbuf, 0, 0 );
			  tttcnt =0;
//for(int i=0;i<256;i++)
	//tttbuf[i]=0;
		  }
		  else
		  {

			  tttcnt ++;
			  if(tttcnt >(256-2))tttcnt=0;
		  }
	 }
	 return 0;
}

#endif


#if 1
  if(rx_chars) {
    /* Data received interrupt service */
    struct DriverReceiveState* receiveState = &driverState[uartIndex].m_receiveState;

    if(receiveState->isInUse()) {
      ASSERT(receiveState->m_buffer);
      bapi_uart_MaxFrameSize_t gap = receiveState->m_requestedCount - receiveState->m_currentCount;

      bapi_uart_MaxFrameSize_t i = 0;
      while((i < count) && gap--) {
        receiveState->m_buffer[receiveState->m_currentCount++] = rx_chars[i++];
      }

      if(receiveState->m_currentCount >= receiveState->m_requestedCount ) {
        /* We have sufficient data received */

        /* Call the arm driver signal event callback */
        dataReceivedComplete(uartIndex);
      }
      return i;
    }

    /* We couldn't consume a character, because we are not in Receive mode and
     * don't have a buffer to place the received characters. */
    signalRxOverflow(uartIndex);
  }

  if(errorEvents) {
    /* Receive error interrupt service */
    _driver_usartDriverHookSignalEvent_t driverHook_cb = driverState[uartIndex].m_signalEventCallback;
    if (driverHook_cb) {
      (driverHook_cb)(uartIndex, errorEvents);
    }
  }

  return 0;


#endif
}



/**
 * \ingroup _cmsis_driver_usart
 * \brief Retrieve the initialization state of a USART.
 * \return true, if the driver is initialized, otherwise false;
 */
C_INLINE bool isInitialized(
  const enum bapi_E_UartIndex_ uartIndex /**< [in] The USART to obtain the initialization state from. */
  ) {
  return (driverState[uartIndex].m_signalEventCallback != 0);
}


/**
 * \ingroup _cmsis_driver_usart
 * \brief
 * Abort the transmit upon faking an ARM_USART_EVENT_SEND_COMPLETE, ARM_USART_EVENT_TX_COMPLETE sequence.
 * \note This function is declared inline, because there is only one caller.
 */
C_INLINE void abortSend(
 const enum bapi_E_UartIndex_ uartIndex /**< [in] The uart for which to abort the transmit. */
  ){

  /* Ensure Tx Interrupt disabled */
  bapi_uart_enterCritical(uartIndex, bapi_uart_IRQT_TX);

  /* if there is an transmission in progress */
  if (bapi_uart_isInUse(&driverState[uartIndex].m_transmissionState)) {

    bapi_uart_flushTxFifo(uartIndex);

    /* Fake a ARM_USART_EVENT_SEND_COMPLETE, ARM_USART_EVENT_TX_COMPLETE sequence */
    myBapiTxComplete_ISRCallback(&driverState[uartIndex].m_transmissionState, ARM_USART_EVENT_SEND_COMPLETE);
    myBapiTxComplete_ISRCallback(&driverState[uartIndex].m_transmissionState, ARM_USART_EVENT_TX_COMPLETE);
  }

  /* Allow Tx Interrupt */
  bapi_uart_exitCritical(uartIndex, bapi_uart_IRQT_TX);
}


/**
 * \ingroup _cmsis_driver_usart
 * \brief
 * Abort the receive upon faking an ARM_USART_EVENT_RECEIVE_COMPLETE.
 * \note This function is declared inline, because there is only one caller.
 */
C_INLINE void abortReceive(
 const enum bapi_E_UartIndex_ uartIndex /**< [in] The UART for which to abort the receive. */
  ){

  /* Ensure Rx Interrupt disabled */
  bapi_uart_enterCritical(uartIndex, bapi_uart_IRQT_RX);

  /* if there is an receive in progress */
  if (driverState[uartIndex].m_receiveState.isInUse()) {

    /* Note: We don't flush the Rx Fifo automatically here, because the
     *  data words in the Rx Fifo might be important for the next
     *  Receive operation. Otherwise, the client can explicitly call
     *  the function bapi_uart_flushRxFifo. */

    /* Fake a ARM_USART_EVENT_RECEIVE_COMPLETE */
    #if ARM_USART_CFG_DEBUG_ABORT_RX_TX > 0
      /* Place text "ABORT" in the receive buffer. */
      static const char text[] = "ABORT"; /* Copy text abort for better debugging. */
      size_t textSize = MIN(ARRAY_SIZE(text)-1, driverState[uartIndex].m_receiveState.m_requestedCount);
      MEMCPY(driverState[uartIndex].m_receiveState.m_buffer, text, textSize);
      driverState[uartIndex].m_receiveState.m_currentCount = textSize;
    #endif

    driverState[uartIndex].m_receiveState.m_requestedCount = driverState[uartIndex].m_receiveState.m_currentCount;
    dataReceivedComplete(uartIndex);
  }

  /* Allow Rx Interrupt */
  bapi_uart_exitCritical(uartIndex, bapi_uart_IRQT_RX);
}

} /* namespace Driver_USART */

/**
 * \ingroup _cmsis_driver_usart
 * \brief Namespace for ARM USART driver corresponding interface functions
 * with an additional UART index parameter.
 */
class ARM_USART_idx {
public:

/**
 * \name ARM_USART_idx Functions.
 * ARM USART driver corresponding interface functions with an additional
 * UART index parameter.
 */
/**@{*/

/**
 * \brief Implements the functionality of CMSIS Driver API function
 * ARM_USART::Control(uint32_t control, uint32_t arg)
 * ,but with an additional parameter <em>uartIndex</em>, in order to decide on
 * which USART the function should be carried out.
 * \warning This function must not be called from within an ISR context.
 */
static int32_t Control(const enum bapi_E_UartIndex_ uartIndex, uint32_t control, uint32_t arg) {
  ASSERT_DEBUG(!bapi_irq_isInterruptContext())

  int32_t retval = ARM_DRIVER_ERROR_UNSUPPORTED;

  switch(control & ARM_USART_CONTROL_Msk) {
    case ARM_USART_MODE_ASYNCHRONOUS:
      retval = bapi_uart_configure(uartIndex, arg, control);
      break;

    case ARM_USART_ABORT_SEND:
      Driver_USART::abortSend(uartIndex);
      retval = ARM_DRIVER_OK;
      break;

    case ARM_USART_ABORT_RECEIVE:
      Driver_USART::abortReceive(uartIndex);
      retval = ARM_DRIVER_OK;
      break;

    case ARM_USART_ABORT_TRANSFER:
      Driver_USART::abortReceive(uartIndex);
      Driver_USART::abortSend(uartIndex);
      retval = ARM_DRIVER_OK;
      break;

      /* Currently non supported modes follow. */
    case ARM_USART_MODE_SYNCHRONOUS_MASTER:
    case ARM_USART_MODE_SYNCHRONOUS_SLAVE:
    case ARM_USART_MODE_SINGLE_WIRE:
    case ARM_USART_MODE_IRDA:
    case ARM_USART_MODE_SMART_CARD:
      break;

      /* Currently non supported misc modes follow. */
    case ARM_USART_SET_DEFAULT_TX_VALUE:
    case ARM_USART_SET_IRDA_PULSE:
    case ARM_USART_SET_SMART_CARD_GUARD_TIME:
    case ARM_USART_SET_SMART_CARD_CLOCK:
    case ARM_USART_CONTROL_SMART_CARD_NACK:
    case ARM_USART_CONTROL_TX:
    case ARM_USART_CONTROL_RX:
    case ARM_USART_CONTROL_BREAK:
      break;

  }

  return retval; // TODO: implement other control codes.
}


/**
 * \brief Implements the functionality of CMSIS Driver API function
 * ARM_USART::Initialize(ARM_USART_SignalEvent_t)
 * ,but with an additional parameter <em>uartIndex</em>, in order to decide on
 * which USART the function should be carried out.
 */
static int32_t Initialize(const enum bapi_E_UartIndex_ uartIndex, _driver_usartDriverHookSignalEvent_t driverHook_cb_event) {
  bapi_irq_enterCritical();

  int32_t retval = ARM_DRIVER_ERROR;

  /* Assert that Initialize isn't called a second time, when we are already initialized. */
  ASSERT(!Driver_USART::isInitialized(uartIndex));

  if (!Driver_USART::isInitialized(uartIndex)) {

    if (Driver_USART::driverState[uartIndex].m_signalEventCallback == driverHook_cb_event) {
      retval = ARM_DRIVER_OK;
    }
    else {
      if (Driver_USART::driverState[uartIndex].m_signalEventCallback == 0) {

        bapi_uart_setMsgTransmission_ISRCallbacks(uartIndex, Driver_USART::myBapiTxComplete_ISRCallback
          , Driver_USART::myBapiGetTxState_ISRCallback);

        bapi_uart_setDataReceived_ISRCallback(uartIndex, Driver_USART::dataReceived_ISRCallback);

        Driver_USART::driverState[uartIndex].m_transmissionState.m_uartIndex = uartIndex; /** initialize backward reference. */
        Driver_USART::driverState[uartIndex].m_signalEventCallback = driverHook_cb_event;
        retval = ARM_DRIVER_OK;
      }
    }
  }
  bapi_irq_exitCritical();

  return retval;
}

/**
 * \brief Implements the functionality of CMSIS Driver API function
 * ARM_USART::Uninitialize()
 * ,but with an additional parameter <em>uartIndex</em>, in order to decide on
 * which USART the function should be carried out.
 */
static int32_t Uninitialize(const enum bapi_E_UartIndex_ uartIndex) {

  bapi_uart_unconfigure(uartIndex);

  /* Abort SEND and RECEIVE using driver_usart_getDriver(), to ensure that we call the hook. */
  ARM_DRIVER_USART* usartDriver = driver_usart_getDriver(uartIndex);
  int32_t abortResult = usartDriver->Control(ARM_USART_ABORT_TRANSFER, 0);

  if( abortResult == ARM_DRIVER_OK ) {
    bapi_uart_setMsgTransmission_ISRCallbacks(uartIndex, Driver_USART::myBapiTxComplete_ISRCallback, 0);

    bapi_uart_setDataReceived_ISRCallback(uartIndex, 0);

    /* Setting a pointer is assumed to be atomic. */
    Driver_USART::driverState[uartIndex].m_signalEventCallback = 0;
  }

  return abortResult;
}


/**
 * \brief Implements the functionality of CMSIS Driver API function
 * ARM_USART::Transfer(const void *, void *, uint32_t)
 * ,but with an additional parameter <em>uartIndex</em>, in order to decide on
 * which USART the function should be carried out.
 */
static int32_t Transfer(const enum bapi_E_UartIndex_ uartIndex, const void *data_out, void *data_in, uint32_t num)
{
  return ARM_DRIVER_ERROR_UNSUPPORTED; // TODO: implement
}

/**
 * \brief Implements the functionality of CMSIS Driver API function
 * ARM_USART::Send(const void *, uint32_t)
 * ,but with an additional parameter <em>uartIndex</em>, in order to decide on
 * which USART the function should be carried out.
 */
static int32_t Send(const enum bapi_E_UartIndex_ uartIndex, const void *data, uint32_t num)
{
  if(!data || !num) {
    return ARM_DRIVER_ERROR_PARAMETER;
  }


  bapi_uart_TransmissionState* transmissionState = &Driver_USART::driverState[uartIndex].m_transmissionState;

  int32_t retval = ARM_DRIVER_ERROR_BUSY;
  bapi_irq_enterCritical();

  enum bapi_E_UartMode_ uartMode = bapi_uart_getMode(uartIndex);
  if( !bapi_uart_isInUse(transmissionState) ) {
    bapi_uart_init_TransmissionState(transmissionState, data, num, bapi_uart_E_TxMode_Transparent);
    bapi_uart_startTx(uartIndex);
    retval = ARM_DRIVER_OK;
  }
  bapi_irq_exitCritical();

  return retval;
}

/**
 * \brief Implements the functionality of CMSIS Driver API function
 * ARM_USART::Receive(void *, uint32_t)
 * ,but with an additional parameter <em>uartIndex</em>, in order to decide on
 * which USART the function should be carried out.
 */
static int32_t Receive(const enum bapi_E_UartIndex_ uartIndex, void *data, uint32_t num) {

  if(!data || !num) {
    return ARM_DRIVER_ERROR_PARAMETER;
  }

  Driver_USART::DriverReceiveState* receiveState = &Driver_USART::driverState[uartIndex].m_receiveState;

  int32_t retval = ARM_DRIVER_ERROR_BUSY;
  bapi_irq_enterCritical();
  enum bapi_E_UartMode_ uartMode = bapi_uart_getMode(uartIndex);

  if(!receiveState->isInUse()) {
    receiveState->init(data, num);
    retval = ARM_DRIVER_OK;
  }

  bapi_irq_exitCritical();

  return retval;
}


/**
 * \brief Implements the functionality of CMSIS Driver API function
 * ARM_USART::GetStatus(void)
 * ,but with an additional parameter <em>uartIndex</em>, in order to decide on
 * which USART the function should be carried out.
 */
static ARM_USART_STATUS GetStatus(const enum bapi_E_UartIndex_ uartIndex) {
  ARM_USART_STATUS status = {0};
  status.rx_busy = Driver_USART::driverState[uartIndex].m_receiveState.isInUse();
  status.tx_busy = bapi_uart_isInUse(&Driver_USART::driverState[uartIndex].m_transmissionState);

  // TODO: support all status flags.
  return status;
}

/**
 * \brief Implements the functionality of CMSIS Driver API function
 * ARM_USART::GetTxCount(void)
 * ,but with an additional parameter <em>uartIndex</em>, in order to decide on
 * which USART the function should be carried out.
 */
static int32_t GetTxCount(const enum bapi_E_UartIndex_ uartIndex) {
  return ARM_DRIVER_ERROR_UNSUPPORTED; // TODO: implement
}

/**
 * \brief Implements the functionality of CMSIS Driver API function
 * ARM_USART::GetRxCount(void)
 * ,but with an additional parameter <em>uartIndex</em>, in order to decide on
 * which USART the function should be carried out.
 */
static uint32_t GetRxCount(const enum bapi_E_UartIndex_ uartIndex)
{
  return atomic_Get(&Driver_USART::driverState[uartIndex].m_receiveState.m_currentCount);
}

/**
 * \brief Implements the functionality of CMSIS Driver API function
 * ARM_USART::PowerControl(ARM_POWER_STATE state)
 * ,but with an additional parameter <em>uartIndex</em>, in order to decide on
 * which USART the function should be carried out.
 */
static int32_t PowerControl(const enum bapi_E_UartIndex_ uartIndex, ARM_POWER_STATE state)
{
    switch (state)
    {
    case ARM_POWER_OFF:
        break;

    case ARM_POWER_LOW:
        break;

    case ARM_POWER_FULL:
        break;

    default:
        return ARM_DRIVER_ERROR_UNSUPPORTED;
    }
    return ARM_DRIVER_ERROR_UNSUPPORTED; /* TODO: Implement */
}

/**
 * \brief Implements the functionality of CMSIS Driver API function
 * ARM_USART::SetModemControl(ARM_USART_MODEM_CONTROL control)
 * ,but with an additional parameter <em>uartIndex</em>, in order to decide on
 * which USART the function should be carried out.
 */
static int32_t SetModemControl(const enum bapi_E_UartIndex_ uartIndex, ARM_USART_MODEM_CONTROL control)
{
  return ARM_DRIVER_ERROR_UNSUPPORTED;
}

/**
 * \brief Implements the functionality of CMSIS Driver API function
 * ARM_USART::GetModemStatus(void)
 * ,but with an additional parameter <em>uartIndex</em>, in order to decide on
 * which USART the function should be carried out.
 */
static ARM_USART_MODEM_STATUS GetModemStatus(const enum bapi_E_UartIndex_ uartIndex)
{
  return ARM_USART_MODEM_STATUS(); // TODO: implement
}

/**@} name ARM_USART_idx*/

}; /* class ARM_USART_idx */


/******************************************************************************
 * Default USART Hooks
 *****************************************************************************/
/**
 * \ingroup _cmsis_driver_usart
 * \brief The list of default hook functions in the sequence as they appear in
 * struct driver_usart_Hooks.
 *
 *  Must be adjusted, in case that struct driver_usart_Hooks is extended.
 */
#define _USART_DRIVER_HOOK_FUNCTIONS_DEFAULT_ { \
   ARM_USART_idx::Initialize \
  ,ARM_USART_idx::Uninitialize \
  ,ARM_USART_idx::Control \
  ,ARM_USART_idx::Receive \
  ,ARM_USART_idx::Send \
  ,ARM_USART_idx::GetStatus \
}

/**
 * \ingroup _cmsis_driver_usart
 * \brief The list of default hook functions in the sequence as they appear in
 * struct driver_usart_Hooks.
 *
 *  Must be adjusted, in case that struct driver_usart_Hooks is extended.
 */
STATIC const struct driver_usart_Hooks s_usartDriverDefaultHooks = _USART_DRIVER_HOOK_FUNCTIONS_DEFAULT_;

/**
 * \ingroup _cmsis_driver_usart
 * \brief
 * Volatile static variable that holds the current Driver hooks in RAM.
 * <em><b>All hook-able functions will be initialized here with the
 * pointers to the default functions.</b></em>.
 * This variable is of type struct driver_usart_Hooks. It is a static member of
 * struct ARM_USART<uartIndex>. The struct ARM_USART<uartIndex> is instantiated
 * for each available UART. E.g. if you have N UARTS available, there will be
 * available the following static variables at runtime:
 * \verbatim
 ARM_USART<0>::ms_hooks
 ARM_USART<1>::ms_hooks
 ARM_USART<2>::ms_hooks
 ...
 ARM_USART<N-1>::ms_hooks
 * \endverbatim
 */
STATIC struct driver_usart_Hooks s_usartDriverHooks[] = {
  _USART_DRIVER_HOOK_FUNCTIONS_DEFAULT_
#if (BAPI_HAS_USART > 1)
 ,_USART_DRIVER_HOOK_FUNCTIONS_DEFAULT_
#endif
#if (BAPI_HAS_USART > 2)
 ,_USART_DRIVER_HOOK_FUNCTIONS_DEFAULT_
#endif
#if (BAPI_HAS_USART > 3)
 ,_USART_DRIVER_HOOK_FUNCTIONS_DEFAULT_
#endif
#if (BAPI_HAS_USART > 4)
 ,_USART_DRIVER_HOOK_FUNCTIONS_DEFAULT_
#endif
#if (BAPI_HAS_USART > 5)
 ,_USART_DRIVER_HOOK_FUNCTIONS_DEFAULT_
#endif
#if (BAPI_HAS_USART > 6)
 ,_USART_DRIVER_HOOK_FUNCTIONS_DEFAULT_
#endif
#if (BAPI_HAS_USART > 7)
 ,_USART_DRIVER_HOOK_FUNCTIONS_DEFAULT_
#endif
#if (BAPI_HAS_USART > 8)
 #error "More than 8 USARTs defined. Please enhance according to the scheme above."
#endif
};


/******************************************************************************
 * ARM_DRIVER_USART instances
 *****************************************************************************/
/**
 * \ingroup _cmsis_driver_usart
 * \brief Implements \ref _ARM_DRIVER_USART.GetVersions
 */
STATIC ARM_DRIVER_VERSION ARM_USART_GetVersion(void)
{
  return Driver_USART::driverVersion;
}


/**
 * \ingroup _cmsis_driver_usart
 * \brief Implementation of the USART driver interface functions for all USARTS.
 *
 * Refer to \ref cmsis_driver_usart_instance_section and struct usartArray__
 *
 */
template<enum bapi_E_UartIndex_ uartIndex> class ARM_USART {
public:

  /* -------- hook-able functions -------- */

  /**
   * \brief Implements \ref _ARM_DRIVER_USART.Initialize
   */
  static int32_t Initialize(
    ARM_USART_SignalEvent_t cb_event) {

    return s_usartDriverHooks[uartIndex].Initialize(uartIndex, cb_event);
  }

  /**
   * \brief Implements \ref _ARM_DRIVER_USART.Uninitialize
   */
  static int32_t Uninitialize() {
    return s_usartDriverHooks[uartIndex].Uninitialize(uartIndex);
  }

  /**
   * \brief Implements \ref _ARM_DRIVER_USART.Control
   */
  static int32_t Control(uint32_t control, uint32_t arg) {
    return s_usartDriverHooks[uartIndex].Control(uartIndex, control, arg);
  }

  /**
   * \brief Implements \ref _ARM_DRIVER_USART.Send
   */
  static int32_t Send(const void *data, uint32_t num) {
    return s_usartDriverHooks[uartIndex].Send(uartIndex, data, num);
  }

  /**
   * \brief Implements \ref _ARM_DRIVER_USART.Receive
   */
  static int32_t Receive(void *data, uint32_t num) {
    return s_usartDriverHooks[uartIndex].Receive(uartIndex, data, num);
  }

  /**
   * \brief Implements \ref _ARM_DRIVER_USART.GetStatus
   */
  static ARM_USART_STATUS GetStatus() {
    return ARM_USART_idx::GetStatus(uartIndex);
  }

  /* ------ Non hook-able functions ------ */

  /**
   * \brief Implements \ref _ARM_DRIVER_USART.GetCapabilities
   */
  static struct _ARM_USART_CAPABILITIES GetCapabilities()
  {
    return *bapi_uart_getCapabilities(uartIndex);
  }

  /**
   * \brief Implements \ref _ARM_DRIVER_USART.Transfer
   */
  static int32_t Transfer(const void *data_out, void *data_in, uint32_t num) {
    return ARM_USART_idx::Transfer(uartIndex, data_out, data_in, num);
  }

  /**
   * \brief Implements \ref _ARM_DRIVER_USART.GetTxCount
   */
  static uint32_t GetTxCount()
  {
    return ARM_USART_idx::GetTxCount(uartIndex);
  }

  /**
   * \brief Implements \ref _ARM_DRIVER_USART.GetRxCount
   */
  static uint32_t GetRxCount()
  {
    return ARM_USART_idx::GetRxCount(uartIndex);
  }

  /**
   * \brief Implements \ref _ARM_DRIVER_USART.PowerControl
   */
  static int32_t PowerControl(ARM_POWER_STATE state) {
    return ARM_USART_idx::PowerControl(uartIndex, state);
  }

  /**
   * \brief Implements \ref _ARM_DRIVER_USART.SetModemControl
   */
  static int32_t SetModemControl(ARM_USART_MODEM_CONTROL control)
  {
    return ARM_USART_idx::SetModemControl(uartIndex, control);
  }

  /**
   * \brief Implements \ref _ARM_DRIVER_USART.GetModemStatus
   */
  static ARM_USART_MODEM_STATUS GetModemStatus(void)
  {
    return ARM_USART_idx::GetModemStatus(uartIndex);
  }
};



/******************************************************************************
 * ARM_DRIVER_USART: s_usartDrivers
 *****************************************************************************/

/**
 * \ingroup _cmsis_driver_usart
 * \brief The list of driver interface functions in the sequence as they appear in
 * struct _ARM_USART_DRIVER.
 */
#define _USART_DRIVER_VALUE_(uartIndex) { \
   ARM_USART_GetVersion \
  ,ARM_USART<uartIndex>::GetCapabilities \
  ,ARM_USART<uartIndex>::Initialize \
  ,ARM_USART<uartIndex>::Uninitialize \
  ,ARM_USART<uartIndex>::PowerControl \
  ,ARM_USART<uartIndex>::Send \
  ,ARM_USART<uartIndex>::Receive \
  ,ARM_USART<uartIndex>::Transfer \
  ,ARM_USART<uartIndex>::GetTxCount \
  ,ARM_USART<uartIndex>::GetRxCount \
  ,ARM_USART<uartIndex>::Control \
  ,ARM_USART<uartIndex>::GetStatus \
  ,ARM_USART<uartIndex>::SetModemControl \
  ,ARM_USART<uartIndex>::GetModemStatus \
}
#if defined (FS_IMXRTEVAL) || defined (FS_IMXRT_TSTAT) || defined (FS_IPVAV) || defined (FS_SNAP_ON_IO) || defined(FS_BEATS_IO)
STATIC const struct _ARM_DRIVER_USART s_usartDrivers[] = {
   _USART_DRIVER_VALUE_(bapi_E_Uart1)
#if (BAPI_HAS_USART > 1)
  ,_USART_DRIVER_VALUE_(bapi_E_Uart2)
#endif
#if (BAPI_HAS_USART > 2)
  ,_USART_DRIVER_VALUE_(bapi_E_Uart3)
#endif
#if (BAPI_HAS_USART > 3)
  ,_USART_DRIVER_VALUE_(bapi_E_Uart4)
#endif
#if (BAPI_HAS_USART > 4)
  ,_USART_DRIVER_VALUE_(bapi_E_Uart5)
#endif
#if (BAPI_HAS_USART > 5)
  ,_USART_DRIVER_VALUE_(bapi_E_Uart6)
#endif
#if (BAPI_HAS_USART > 6)
  ,_USART_DRIVER_VALUE_(bapi_E_Uart7)
#endif
#if (BAPI_HAS_USART > 7)
  ,_USART_DRIVER_VALUE_(bapi_E_Uart8)
#endif
#if (BAPI_HAS_USART > 8)
  #error "More than 8 USARTs defined. Please enhance according to the scheme above."
#endif
};
#else
STATIC const struct _ARM_DRIVER_USART s_usartDrivers[] = {
   _USART_DRIVER_VALUE_(bapi_E_Uart0)
#if (BAPI_HAS_USART > 1)
  ,_USART_DRIVER_VALUE_(bapi_E_Uart1)
#endif
#if (BAPI_HAS_USART > 2)
  ,_USART_DRIVER_VALUE_(bapi_E_Uart2)
#endif
#if (BAPI_HAS_USART > 3)
  ,_USART_DRIVER_VALUE_(bapi_E_Uart3)
#endif
#if (BAPI_HAS_USART > 4)
  ,_USART_DRIVER_VALUE_(bapi_E_Uart4)
#endif
#if (BAPI_HAS_USART > 5)
  ,_USART_DRIVER_VALUE_(bapi_E_Uart5)
#endif
#if (BAPI_HAS_USART > 6)
  ,_USART_DRIVER_VALUE_(bapi_E_Uart6)
#endif
#if (BAPI_HAS_USART > 7)
  ,_USART_DRIVER_VALUE_(bapi_E_Uart7)
#endif
#if (BAPI_HAS_USART > 8)
  #error "More than 8 USARTs defined. Please enhance according to the scheme above."
#endif
};
#endif



int32_t driver_usart_setHooks(
  enum bapi_E_UartIndex_ uartIndex,
  const driver_usart_Hooks *const hooks,
  driver_usart_Hooks *const replacedHooks
) {
  int32_t retval = ARM_DRIVER_ERROR_BUSY;

  /* Initialized replaced hooks with NULL. */
  if(replacedHooks) {
     driver_usart_initHooks(replacedHooks);
  }

  bapi_irq_enterCritical();
  if(!Driver_USART::isInitialized(uartIndex)) {


    /* cast the driver_usart_Hooks structure to an array of void pointer so
     * that we can loop over all hook functions. */
    typedef void* _func_cptr;
    const _func_cptr* const src = reinterpret_cast<const _func_cptr *>(hooks);
    _func_cptr* const dst = reinterpret_cast<_func_cptr *>(&s_usartDriverHooks[uartIndex]);
    _func_cptr* const rpl = reinterpret_cast<_func_cptr *>(replacedHooks);

    /* loop over all hook functions. */
    unsigned i = 0;
    for(;i < _driver_usart_HOOK_FUNCTION_COUNT; i++) {
      /* set new hook function, if source hook function in nonzero. */
      if(src[i] != 0) {
        if(replacedHooks) {
            rpl[i] = dst[i];
        }
        dst[i] = src[i];
      }
    }
    retval = ARM_DRIVER_OK;
  }
  bapi_irq_exitCritical();
  return retval;
}


int32_t driver_usart_resetHooks(
  enum bapi_E_UartIndex_ uartIndex
) {
  int32_t retval = ARM_DRIVER_ERROR_BUSY;
  bapi_irq_enterCritical();
  if(Driver_USART::isInitialized(uartIndex)) {
    s_usartDriverHooks[uartIndex] = s_usartDriverDefaultHooks;
    retval = ARM_DRIVER_OK;
  }
  bapi_irq_exitCritical();
  return retval;
}


ARM_DRIVER_USART* driver_usart_getDriver(bapi_E_UartIndex uartIndex) {
  if(uartIndex != bapi_E_Uart_Invalid && uartIndex < bapi_E_UartCount) {
    return &s_usartDrivers[uartIndex];
  }
  return 0;
}

bapi_uart_TransmissionState* driver_usart_getTransmissionState(
  enum bapi_E_UartIndex_ uartIndex
  ) {
  return &Driver_USART::driverState[uartIndex].m_transmissionState;
}
