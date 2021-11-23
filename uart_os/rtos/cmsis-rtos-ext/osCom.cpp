/*
 * osaCom.cpp
 *
 *  Created on: 10.04.2013
 *      Author: e673505
 */

/**
 * \file
 * This file implements the ARM CMSIS RTOS Com Extension API for read/write access
 * communication interfaces via file descriptors.
 */

#include "baseplate.h"
#include <string.h>

#include "osCom.h"
#include "utils/utils.h"

#include "boards/board-api/bapi_uart.h"
#include "cmsis-driver/Driver_USART.h"
#include "cmsis-driver/usart-filter/console_usart_filter.h"
#include "cmsis-driver/usart-filter/bacnetMSTP_usart_filter.h"

#include "boards/board-api/bapi_irq.h"
#include "utils/isrmem.h"
#include "rtos/c++/osMailQueue.hpp"
//#include <stdio.h>


#ifndef COM_CFG_NAMED_RX_TX_BUFFER
  #define COM_CFG_NAMED_RX_TX_BUFFER 0
#endif

#ifndef COM_CFG_TX_TRACE_BUFFER_SIZE_BITS
  #define COM_CFG_TX_TRACE_BUFFER_SIZE_BITS 0
#endif


#ifndef BAPI_UART_RX_TX_LOCAL_BUFFER_SIZE
  #error "BAPI_UART_RX_TX_LOCAL_BUFFER_SIZE not defined in the product configuration header file."
#endif


C_INLINE void _deallocate_msg(struct com_msg_buffer *msg);


/**
 * \ingroup _cmsis_os_ext_com
 *
 * \brief
 * This is the structure that holds a message to be sent or received.
 *
 * */
packed_struct(com_msg_buffer) {
#if COM_TX_TRACE_BUFFER_SIZE_BITS > 0
  unsigned int trace_index;
#endif

  char*     m_mem;                      /**< Pointer to the message that resides in allocated heap memory or in
                                         *  the local buffer.
                                         */
  uint32_t  m_len                 : 31; /**< The length of the message. */
  uint32_t  m_localBufferEnabled  :  1; /**< Set to 1 if the local buffer holds the message. Set to 0 if m_mem
                                         * holds the message in dynamically allocated memory.
                                         */
#if OS_COM_ENABLE_WRITE_WITH_FEEDBACK
  osComWriteCallback_t m_txCallback;        /**< The callback that will be called, when the transmission has been completed. */
  void* m_txUsrParam;                      /**< The parameter that will be passed to the callback. */
#endif

  char      m_localBuffer[BAPI_UART_RX_TX_LOCAL_BUFFER_SIZE];/**< Holds the contents of the message in case of
                                         * short messages. Avoids buffer memory allocation for a short message.
                                         */

  inline com_msg_buffer()
      : m_mem(0)
      , m_len(0)
      , m_localBufferEnabled(false)
#if OS_COM_ENABLE_WRITE_WITH_FEEDBACK
      , m_txCallback(0)
      , m_txUsrParam(0)
#endif
  {
  }

  /**
   * \brief Moves the contents of a source message buffer to this message buffer.
   * \warning The contents of the source message buffer
   * will be cleared in order to avoid duplicate release of allocated memory.
   * The return value is void, because an assignment chain doesn't make sense.
   * This object becomes the new owner of allocated memory and is responsible
   * deallocate it, when it is not longer needed.
   */
  void operator=(
    com_msg_buffer& src /**< The source message buffer from where to move the contents. */
  ) {
    m_len = src.m_len;
    m_localBufferEnabled = src.m_localBufferEnabled;
    if(src.m_localBufferEnabled) {
      MEMCPY(m_localBuffer, src.m_localBuffer, src.m_len);
      m_mem = m_localBuffer;
    } else {
      m_mem = src.m_mem;
      src.m_mem = 0;
      src.m_len = 0;
      src.m_localBufferEnabled = false;
    }
#if OS_COM_ENABLE_WRITE_WITH_FEEDBACK
    m_txCallback = src.m_txCallback;
    m_txUsrParam = src.m_txUsrParam;
#endif
  }

  inline ~com_msg_buffer() {
    _deallocate_msg(this);
  }

private:
  inline com_msg_buffer(const com_msg_buffer& rhs)
    : m_mem(rhs.m_localBufferEnabled ? m_localBuffer : rhs.m_mem)
    , m_len(rhs.m_len)
    , m_localBufferEnabled(rhs.m_localBufferEnabled)
#if OS_COM_ENABLE_WRITE_WITH_FEEDBACK
    , m_txCallback(rhs.m_txCallback)
    , m_txUsrParam(rhs.m_txUsrParam)
#endif
  {
    /* Nobody must ever do an assignment with a const sourcein order to avoid to buffers pointing to the same allocated memory. */
    ASSERT(false);
  }

  /* Nobody must ever do an assignment with a const source in order to avoid to buffers pointing to the same allocated memory. */
  const com_msg_buffer& operator=(const com_msg_buffer& src);
};



