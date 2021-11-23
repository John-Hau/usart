
#include "baseplate.h"

#include "boards/board-api/bapi_uart.h"
#include "cmsis-driver/Driver_USART.h"
#include "buffering_usart_filter.h"
#include "utils/typed_queue.hpp"

/**
 * \file
 * \brief This file implements the Buffering USART Filter.
 *
 * \note The code is not optimized in terms of memory usage, because this filter
 * is for demo purpose only. A real filter should rather dynamically allocate
 * its data.
 */

/**
 * \defgroup _buffering_usart_filter Internals
 * \brief Buffering USART Filter Internals (non-exposed functions, types, variables).
 * \ingroup buffering_usart_filter
 */

/**
 * \addtogroup _buffering_usart_filter
 */
/*@{*/

/**
 * \brief Name space for buffer filter internals (non-exposed functions, types, variables).
 */
namespace _bufferingUsartFilter {

/**
 * \ingroup _buffering_usart_filter
 * \brief The queue type that is used for buffering.
 */
typedef utils::TypedQueue<uint8_t, buffer_driver_index_t> queue_t;

/**
 * @ingroup _buffering_usart_filter
 * @brief
 * The data type to store the original hook functions. Here we can use
 * the struct driver_usart_Hooks from the CMSIS USART Hook Interface,
 * because we are saving almost all hooks. If we would only hook a
 * subset, we would create structure with a subset of hook functions
 * in order to save RAM.
 */
typedef driver_usart_Hooks replacedHooks_t;

/**
 * \ingroup _buffering_usart_filter
 * \brief This structure declares all data that the filter needs to operate.
 */
struct UsartFilterData {

  /**
   * \brief Here we save the original dataReceived_ISRCallback.
   */
  bapi_uart_dataReceived_ISRCallback_t m_dataReceived_ISRCallback;

  /**
   * \brief Here we save the original Signal Event callback.
   */
  _driver_usartDriverHookSignalEvent_t m_signalEventCallback;

  /**
   * \brief Here we queue the characters, that we received.
   */
  queue_t m_rxQueue;