#if COM_TX_TRACE_BUFFER_SIZE_BITS > 0

#define COM_TX_TRACE_BUFFER_SIZE (1 << (COM_CFG_TX_TRACE_BUFFER_SIZE_BITS))
#define COM_TX_TRACE_BUFFER_SIZE_INDEX(i) \
  (i & (COM_TX_TRACE_BUFFER_SIZE - 1))


struct bapi_tx_msg_trace {
  enum E_TRACE_STATUS {
    UNUSED = 0,
    USED,
    RELEASED,
    ISR_RELEASED
  };

  enum E_TRACE_STATUS status;
  com_msg_buffer msg;
};

struct tx_trace_t {
  unsigned int add_counter;
  unsigned int rmv_counter;
  bapi_tx_msg_trace trace_buffer[COM_TX_TRACE_BUFFER_SIZE];
};

STATIC tx_trace_t tx_trace;


C_INLINE void trace_tx_msg_add(com_msg_buffer* const msg) {
  unsigned int trace_index = COM_TX_TRACE_BUFFER_SIZE_INDEX(tx_trace.add_counter);
  msg->trace_index = trace_index;
  tx_trace.trace_buffer[COM_TX_TRACE_BUFFER_SIZE_INDEX(tx_trace.add_counter)].status = bapi_tx_msg_trace::USED;
  tx_trace.trace_buffer[COM_TX_TRACE_BUFFER_SIZE_INDEX(tx_trace.add_counter)].msg = *msg;
  tx_trace.add_counter++;
}

C_INLINE void trace_tx_msg_rmv(const com_msg_buffer* const msg, bool fromISR) {
  bapi_tx_msg_trace* trace_buffer_entry = &tx_trace.trace_buffer[msg->trace_index];
  int error = MEMCMP(msg, &trace_buffer_entry->msg, sizeof(com_msg_buffer));
  ASSERT(error == 0);
  trace_buffer_entry->status = fromISR ?
      bapi_tx_msg_trace::ISR_RELEASED : bapi_tx_msg_trace::RELEASED;
  tx_trace.rmv_counter++;
}

#else
  #define trace_tx_msg_add(msg)
  #define trace_tx_msg_rmv(msg, fromISR)
#endif

/** To be implemented in C library malloc adapter (e.g.newlib_malloc.c) */
extern struct ualloc_mem_* volatile deferred_deallocation_chain;

C_INLINE void _alloc_local_buffer(int len, struct com_msg_buffer* const msg)
  {
  /* local buffer to avoid time consuming malloc. */
  msg->m_localBufferEnabled = true;
  msg->m_mem = msg->m_localBuffer;
  msg->m_len = len;
}

/**
 * \ingroup _cmsis_os_ext_com
 * \brief
 * allocate buffer for the message to send.
 */
STATIC void _alloc_buffer(struct com_msg_buffer *const  msg, int len) {
  if(len > ARRAY_SIZE(msg->m_localBuffer) ) {

    /* local buffer is insufficient, allocate memory */
    msg->m_localBufferEnabled = false;

    msg->m_mem = static_cast<char*>(ISRMEM_malloc(len));
    SYSLOG(msg->m_mem != NULL);

    if(msg->m_mem)
    {
      msg->m_len = len;
    } else {
      msg->m_len = 0;
    }
  } else {

    /* local buffer to avoid time consuming malloc. */
    _alloc_local_buffer(len, msg);
  }

  trace_tx_msg_add(msg);
}