  /**
   * \brief Here we save the original hook functions which are replaced by
   * this USART filter in the CMSIS USART Driver.
   */
  replacedHooks_t m_replacedHookFunctions;
};

/**
 * \ingroup _buffering_usart_filter
 * \brief Variable storing the FilterData for all USARTs.
 */
STATIC struct UsartFilterData _usartFilterData[bapi_E_UartCount];

/*------------------------------------------------------------------------*//**
 * \ingroup _buffering_usart_filter
 * \brief Our Signal Event callback
 */
STATIC void driver_usart_onEvent(const enum bapi_E_UartIndex_ uartIndex, uint32_t event) {
  if(_usartFilterData[uartIndex].m_signalEventCallback) {

    /* We just call the original Signal Event callback. */
    (*_usartFilterData[uartIndex].m_signalEventCallback)(uartIndex, event);
  }
}

/*------------------------------------------------------------------------*//**
 * \ingroup _buffering_usart_filter
 * \brief Our GetStatus hook function
 */
STATIC ARM_USART_STATUS GetStatus(const enum bapi_E_UartIndex_ uartIndex) {
  if(_usartFilterData[uartIndex].m_replacedHookFunctions.GetStatus) {
    /* We just forward to the original GetStatus function. */
    return (*_usartFilterData[uartIndex].m_replacedHookFunctions.GetStatus)(uartIndex);
  }
  return ARM_USART_STATUS();
}

/*------------------------------------------------------------------------*//**
 * \ingroup _buffering_usart_filter
 * \brief Our Rx Interrupt Service routine.
 * This is our local callback that will be called by the bapi_uart module
 * to dispose one or more received uint8_t data words.
 */
STATIC bapi_uart_MaxFrameSize_t usart_dataReceived_ISRCallback(
  const enum bapi_E_UartIndex_ uartIndex,/**< The source UART of the received character(s). */
  const uint32_t errorEvents,
  const uint8_t rx_chars[],              /**< The received character(s) */
  bapi_uart_MaxFrameSize_t count         /**< The number of received characters */
  ){

  typedef queue_t::index_type index_type;
  index_type retval = 0;

  if(rx_chars) {

    /* We received data. */

    queue_t& rxQueue = _usartFilterData[uartIndex].m_rxQueue;


  //#ifdef _DEBUG
  //  const uint8_t text[] = "abcd";
  //  bapi_uart_MaxFrameSize_t retval = rxQueue.pushMultiple(text, ARRAY_SIZE(text)-1);
  //#else

    /* Push as many received characters as possible into our buffer. */
    retval = rxQueue.pushMultiple(rx_chars, count);

    /* Calculate the remaining number of characters. */
    bapi_uart_MaxFrameSize_t remaining = count - retval;

  //#endif


    /* Get the number of bytes consecutively stored in the queue beginning from
     * the front. */
    index_type consecutive = rxQueue.consecutive();
    while(consecutive) {

      /* Note: We need first to test if the Driver can receive. If we would
       * just call the _oldDataReceived_ISRCallback and evaluate if there
       * was no data processed, the _oldDataReceived_ISRCallback will already
       * have signaled a Rx Overflow, if it couldn't receive. But this would
       * be wrong, because we do the buffering here. Only if we exceed our
       * buffer capabilities, we must issue an Rx Overflow signal (See the
       *  evaluation of the 'remaining' variable at the end of this function).
       */

      /* Can the Driver receive? */
      if(GetStatus(uartIndex).rx_busy) {

        /* Yes it can, so call the old callback passing our buffer. */
        const uint8_t* buffer = rxQueue.pfront();

        index_type processed =
          (*_usartFilterData[uartIndex].m_dataReceived_ISRCallback)(uartIndex, 0, buffer, consecutive);

        if(processed > 0) {
          rxQueue.popMultiple(processed);

          /* There is now additional space in the rx queue. We should use it! */
          if(remaining) {
            retval += rxQueue.pushMultiple(&rx_chars[retval], remaining);
            remaining = count - retval;
          }

          /* See if we could process even more characters. */
          consecutive = rxQueue.consecutive();
          continue;
        }
      }

      /* The old callback could not process any data words. */
      break;
    }

    /* Signal Rx overflow, if we couldn't dispose all our received characters
     * into our buffer. */
    if(remaining) {
      driver_usart_onEvent(uartIndex, ARM_USART_EVENT_RX_OVERFLOW);
    }

  }

  if(errorEvents) {
    /* An Rx Error occurred. We just call the original Signal Event callback. */
    (*_usartFilterData[uartIndex].m_signalEventCallback)(uartIndex, errorEvents);
  }

  return retval;
}

/*------------------------------------------------------------------------*//**
 * \ingroup _buffering_usart_filter
 * \brief Test if driver is already initialized for a USART.
 * @return true, if the driver is already initialized, otherwise false.
 */
C_INLINE bool isInitialized(
  const enum bapi_E_UartIndex_ uartIndex /**< [in] The USART to obtain the initialization state from. */
  ) {
  return (_usartFilterData[uartIndex].m_dataReceived_ISRCallback != 0);
}

/*------------------------------------------------------------------------*//**
 * \ingroup _buffering_usart_filter
 * \brief Our Initialize hook function
 */
STATIC int32_t Initialize(const enum bapi_E_UartIndex_ uartIndex, _driver_usartDriverHookSignalEvent_t driverHook_cb_event) {


  /* Here we could do some own additional pre - initialization. */

  /* ...and passes our own Signal Event Callback to the original Initialize function. */
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

      _usartFilterData[uartIndex].m_dataReceived_ISRCallback = bapi_uart_setDataReceived_ISRCallback(
        uartIndex, usart_dataReceived_ISRCallback);

      /* Assert that the old hook is not the new one. Otherwise well get recursive calls with stack overflow. */
      ASSERT(_usartFilterData[uartIndex].m_dataReceived_ISRCallback != usart_dataReceived_ISRCallback);
    }
  }
  return retval;
}