C_INLINE int _alloc_and_copy_write_buffer(struct com_msg_buffer *const  msg, int len, const char* data) {
  int spare = 0;

  if(bapi_irq_isInterruptContext()) {
    /* In ISR we cannot allocate memory, so we cut off the data */
    if(len >= ARRAY_SIZE(msg->m_localBuffer)) {
      if(len > 0 && data[len-1] == '\n' ) {
        msg->m_localBuffer[ARRAY_SIZE(msg->m_localBuffer)-1] = '\n';
        spare = 1;
      }
      len = ARRAY_SIZE(msg->m_localBuffer);
    }
    _alloc_local_buffer(len, msg);
  } else {
    _alloc_buffer(msg, len);
  }

  MEMCPY(msg->m_mem, data, len - spare);
  return len;
}

C_INLINE void _deallocate_msg_From_ISR(struct com_msg_buffer *msg) {
  trace_tx_msg_rmv(msg, true);
  bapi_irq_enterCritical();

  if (!msg->m_localBufferEnabled && (msg->m_mem != 0)) {
    deferred_deallocation_chain = ISRMEM_add_to_dealloc_chain(deferred_deallocation_chain, msg->m_mem);
  }

  msg->m_len = 0;
  msg->m_mem = 0;

  bapi_irq_exitCritical();
}


C_INLINE void _deallocate_msg(struct com_msg_buffer *msg) {
  ASSERT(msg);
  if(bapi_irq_isInterruptContext()) {
    _deallocate_msg_From_ISR(msg);
    return;
  }

  trace_tx_msg_rmv(msg, false);
  if(!msg->m_localBufferEnabled && (msg->m_mem != 0)) {
    ISRMEM_immediate_dealloc(msg->m_mem);
  }
  msg->m_mem = 0;
  msg->m_len = 0;

  return;
}


typedef os::MailQueue<com_msg_buffer> com_tx_queue;
typedef os::MailQueue<com_msg_buffer> com_rx_queue;

struct com_buffer {
  com_tx_queue m_txQueue;
  //com_msg_buffer * m_ptxCurrent;
  com_msg_buffer m_txCurrent;
  int8_t         m_isTransmitting;

#if OS_COM_ENABLE_WRITE_WITH_FEEDBACK
  osComWriteCallback_t m_txCallback;
  void* m_txUsrParam;
#endif

  com_rx_queue m_rxQueue; /** TODO: get rid of this queue by using an event and passing m_rxCurrent to the recipient. */
  com_msg_buffer m_rxCurrent;

  uint16_t m_requestedRxCount;
  uint16_t m_interimRxCount;
  int8_t   m_isReading;
  int8_t   m_isWriting;
  int8_t   m_fd[3];       /** Up to 3 file descriptors can be assigned to a UART */
};

STATIC struct com_buffer _comQueues[bapi_E_UartCount];

/**
 *\brief
 *Retrieve the uart index for a file descriptor
 *
 * \return The uart index if there is one for this file descriptor. Otherwise bapi_E_UartCount.
 */
enum bapi_E_UartIndex_ _osComFd2Usart(int fd) {
  size_t i = 0;
  for(;i < bapi_E_UartCount; i++) {

    const struct com_buffer* comQueue = &_comQueues[i];

    if( comQueue->m_fd[0] == fd
      || comQueue->m_fd[1] == fd
      || comQueue->m_fd[2] == fd
    ) {
      return static_cast<enum bapi_E_UartIndex_>(i);
    }
  }

  return bapi_E_UartCount;
}

/**
 * \ingroup _cmsis_os_ext_com
 * Initialize all file descriptors with -1 even before main(..) is running.
 *
 */
class ComFdInitializer {
  static ComFdInitializer instance;
  ComFdInitializer() {
    size_t i = 0;
    for(;i < bapi_E_UartCount; i++) {

      struct com_buffer* comQueue = &_comQueues[i];

      comQueue->m_fd[0] = DEV_FD_INVALID;
      comQueue->m_fd[1] = DEV_FD_INVALID;
      comQueue->m_fd[2] = DEV_FD_INVALID;
    }
  }
};

ComFdInitializer ComFdInitializer::instance;

///**
// * \ingroup _cmsis_os_ext_com
// * \Test if a file descriptor is in use.
// */
//C_INLINE bool _isFdInUse(int fd) {
//  size_t i = 0;
//  for(;i < bapi_E_UartCount; i++) {
//    if(_comQueues[i].m_fd[0] == fd) {
//      return true;
//    }
//    if(_comQueues[i].m_fd[1] == fd) {
//      return true;
//    }
//    if(_comQueues[i].m_fd[2] == fd) {
//      return true;
//    }
//  }
//  return false;
//}