/*-------------------------------------------------------------------------*//*
 * \ingroup _buffering_usart_filter
 * \brief Our Unitialize hook function
 */
STATIC int32_t Uninitialize(const enum bapi_E_UartIndex_ uartIndex) {

  /* Here we could do some own additional pre - uninitialization. */

  /* ...and passes our own Signal Event Callback to the original Initialize function. */
  int32_t retval = ARM_DRIVER_ERROR;

  if(_usartFilterData[uartIndex].m_dataReceived_ISRCallback) {
    /* Uninstall our ISR callback */
    bapi_uart_dataReceived_ISRCallback_t currentCallback = bapi_uart_setDataReceived_ISRCallback(uartIndex, _usartFilterData[uartIndex].m_dataReceived_ISRCallback);

    /* Assert that the current callback was ours. */
    ASSERT(currentCallback == usart_dataReceived_ISRCallback);
    _usartFilterData[uartIndex].m_dataReceived_ISRCallback = 0;
  }

  if(_usartFilterData[uartIndex].m_replacedHookFunctions.Uninitialize){

    /* Call the original uninitialize function. The original Uninitialize function
     *  de-installs the  */
    retval = (*_usartFilterData[uartIndex].m_replacedHookFunctions.Uninitialize)(uartIndex);

    if(retval == ARM_DRIVER_OK) {
      /* Our Signal Event call back is de-installed, now we can forget the original
       * Signal Event callback. */
      _usartFilterData[uartIndex].m_signalEventCallback = 0;

      /* Flush the receive queue */
      queue_t& rxQueue = _bufferingUsartFilter::_usartFilterData[uartIndex].m_rxQueue;
      rxQueue.flush();

    } else {
      /* Uninitialization failed, so roll back ! */

      /* Re - hook ourselves into the bapi receive ISR callback event. */
      _usartFilterData[uartIndex].m_dataReceived_ISRCallback = bapi_uart_setDataReceived_ISRCallback(
        uartIndex, usart_dataReceived_ISRCallback);
    }
  }

  return retval;
}


/*------------------------------------------------------------------------*//**
 * \ingroup _buffering_usart_filter
 * \brief Our Receive hook function
 */
STATIC int32_t Receive(const enum bapi_E_UartIndex_ uartIndex, void *data, uint32_t num) {
  int32_t retval = ARM_DRIVER_ERROR;

  if(_usartFilterData[uartIndex].m_replacedHookFunctions.Receive) {

    retval = (*_usartFilterData[uartIndex].m_replacedHookFunctions.Receive)(uartIndex, data, num);

    if(retval == ARM_DRIVER_OK) {
      /* Transfer whatever is in the buffer. */
      queue_t& rxQueue = _bufferingUsartFilter::_usartFilterData[uartIndex].m_rxQueue;
      typedef queue_t::index_type index_type;

      /* Avoid interrupts for the UART from this context. */
      bapi_uart_enterCritical(uartIndex, bapi_uart_IRQT_RX);

      index_type consecutive = rxQueue.consecutive();

      while ( consecutive > 0 ) {

        const uint8_t* buffer = rxQueue.pfront();

        /* Call the old callback with the buffer. */
        index_type processed = (*_usartFilterData[uartIndex].m_dataReceived_ISRCallback)(uartIndex, 0, buffer, consecutive);

        if (processed) {
          rxQueue.popMultiple(processed);

          /* Give the Rx interrupt a chance re-fill the buffer. */
          bapi_uart_exitCritical(uartIndex, bapi_uart_IRQT_RX);
          bapi_uart_enterCritical(uartIndex, bapi_uart_IRQT_RX);

          consecutive = rxQueue.consecutive();
        }
        else {
          /*
           * If the old callback cannot process any data words anymore, we just exit.
           */
          break;
        }
      }

      /* Allow interrupts for the UART from this context. */
      bapi_uart_exitCritical(uartIndex, bapi_uart_IRQT_RX);
    }
  }
  return retval;
}


/*------------------------------------------------------------------------*//**
 * \ingroup _buffering_usart_filter
 * \brief Our Send hook function
 */
STATIC int32_t Send(const enum bapi_E_UartIndex_ uartIndex, const void *data, uint32_t num) {
  if(_usartFilterData[uartIndex].m_replacedHookFunctions.Send) {
    /* We just forward to the original Send function. */
    return (*_usartFilterData[uartIndex].m_replacedHookFunctions.Send)(uartIndex, data, num);
  }
  return ARM_DRIVER_ERROR;
}

/**
 * \ingroup _buffering_usart_filter
 * \brief Get buffer driver hooks.
 *
 * Provide the hook functions of this filter in a \ref driver_usart_Hooks
 * structure.
 * Hook functions that are not part of this filter, will be null in the
 * returned structure.
 */
STATIC const struct driver_usart_Hooks _hookFunctions = {
  Initialize,
  Uninitialize,
  NULL,  /* We don't hook an own Control(..) hook function. */
  Receive,
  Send,
  GetStatus
};


} /* namespace BufferDriver */

/*@} _buffering_usart_filter */



/*
 * The C - API functions that cannot go into a name space follow here.
 */
buffer_driver_index_t buffering_usart_filter_popBufferMultiple(
  enum bapi_E_UartIndex_ uartIndex              /**< The UART for which to pop data words from the buffer. */
  , buffer_driver_index_t count                 /**< Number of data words to pop. Zero flushes the buffer. */
  ) {
  typedef _bufferingUsartFilter::queue_t queue_t;
  typedef queue_t::index_type index_type;

  index_type retval = 0;
  queue_t& rxQueue = _bufferingUsartFilter::_usartFilterData[uartIndex].m_rxQueue;

  /* Avoid interrupts for the UART from this context. */
  bapi_uart_enterCritical(uartIndex, bapi_uart_IRQT_RX);

  /* Zero has the special meaning to remove all. */
  if(count == 0) {
    count = rxQueue.size();
  }

  rxQueue.popMultiple(count);

  /* Allow interrupts for the UART from this context. */
  bapi_uart_exitCritical(uartIndex, bapi_uart_IRQT_RX);

  return retval;
}



buffer_driver_index_t buffering_usart_filter_getBufferContents(
  enum bapi_E_UartIndex_ uartIndex
  , uint8_t* contents
  , buffer_driver_index_t max
  , buffer_driver_index_t* queueIndexFirst
  , buffer_driver_index_t* queueIndexEnd
  ) {
  typedef _bufferingUsartFilter::queue_t queue_t;
  typedef queue_t::index_type index_type;

  index_type retval = 0;
  queue_t& rxQueue = _bufferingUsartFilter::_usartFilterData[uartIndex].m_rxQueue;

  /* Avoid interrupts for the UART from this context. */
  bapi_uart_enterCritical(uartIndex, bapi_uart_IRQT_RX);


  retval = rxQueue.consecutive();
  const uint8_t* buffer = rxQueue.pfront();

  if(max < retval) {
    utils::object<uint8_t, index_type>::copy(contents, buffer, max);
    retval = +max;

  } else {
    utils::object<uint8_t, index_type>::copy(contents, buffer, retval);
    max -= retval; /* Adjust to the remaining data that can be consumed. */

    index_type wrapped = rxQueue.wrappedConsecutive();
    buffer = rxQueue.pdata();

    if(max < wrapped) {
      utils::object<uint8_t, index_type>::copy(&contents[retval], buffer, max);
      retval += max;

    } else {
      utils::object<uint8_t, index_type>::copy(&contents[retval], buffer, wrapped);
      retval += wrapped;
    }
  }

  *queueIndexFirst = rxQueue._first();
  *queueIndexEnd = rxQueue._end();

  /* Allow interrupts for the UART from this context. */
  bapi_uart_exitCritical(uartIndex, bapi_uart_IRQT_RX);

  return retval;
}