/**
 * \ingroup _cmsis_os_ext_com
 * \This is the ARM Driver Signal event call back that will handle all ARM Driver USART signals.
 */
STATIC void driver_usart_onEvent(
    enum bapi_E_UartIndex_ uartIndex,   /**< The USART that generated this event. */
    uint32_t event                      /**< Any ARM_USART_EVENT_x code of the ARM CMSIS driver definitions. */
  );

int32_t osComUsartUninitialize(
  bapi_E_UartIndex uartIndex /**< The UART to be uninitialized */
  ) {

  /* Preset the return value for the case that we are already uninitialized or uninitializing. */
  int32_t retval = ARM_DRIVER_OK;

  com_buffer* queues = &_comQueues[uartIndex];
  bapi_irq_enterCritical();

  if (queues->m_fd[0] !=  DEV_FD_INVALID) {

    /* We are either initialized, or in an initializing
     * or uninitializing state.*/

    if (queues->m_fd[0] !=  DEV_FD_UNINITIALIZING) {
      /* We are either initialized, or in an initializing state.*/

      /* Preset the return value for the case that we are initializing.
       * In case we are initializing, this uninitialize call will fail.
       * */
      retval = ARM_DRIVER_ERROR;

      if(queues->m_fd[0] !=  DEV_FD_INITIALIZING) {
        /* We are initialized, so go ahead and uninitialize */

        /* Signal that we are uninitializing. This will force
         * an asynchronous call of this function or any
         * osComRead, osComWrite, osComUsartInitialize
         * to exit with error. */
        int8_t oldFd = atomic_Get(&queues->m_fd[0]);
        atomic_Set(&queues->m_fd[0], DEV_FD_UNINITIALIZING);

        /* Enable interrupts as soon as possible. We are save here,
         * because our status is set to DEV_FD_UNINITIALIZING. */
        bapi_irq_exitCritical();

        ARM_DRIVER_USART* driver = driver_usart_getDriver(uartIndex);

        retval = driver->Uninitialize();
        if(retval == ARM_DRIVER_OK) {

          /* Clear send and receive buffers. */
          queues->m_rxQueue.destroy(osWaitForever); // TODO: pass timeout from additional parameter of this function.
          queues->m_txQueue.destroy(osWaitForever); // TODO: pass timeout from additional parameter of this function.

          atomic_Set(&queues->m_fd[0], DEV_FD_INVALID);
          atomic_Set(&queues->m_fd[1], DEV_FD_INVALID);
          atomic_Set(&queues->m_fd[2], DEV_FD_INVALID);


        } else {
          /* driver uninitialization failed, so recover the old file
           * descriptor. */
          atomic_Set(&queues->m_fd[0], oldFd);
        }

        /* return already here, because we called bapi_irq_exitCritical()
         * before, to enable interrupts as soon as possible. */
        return retval;
      }
    }
  }

  bapi_irq_exitCritical();
  return retval;
}

int32_t osComUsartInitialize(
  bapi_E_UartIndex uartIndex
  ,int fd, uint16_t txQueueSize
  ) {

  int32_t retval = ARM_DRIVER_ERROR_PARAMETER;
  com_buffer* queues = &_comQueues[uartIndex];

  bapi_irq_enterCritical();


  if ((fd >= 0) && (fd < DEV_FD_COUNT)) {

    if (queues->m_fd[0] == DEV_FD_INVALID) {
      /* We are in an uninitialized state. */

      /* Signal that we are initializing. This will force
       * an asynchronous call of this function or any
       * osComRead, osComWrite, osComUsartUninitialize
       * to exit with error.
       */
      atomic_Set(&queues->m_fd[0], DEV_FD_INITIALIZING);

      /* Enable interrupts as soon as possible. We are save here,
       * because our status is set to DEV_FD_INITIALIZING. */
      bapi_irq_exitCritical();

      if (!queues->m_txQueue.isCreated()) {
#if COM_CFG_NAMED_RX_TX_BUFFER
        char name[16];
        memset(name, 0, ARRAY_SIZE(name));
        sniprintf(name, ARRAY_SIZE(name), "txQueue[%d]", uartIndex);
        queues->m_txQueue.create(txQueueSize, name);
#else
        queues->m_txQueue.create(txQueueSize);
#endif
      }

      if (!queues->m_rxQueue.isCreated()) {
#if COM_CFG_NAMED_RX_TX_BUFFER
        char name[16];
        memset(name, 0, ARRAY_SIZE(name));
        sniprintf(name, ARRAY_SIZE(name), "rxQueue[%d]", uartIndex);
        queues->m_rxQueue.create(1, name);
#else
        queues->m_rxQueue.create(1);
#endif
      }

      {
        ARM_DRIVER_USART* driver = driver_usart_getDriver(uartIndex);
        driver->Initialize(driver_usart_onEvent);
      }

      /* Finally leave the initializing state and set the
       * right file descriptor in one shot. */
      atomic_Set(&queues->m_fd[0], fd);

      /* return already here, because we called bapi_irq_exitCritical()
       * before, to enable interrupts as soon as possible. */
      return ARM_DRIVER_OK;
    }


    /* We are already initialized, initializing or uninitializing. */
    retval = ARM_DRIVER_ERROR;
  }

  /* Re-Enable interrupts in error case. */
  bapi_irq_exitCritical();
  return retval;
}

int32_t osComConsoleInitialize(
  bapi_E_UartIndex uartIndex
  ,uint16_t txQueueSize
  ) {

	console_driver_hookDriver(uartIndex);//console_driver_hookDriver(CONSOLE_UART);

  int32_t retval = osComUsartInitialize(uartIndex, STDIN_FILENO /* stdin */, txQueueSize);

  if(retval == ARM_DRIVER_OK) {
    com_buffer* queues = &_comQueues[uartIndex];
    queues->m_fd[1] = STDOUT_FILENO; /* stdout */
    queues->m_fd[2] = STDERR_FILENO; /* stderr */
  }
  return retval;
}

C_INLINE int pushIntoTxQueue(int retval, int len, const char* ptr, bapi_E_UartIndex uartIndex
#if OS_COM_ENABLE_WRITE_WITH_FEEDBACK
  , osComWriteCallback_t txCompleteCallback
  , void* txUserParam
#endif /* #if OS_COM_ENABLE_WRITE_WITH_FEEDBACK */
) {
  static struct com_msg_buffer msg;
  retval = _alloc_and_copy_write_buffer(&msg, len, ptr);
  if (msg.m_mem) {

#if OS_COM_ENABLE_WRITE_WITH_FEEDBACK
    msg.m_txCallback  = txCompleteCallback;
    msg.m_txUsrParam = txUserParam;
#endif /* #if OS_COM_ENABLE_WRITE_WITH_FEEDBACK */

    osStatus_t result = _comQueues[uartIndex].m_txQueue.allocateAndPut(&msg);
    if (result != osOK) {
      _deallocate_msg(&msg);
      retval = ARM_DRIVER_ERROR_BUSY;
    }
  }
  return retval;
}

C_INLINE void freeTxCurrent(bapi_E_UartIndex uartIndex) {
  //com_msg_buffer* ptxCurrent = atomic_Replace(&_comQueues[uartIndex].m_ptxCurrent, S_CAST(com_msg_buffer*, 0));
  //_comQueues[uartIndex].m_txQueue.freeItem(ptxCurrent);
  //_deallocate_msg(ptxCurrent);
  _deallocate_msg(&_comQueues[uartIndex].m_txCurrent);
}

C_INLINE void popFromTxQueueAndSend(bapi_E_UartIndex uartIndex) {
  /* Nothing in Current Tx */
  ARM_DRIVER_USART* driver = driver_usart_getDriver(uartIndex);

  bapi_irq_enterCritical();

  //if (!_comQueues[uartIndex].m_ptxCurrent) {
  if (!_comQueues[uartIndex].m_isTransmitting) {
    //_comQueues[uartIndex].m_ptxCurrent = _comQueues[uartIndex].m_txQueue.get(0);
    //if (_comQueues[uartIndex].m_ptxCurrent) {
    if(osOK == _comQueues[uartIndex].m_txQueue.get(&_comQueues[uartIndex].m_txCurrent, 0)) {
      _comQueues[uartIndex].m_isTransmitting = 1;

      bapi_irq_exitCritical();

      char * txdata = 0;
      if( _comQueues[uartIndex].m_txCurrent.m_localBufferEnabled ){
        txdata = _comQueues[uartIndex].m_txCurrent.m_localBuffer;
      }else{
        txdata = _comQueues[uartIndex].m_txCurrent.m_mem;
      }
      //int32_t err = (*driver->Send)(_comQueues[uartIndex].m_ptxCurrent->m_mem, _comQueues[uartIndex].m_ptxCurrent->m_len);
      int32_t err = (*driver->Send)(txdata, _comQueues[uartIndex].m_txCurrent.m_len);
      if (err != ARM_DRIVER_OK) {
        freeTxCurrent(uartIndex);
      }
      return;
    }
  }

  bapi_irq_exitCritical();
}