/* This is the hook function pointer that we use to indicate that the driver is hooked or under transition from
 * hooked to unhooked or vice versa. */
typedef UninitializeFunction_t HookedIndicatorFunction_t;

C_INLINE bool isHookedOrTransitioning(enum bapi_E_UartIndex_ uartIndex) {
  return _bufferingUsartFilter::_usartFilterData[uartIndex].m_replacedHookFunctions.Uninitialize != 0;
}

C_INLINE bool isTransitioning(enum bapi_E_UartIndex_ uartIndex) {
  return _bufferingUsartFilter::_usartFilterData[uartIndex].m_replacedHookFunctions.Uninitialize == _bufferingUsartFilter::Uninitialize;
}

C_INLINE HookedIndicatorFunction_t setTransitioning(enum bapi_E_UartIndex_ uartIndex) {
  HookedIndicatorFunction_t retval = _bufferingUsartFilter::_usartFilterData[uartIndex].m_replacedHookFunctions.Uninitialize;
  _bufferingUsartFilter::_usartFilterData[uartIndex].m_replacedHookFunctions.Uninitialize = _bufferingUsartFilter::Uninitialize;
  return retval;
}

C_INLINE void setUnhooked(enum bapi_E_UartIndex_ uartIndex) {
  bapi_irq_enterCritical();
  driver_usart_initHooks(&_bufferingUsartFilter::_usartFilterData[uartIndex].m_replacedHookFunctions);
  bapi_irq_exitCritical();
}

C_INLINE void abortUnhook(enum bapi_E_UartIndex_ uartIndex, HookedIndicatorFunction_t restore) {
  _bufferingUsartFilter::_usartFilterData[uartIndex].m_replacedHookFunctions.Uninitialize = restore;
}

int32_t buffering_usart_filter_hook(
  enum bapi_E_UartIndex_ uartIndex, uint16_t bufferSize
) {

  bapi_irq_enterCritical();

  if (!isHookedOrTransitioning(uartIndex)) {
    if(isTransitioning(uartIndex)) {
      bapi_irq_exitCritical();
      return ARM_DRIVER_ERROR_BUSY;
    }

    setTransitioning(uartIndex);

    bapi_irq_exitCritical();

    int32_t retval = driver_usart_setHooks(uartIndex, &_bufferingUsartFilter::_hookFunctions
      , &_bufferingUsartFilter::_usartFilterData[uartIndex].m_replacedHookFunctions);

    if (retval == ARM_DRIVER_OK) {
      /* Success: Create the buffer. */
      _bufferingUsartFilter::_usartFilterData[uartIndex].m_rxQueue.create(bufferSize);
    }
    else {
      /* Cleanup upon error. */
      setUnhooked(uartIndex);
    }

    return retval;
  }

  bapi_irq_exitCritical();

  /* The driver is already hooked. */
  return ARM_DRIVER_ERROR;
}

int32_t buffering_usart_filter_unhook(
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

    if(_bufferingUsartFilter::_usartFilterData[uartIndex].m_signalEventCallback == 0) {
      int32_t retval = driver_usart_setHooks(uartIndex, &_bufferingUsartFilter::_usartFilterData[uartIndex].m_replacedHookFunctions, 0);
      if(retval == ARM_DRIVER_OK) {
        _bufferingUsartFilter::_usartFilterData[uartIndex].m_rxQueue.destroy();
        setUnhooked(uartIndex);
        return retval;
      }
    }

    /* Driver is still initialized, cannot unhook. Client must Uninitialize before unhook. */
    abortUnhook(uartIndex, originalHookFunction);
    return ARM_DRIVER_ERROR;
  }

  bapi_irq_exitCritical();

  /* Is already unhooked. */
  return ARM_DRIVER_ERROR;
}