#if OS_COM_ENABLE_WRITE_WITH_FEEDBACK

int osComWriteWithFeedback(int fd, const char *ptr, int len,
  osComWriteCallback_t callback, void* userParam
  )
#else

int osComWrite(int fd, const char *ptr, int len)

#endif
  {
  int retval = ARM_DRIVER_ERROR_PARAMETER;

  if (fd < DEV_FD_COUNT && fd >= 0) {

    bapi_E_UartIndex uartIndex = _osComFd2Usart(fd);

    if (uartIndex < bapi_E_UartCount) {

#if OS_COM_ENABLE_WRITE_WITH_FEEDBACK
      retval = pushIntoTxQueue(retval, len, ptr, uartIndex, callback, userParam);
#else
      retval = pushIntoTxQueue(retval, len, ptr, uartIndex);
#endif /* #ifdef OS_COM_ENABLE_WRITE_WITH_FEEDBACK */

      /* Check if someone is already writing */
      bapi_irq_enterCritical();

      if (_comQueues[uartIndex].m_isWriting) {
        /* The Send request has been queued so return with ARM_DRIVER_OK  */
        bapi_irq_exitCritical();
      } else {
        /* Nobody else is currently writing, so go ahead. */
        ++_comQueues[uartIndex].m_isWriting;
        bapi_irq_exitCritical();

        /* if we can send something, do it.  */
        //if (!_comQueues[uartIndex].m_ptxCurrent)  { /* Nothing in Current Tx */
        if (0 == _comQueues[uartIndex].m_isTransmitting)  { /* Nothing in Current Tx */
          popFromTxQueueAndSend(uartIndex);
        }
        atomic_Add(&_comQueues[uartIndex].m_isWriting, -1);
      }
    }
  }
  return retval;
}


/* Template function for the case that waitForeverValue is not equal to osWaitForever */
template<MsecType waitForeverValue> MsecType getOsBlockTime(MsecType msecBlockTime) {
  return msecBlockTime == waitForeverValue ? osWaitForever : msecBlockTime;
}

/* Template function specialization for the case that waitForeverValue is equal to osWaitForever */
template<> MsecType getOsBlockTime<osWaitForever>(MsecType msecBlockTime) {
  return msecBlockTime;
}

int osComRead(int fd, char *ptr, int len, MsecType msecBlockTime, bool flushFirst) {
  bapi_E_UartIndex uartIndex = _osComFd2Usart(fd);

  if (uartIndex < bapi_E_UartCount) {
    com_buffer& queues = _comQueues[uartIndex];

    bapi_irq_enterCritical();

    if(queues.m_isReading) {
      bapi_irq_exitCritical();
      return ARM_DRIVER_ERROR_BUSY;
    }

    /* Nobody else is currently reading, so go ahead. */
    ++queues.m_isReading;

    /* Let an interrupt step in */
    bapi_irq_exitCritical();

    ARM_DRIVER_USART* driver = driver_usart_getDriver(uartIndex);

    int retval = ARM_DRIVER_ERROR;

    bapi_irq_enterCritical();
    if (queues.m_requestedRxCount > 0) {

#ifdef _DEBUG
      ARM_USART_STATUS status = driver->GetStatus();
      ASSERT(status.rx_busy);
#endif

      if (queues.m_requestedRxCount != len) {

        driver->Control(ARM_USART_ABORT_RECEIVE, 0);
        bapi_irq_exitCritical();

        /* TODO: When a character will be received here, we will miss it ! Is that ok ?*/

        queues.m_interimRxCount += (*driver->GetRxCount)();

        if (queues.m_requestedRxCount < len) {
          com_msg_buffer msg;

          /* Allocate larger buffer */
          _alloc_buffer(&msg, len);

          if (msg.m_mem && ! flushFirst) {
            /* Copy what we have received so far to the new buffer */
            MEMCPY(msg.m_mem, queues.m_rxCurrent.m_mem, queues.m_interimRxCount);
          } else {
            /* Cannot keep what was received so far, because buffer allocation
             * failed or flushFirst forces dropping what has been received so
             * far. */
            queues.m_interimRxCount = 0;
          }

          /* Deallocate the old buffer that was too small */
          _deallocate_msg(&queues.m_rxCurrent);

          /* Use the new buffer, in case that the allocation of it was successful */
          if (msg.m_mem) {
            queues.m_rxCurrent = msg;
          }
        }

        /* Continue receive, if there is buffer available. */
        if (queues.m_rxCurrent.m_mem) {
          queues.m_requestedRxCount = len;
          retval = (*driver->Receive)(&queues.m_rxCurrent.m_mem[queues.m_interimRxCount], len - queues.m_interimRxCount);
        }

      } else {

        if (flushFirst) {

          /* Stop receive */
          driver->Control(ARM_USART_ABORT_RECEIVE, 0);
          bapi_irq_exitCritical();

          /* Flush */
          queues.m_interimRxCount = 0;

          /* Restart receive with the same buffer. */
          retval = (*driver->Receive)(queues.m_rxCurrent.m_mem, len);
        } else {
          /* Continue receive */
          retval = ARM_DRIVER_OK;
          bapi_irq_exitCritical();
        }
      }

    } else {
      /* Receiver not busy, so last receive sequence should have left everything cleaned up. */
      bapi_irq_exitCritical();
      ASSERT(queues.m_rxCurrent.m_mem == 0);
      ASSERT(queues.m_rxCurrent.m_len == 0);
      ASSERT(queues.m_interimRxCount == 0);
      ASSERT(queues.m_requestedRxCount == 0);

      /* Allocate buffer */
      _alloc_buffer(&queues.m_rxCurrent, len);

      /* Receive in case buffer allocation was successful. */
      if (queues.m_rxCurrent.m_mem) {
        queues.m_requestedRxCount = len;
#ifdef _DEBUG
        if(uartIndex != CONSOLE_UART) {
          int x = 0; /* Allows to set a breakpoint for non Console UART events. */
          x++; /* Avoid warning about unused variable. */
        }
#endif
        retval = (*driver->Receive)(queues.m_rxCurrent.m_mem, len);
#ifdef _DEBUG
        if(uartIndex != CONSOLE_UART) {
          int x = 0; /* Allows to set a breakpoint for non Console UART events. */
          x++; /* Avoid warning about unused variable. */
        }
#endif
      }
    }

    if (retval == ARM_DRIVER_OK) {

      /* The ARM_USART_EVENT_RECEIVE_COMPLETE signal will push the received message into the
       * queue. In case we run an RTOS, the popFront() call will until something a message is
       * in the queue or until a timeout appears. Without an RTOS, the popFront() call will
       * return immediately with false, because nothing has been received so far. Hence
       * the osComRead() function must be called again after some time, hoping that
       * a message is in the queue then.  */
      com_msg_buffer msg;

      //osEvent event = queues.m_rxQueue.getAndFree(&msg, getOsBlockTime<osComWaitForever>(msecBlockTime));
      //if(event.status == osEventMail || event.status == osEventMessage) {
      osStatus_t retStatus;
      if(osOK == (retStatus = queues.m_rxQueue.getAndFree(&msg, getOsBlockTime<osComWaitForever>(msecBlockTime)))) {
        MEMCPY(ptr, msg.m_mem, msg.m_len);
        retval = msg.m_len;

        _deallocate_msg(&msg);
      } else {
        //if( event.status == osErrorResource ) {
        if( retStatus == osErrorResource ) {
          /* The mail queue was deleted, upon an Arm Driver USART Uninitialize call.
           * Since there is no specific error code, we return the unspecific one. */
          retval = ARM_DRIVER_ERROR;
        }
      }

    }

    /* Signal that we are not reading anymore. */
    atomic_Add(&queues.m_isReading, -1);

    return retval;

  }

  /* The file descriptor argument is not a valid one. This may be, because the USART is still
   * or was uninitialized by an Arm Driver USART Uninitialize call. */
  return ARM_DRIVER_ERROR_PARAMETER;
}


/**
 * \ingroup _cmsis_os_ext_com
 */
STATIC void driver_usart_onEvent(enum bapi_E_UartIndex_ uartIndex, uint32_t event) {

#ifdef _DEBUG
  osThreadId_t id = osThreadGetId(); /* See what osThreadId() returns in an ISR context. */
#endif

  switch ( event ) {
    case ARM_USART_EVENT_SEND_COMPLETE:
      {
      /* No more data required from the transmit buffer, so deallocate it. */
#ifdef _DEBUG
        if(uartIndex != CONSOLE_UART) {
          int x = 0; /* Allows to set a breakpoint for non Console UART events. */
          x++; /* Avoid warning about unused variable. */
        }
#endif

      /* Assert there is a message under transmission when we get this event. */
      //SYSLOG(_comQueues[uartIndex].m_ptxCurrent);

#if OS_COM_ENABLE_WRITE_WITH_FEEDBACK
      /* Save the callback and userParmeter in _comQueues[] variable so
       * that they are still available for the ARM_USART_EVENT_TX_COMPLETE
       * event.
       */
      //_comQueues[uartIndex].m_txCallback  = _comQueues[uartIndex].m_ptxCurrent->m_txCallback;
      //_comQueues[uartIndex].m_txUsrParam  = _comQueues[uartIndex].m_ptxCurrent->m_txUsrParam;
      _comQueues[uartIndex].m_txCallback  = _comQueues[uartIndex].m_txCurrent.m_txCallback;
      _comQueues[uartIndex].m_txUsrParam  = _comQueues[uartIndex].m_txCurrent.m_txUsrParam;
#endif

      /* Free up the buffers for the message that was sent. */
      //if(_comQueues[uartIndex].m_ptxCurrent) {
      //  freeTxCurrent(uartIndex);
      //}
      if( _comQueues[uartIndex].m_isTransmitting > 0 ){
        freeTxCurrent(uartIndex);
        _comQueues[uartIndex].m_isTransmitting = 0;
      }

#if OS_COM_ENABLE_WRITE_WITH_FEEDBACK
      /* Now call the user callback function. */
      if(_comQueues[uartIndex].m_txCallback) {
        (*_comQueues[uartIndex].m_txCallback)(_comQueues[uartIndex].m_fd[0], event, _comQueues[uartIndex].m_txUsrParam);
      }
#endif
      break;
    }

    case ARM_USART_EVENT_TX_COMPLETE:
      {
#ifdef _DEBUG
        if(uartIndex != CONSOLE_UART) {
          int x = 0; /* Allows to set a breakpoint for non Console UART events. */
          x++; /* Avoid warning about unused variable. */
        }
#endif

#if OS_COM_ENABLE_WRITE_WITH_FEEDBACK
      /* Now call the user callback function. */
      if(_comQueues[uartIndex].m_txCallback) {
        (*_comQueues[uartIndex].m_txCallback)(_comQueues[uartIndex].m_fd[0], event, _comQueues[uartIndex].m_txUsrParam);
      } else {
        _comQueues[uartIndex].m_txCallback = 0;
        _comQueues[uartIndex].m_txUsrParam = 0;
      }

#endif
        /* The last byte is physically out, so try getting the next message from the transmit queue into
         * the current transmit buffer. */
        popFromTxQueueAndSend(uartIndex);
        break;
      }
    case ARM_USART_EVENT_RECEIVE_COMPLETE:
      {

      ARM_DRIVER_USART* driver = driver_usart_getDriver(uartIndex);

      /* Look and how many bytes we really received. */
      _comQueues[uartIndex].m_rxCurrent.m_len = _comQueues[uartIndex].m_interimRxCount + driver->GetRxCount();

#ifdef _DEBUG
        if(uartIndex != CONSOLE_UART) {
          int x = 0; /* Allows to set a breakpoint for non Console UART events. */
          x++; /* Avoid warning about unused variable. */
        }
#endif

      /* Move the received message to the receive queue. */
      _comQueues[uartIndex].m_rxQueue.allocateAndPut(&_comQueues[uartIndex].m_rxCurrent);

      /* The queue is now responsible for the received message. */
      _comQueues[uartIndex].m_rxCurrent.m_mem = 0;
      _comQueues[uartIndex].m_rxCurrent.m_len = 0;
      _comQueues[uartIndex].m_interimRxCount = 0;

#ifdef _DEBUG
      ARM_USART_STATUS status = (*driver->GetStatus)();
#endif
      _comQueues[uartIndex].m_requestedRxCount = 0;
#ifdef _DEBUG
      status = (*driver->GetStatus)();
#endif
      break;
    }
  }
}
