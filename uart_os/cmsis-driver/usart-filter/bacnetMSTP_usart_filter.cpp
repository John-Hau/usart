#include "baseplate.h"

#include "boards/board-api/bapi_uart.h"
#include "cmsis-driver/Driver_USART.h"
#include "bacnetMSTP_usart_filter.h"
#include "utils/typed_queue.hpp"
#include "boards/board-api/bapi_hwtimer.h"
#include "rtos/cmsis-rtos-ext/osCom.h"
#include "FreeRangeConfig.h"
#include "log-channel/log_channel.h"


#ifdef FREERANGE_STACK_VER00
#include "freerange/freerange-v0.0/include/fr.h"
#include "freerange/freerange-v0.0/include/mstp.h"
#include "freerange/freerange-v0.0/include/mstpSerial.h"
#else //#ifdef FREERANGE_STACK_VER00
#include "freerange/freerange-v14.15/include/fr.h"
#include "freerange/freerange-v14.15/include/mstp.h"
#include "freerange/freerange-v0.0/include/mstpSerial.h"
#endif //#ifdef FREERANGE_STACK_VER00


#include <string.h>

#include "rtos/cmsis-rtos/cmsis_os_redirect.h"

#include "boards/board-api/bapi_io.h"


/**
 * \file
 * \brief This file implements the BACnet MS/TP driver on USART
 *
 */

/**
 * \defgroup _BACnetMSTPDriver_USART Internals
 * \brief BACnet MSTP driver driver hooked on USART (non-exposed functions, types, variables).
 * \ingroup BACnetMSTPDriver_USART
 */

/**
 * \addtogroup _BACnetMSTPDriver_USART
 */
/*@{*/


#define BACNET_MSTP_DRIVER_TRACE_INTERNAL_TX 1
#define BACNET_MSTP_DRIVER_TRACE_RX 0

#if BACNET_MSTP_DRIVER_TRACE_RX

  STATIC uint32_t goodBytes[nMSTPports];
  STATIC uint32_t badBytes [nMSTPports];

  STATIC uint32_t MSTP_RxTraceCounter[nMSTPports]={0};
  STATIC uint8_t  MSTP_RxTraceBuffer[nMSTPports][maxrx];

#endif
#define MSTPRECVBUFLEN 512
 //Implement Cyclic buffer for receiving data from UART
 typedef struct {
  //    uint8    ch;         /* which channel is this structure for? */
      uint16_t	wp;	//written by driver, increased while recv new data from bus
      uint16_t	rp;	//fretch data to mstp layer, increased while sent data to mstp layer
      uint8_t 	buf[MSTPRECVBUFLEN+2];
  }CyclicBuffer;

#define nMSTPports	1	//RAJAT_MEGE_FIX_TODO
static CyclicBuffer MstpCyclicBuffer[nMSTPports];

#if BACNET_MSTP_DRIVER_TRACE_INTERNAL_TX
  STATIC uint32_t MSTP_TraceInternalTxWatermark[nMSTPports]={0};
#endif

STATIC bool frInitialized = false;

C_INLINE void appControlMSTP_TxLed(bool ledState)
{
#ifdef BACNET_MSTP_TX_LED
	bapi_bo_setBoState(bapi_bo_E_BACnet_LED_TX,ledState);
#endif
}
C_INLINE void appControlMSTP_RxLed(bool ledState)
{
#ifdef BACNET_MSTP_RX_LED
	bapi_bo_setBoState(bapi_bo_E_BACnet_LED_RX,ledState);
#endif
}
/**
 * @ingroup _BACnetMSTPDriver_USART
 * @brief
 * This structure keeps the runtime information of transmission in progress for all active MSTP UARTS
 */
typedef struct {
  uint8_t* TxBuffPtr; // this points to the osCom buffer
  uint32_t TxCount;
  uint32_t TxBytesSent;

  bapi_E_UartIndex_ uartIndex;
  uint8_t TxData;
  uint8_t xmit_ff_state;
  uint8_t xmit_ff_wait;
  uint8_t xmit_data_wait;

  uint8_t isInternalCall : 1;
  uint8_t inUse : 1;

  // Gerhard Bahr, 24. July 2017:
  // maxtx is the max data size without header and CRC and padding!!!
  // we need to add some more space here!!
  // look at mstpTransmit() function of the FreeRange stack
  // you'll find that nine is added to the data length: "fp->plen = dlen + 9; //account for preamble,hdr and pad"
  // let's add 10
  uint8_t  TxBuff[maxtx+10]; // local Buffer used for internal Send function of MSTP driver

} MSTPTransmit;


#define MSTP_TRANSMISSION_INITIALIZATION {0,0,0,bapi_E_Uart_Invalid,0,0,0,0,false,false, {0}}

MSTPTransmit MSTPTransmission[nMSTPports] = {
  MSTP_TRANSMISSION_INITIALIZATION,
#if nMSTPports > 1
  MSTP_TRANSMISSION_INITIALIZATION,
#endif
#if nMSTPports > 2
  MSTP_TRANSMISSION_INITIALIZATION,
#endif
#if nMSTPports > 3
  MSTP_TRANSMISSION_INITIALIZATION,
#endif
};

static unsigned allocateFreeMSTPPort(enum bapi_E_UartIndex_ uartIndex) {

  for (int i = 0; i < ARRAY_SIZE(MSTPTransmission); i++ ) {
    if (MSTPTransmission[i].uartIndex == bapi_E_Uart_Invalid)
      {
      MSTPTransmission[i].uartIndex = uartIndex;
      return i;
    }
  }

  return nMSTPports;

}

unsigned bacnetMSTP_driver_uartIndexToMSTPPort(enum bapi_E_UartIndex_ uartIndex)
  {
  for (int i = 0; i < ARRAY_SIZE(MSTPTransmission); i++ )
    {
    if (MSTPTransmission[i].uartIndex == uartIndex)
      {
      return i;
    }
  }

  /* Signal error by a value that is 1 above the highest mstp port. */
  return nMSTPports;
}

bapi_E_UartIndex bacnetMSTP_driver_MSTPPortToUartIndex(unsigned port)
  {

  return(MSTPTransmission[port].uartIndex);
}

inline MSTPTransmit* getMstpTransmissionByPort(unsigned port) {
  return (port < nMSTPports) ? &MSTPTransmission[port] : S_CAST(MSTPTransmit*, 0);
}

inline MSTPTransmit* getMstpTransmissionByUart(enum bapi_E_UartIndex_ uartIndex) {
  unsigned port = bacnetMSTP_driver_uartIndexToMSTPPort(uartIndex);
  return getMstpTransmissionByPort(port);
}


static void freeMstpTransmissionByUart(enum bapi_E_UartIndex_ uartIndex) {
  MSTPTransmit* transmission = getMstpTransmissionByUart(uartIndex);

  /* Assert the we free up something that was allocated before. */
  ASSERT(transmission);

  if(transmission) {
    transmission->uartIndex = bapi_E_Uart_Invalid;
  }
}

#if useFASTMSTP

template<unsigned port> struct MSTPSlotTimerCallback {
  static void inputFunc(void* param) {
    fmstpRfeSlotTimerInterruptCallback(port);
  }
};

fmstpSlotTimerCallback SlotTimerCallbackArray[] = {
   MSTPSlotTimerCallback<portMSTP0>::inputFunc /* Index 0 maps to port 0 (which may be not portMSTP0) */
#if(nMSTPports >1)
  ,MSTPSlotTimerCallback<portMSTP1>::inputFunc /* Index 0 maps to port 1 (which may be not portMSTP1) */
#endif
#if nMSTPports > 2
  ,MSTPSlotTimerCallback<portMSTP2>::inputFunc /* Index 2 maps to port 2 (which may be not portMSTP2) */
#endif
#if nMSTPports > 3
  ,MSTPSlotTimerCallback<portMSTP3>::inputFunc /* Index 3 maps to port 0 (which may be not portMSTP3) */
#endif
#if nMSTPports > 4
#error "More than 4 MSTP Ports defined, please enhance"
#endif
};

#endif

#if TARGET_RTOS == RTOS_NoRTOS

STATIC UINT32 last_run_tick = 0;

void bacnetMSTP_driver_bgnd_timerpoll(void)
  {
  UINT32 myTickcount;
  myTickcount = bapi_getSystemTick();
  if (myTickcount - last_run_tick >= 5)
    {
    //MSTP Work
    frWork(myTickcount - last_run_tick);
    last_run_tick = myTickcount;
  }
}
#else
void bacnetMSTP_driver_executeStateMachine(uint32_t aTimeElapsed)
{
  frWork(aTimeElapsed);
}
#endif

typedef int32_t (*_receiveFunction_type)(enum bapi_E_UartIndex_ uartIndex, uint32_t, void *data, uint32_t num);
typedef int32_t (*_sendFunction_type)(enum bapi_E_UartIndex_ uartIndex, void const *data, uint32_t num);
typedef int32_t (*_initializeFunction_type)(enum bapi_E_UartIndex_ uartIndex, ARM_USART_SignalEvent_t cb_event);


/* Use namespace to avoid conflict with other modules*/
namespace bacnetMSTPDriver
{
typedef driver_usart_Hooks replacedHooks_t;

/**
 * \brief
 * this structure declares all data that the driver needs to operate.
 */
struct DriverData {
  /**
   * \brief Here we save the original Tx_ISRCallbacks.
   */
  bapi_uart_Tx_ISRCallbacks m_messageTransmissionComplete_ISRCallback;

  /**
   * \brief Here we save the original dataReceived_ISRCallback.
   */
  bapi_uart_dataReceived_ISRCallback_t m_dataReceived_ISRCallback;

  /**
   * \brief Here we save the original Signal Event callback.
   */
  _driver_usartDriverHookSignalEvent_t m_signalEventCallback;

  /**
   * \brief Here we save the original hook functions which are replaced by
   * this USART filter in the CMSIS USART Driver.
   */
  replacedHooks_t m_replacedHookFunctions;

  /**
   * \brief A flag that tells whether an hook/unhook operation is currently in
   * progress.
   */
  uint8_t m_busy   :1;

  /**
   * \brief A flag that tells whether this driver is already hooked.
   */
  uint8_t m_hooked :1;
};

STATIC struct DriverData driverData[bapi_E_UartCount];

/*------------------------------------------------------------------------*//**
 * \ingroup _BACnetMSTPDriver_USART
 * \brief Our Signal Event callback
 */
STATIC void driver_usart_onEvent(const enum bapi_E_UartIndex_ uartIndex, uint32_t event) {
  if (driverData[uartIndex].m_signalEventCallback) {
    /* We just call the original Signal Event callback. */
    (*driverData[uartIndex].m_signalEventCallback)(uartIndex, event);
  }
}

/**
 * \ingroup _BACnetMSTPDriver_USART
 * \brief
 * The callback that will be called by the bapi_uart module upon a TRANSMISSION COMPLETE event
 */
STATIC void _arm_usart_mstp_msgTransmissionComplete_ISRCallback(
  struct bapi_uart_TransmissionState* transmissionState, /**< [in] The transmission state that the ISR
   * associated with the callback event. */
  uint32_t event /**< [in] ARM_USART_EVENT_SEND_COMPLETE in case that all bytes have been moved to the
   * UART send register or UART FIFO. ARM_USART_EVENT_TX_COMPLETE in case that the
   * last byte was physically transmitted. */
  ) {
#ifdef _DEBUG
  ASSERT(transmissionState->m_uartIndex >= 0);
  ASSERT(transmissionState->m_uartIndex < bapi_E_UartCount);
#endif

  MSTPTransmit* transmission = getMstpTransmissionByUart(transmissionState->m_uartIndex);

#ifdef _DEBUG
  ASSERT(transmission);
#endif
  appControlMSTP_TxLed(true);
  switch ( event )
  {
    case ARM_USART_EVENT_SEND_COMPLETE:
    {
      if (transmission->xmit_data_wait == 1)
      {
        // We will now get the TX_COMPLETE event flag
        transmission->xmit_data_wait = 0;
        return;
      }

      if (transmission->xmit_ff_state != 0)
      {
        /* We send a 0xFF when "xmit_ff_state" is 5..1 (after decrementing),
         * but not on the transition from 1 to 0*/

        //send 0xFF with transmitter off
        transmission->TxData = 0xFF;
        // setup transmission state
        bapi_irq_enterCritical();
        if (!bapi_uart_isInUse(transmissionState)) {
          bapi_uart_init_TransmissionState(transmissionState, &transmission->TxData, 1, bapi_uart_E_TxMode_TxOff);
          bapi_uart_startTx(transmissionState->m_uartIndex);
        }
        bapi_irq_exitCritical();
        transmission->xmit_ff_state--;
        return;
      }
      else
      {
        if(transmission->TxBytesSent >= transmission->TxCount){
          //oops; we just sent the data it seems

          //We just sent out the last byte
          transmission->TxBytesSent = 0;
          transmission->TxCount = 0;
          transmission->xmit_data_wait = 1;

          //pad byte
          transmission->TxData = 0xFF;
          bapi_irq_enterCritical();
          if (!bapi_uart_isInUse(transmissionState)) {
             bapi_uart_init_TransmissionState(transmissionState, &transmission->TxData, 1, bapi_uart_E_TxMode_TxOff);
             bapi_uart_startTx(transmissionState->m_uartIndex);
           }
           bapi_irq_exitCritical();
           return;
        }
        else{
          //FF state is zero; but we cant go ahead; we have to return
          transmission->xmit_ff_wait = 1;
          return;
        }
      }

/* WSC: Commented out code below, because compiler says it is unreachable code. */
//      const uint8_t installedMSTPPort = getInstalledMSTPPort(transmissionState->m_uartIndex);
//      if (fmstpIsRfeResponseInitiated(installedMSTPPort) == TRUE)
//      {
//        if ((fmstpGetMstpMessageState(installedMSTPPort) != 0) || (fmstpCheckRfeResponseAbortStatus(installedMSTPPort) == TRUE))
//          {
//          //Abort RFE Response Transmission
//          fmstpRfeResponseDone (installedMSTPPort);
//          //reset transmitting flag
//          mstpResetTxFlag(installedMSTPPort);
//          ARM_DRIVER_USART* mstpDriver = osComArmDriverUsartGet(transmissionState->m_uartIndex);
//          mstpDriver->Control(transmissionState->m_uartIndex, ARM_USART_ABORT_SEND);
//          return;
//        }
//      }
//      break;
    }

    case ARM_USART_EVENT_TX_COMPLETE:
    {
      if(transmission->xmit_ff_wait == 1){
        //We just transmitted or started the transmission of the last 0xFF
        //we need to send data now
        //Safe to turn on transmitter now
        bapi_uart_setInterfaceFlag(transmissionState->m_uartIndex, bapi_E_RS485_EnableTransmitter, 1);
        if (transmission->TxBytesSent < transmission->TxCount)
          {
          // setup transmission state
          // bapi_uart_TransmissionState* transmissionState = driver_usart_getTransmissionState(MSTP_UART);
          bapi_irq_enterCritical();
          if (!bapi_uart_isInUse(transmissionState)) {
            bapi_uart_init_TransmissionState(transmissionState, transmission->TxBuffPtr, transmission->TxCount, bapi_uart_E_TxMode_Transparent);
            bapi_uart_startTx(transmissionState->m_uartIndex);
            transmission->TxBytesSent += transmission->TxCount;
          }
          bapi_irq_exitCritical();
        }
        transmission->xmit_ff_wait = 0;
        //we should now get data reg empty event
        return;
      }

      const unsigned installedMSTPPort = bacnetMSTP_driver_uartIndexToMSTPPort(transmissionState->m_uartIndex);
#ifdef FREERANGE_STACK_VER00
      mstpResetTxFlag(installedMSTPPort);
#else //#ifdef FREERANGE_STACK_VER00
      //mstpResetTxFlag(installedMSTPPort);// TODO HM
#endif //#ifdef FREERANGE_STACK_VER00

      bapi_uart_setInterfaceFlag(transmissionState->m_uartIndex, bapi_E_RS485_EnableTransmitter, 0);
      //call event callbacks only if the Send call was not internal;
      if(!transmission->isInternalCall){
        driver_usart_onEvent(transmissionState->m_uartIndex, ARM_USART_EVENT_SEND_COMPLETE); // this frees up the OsCom buffer
        driver_usart_onEvent(transmissionState->m_uartIndex, event);
      }
      transmission->isInternalCall = 0;
      transmission->inUse = 0;
      break;
    }
  }

  appControlMSTP_TxLed(false);
}

/**
 * \ingroup BACnetMSTPDriver_USART
 * \brief
 * The callback that will be called by the bapi_uart module to dispose 1 or more
d characters.
 * \details
 * It calls the BACnet stack <STRONG>ReceiveFrameStateMachine()</STRONG>.
 *
 */
// unsigned int numBytes = 0;
STATIC bapi_uart_MaxFrameSize_t usart_mstp_dataReceived_ISRCallback(
  const enum bapi_E_UartIndex_ uartIndex, /**< The source UART of the received character(s). */
  const uint32_t errorEvents, /**< ? */
  const uint8_t rx_chars[], /**< The received character(s) */
  bapi_uart_MaxFrameSize_t count /**< The number of received characters */
  ) {

  // we have received a character(s)
  uint8_t installedMSTPPort = bacnetMSTP_driver_uartIndexToMSTPPort(uartIndex);

#if useFASTMSTP
  //Check if in MS/TP is waiting for an indication byte
  if (fmstpGetMstpState(installedMSTPPort) == mnsmWaitForRfeIndication)
  {
    fmstpRegisterByteReceived(installedMSTPPort);
  }

#endif

  // Check if a byte is received OK or is received in error
  if (errorEvents == 0)
  {

#if BACNET_MSTP_DRIVER_TRACE_RX

    goodBytes[installedMSTPPort]++;

    MSTP_RxTraceBuffer[installedMSTPPort][MSTP_RxTraceCounter[installedMSTPPort]++] = rx_chars[0];
    MSTP_RxTraceCounter[installedMSTPPort] %= ARRAY_SIZE(MSTP_RxTraceBuffer[installedMSTPPort]);

#endif

#if useFASTMSTP
    if(fmstpIsTransmissionInProgress(installedMSTPPort) == TRUE)
    {
      //we received something when transmission was already triggerred - > indicates collision
      if(rx_chars[0] != 0xFF)
      {
        if(fmstpIsRfeResponseInitiated(installedMSTPPort) == TRUE)
        {
          //abort response transmission
          fmstpAbortRfeResponse(installedMSTPPort);
        }
      }
    }
#endif
#if (useAutoBaud && enableOnTheFlyBaudRateHunt)
    //Check if some transmission was in progress
  	  MSTPTransmit* transmission = getMstpTransmissionByUart(uartIndex);
  	  if(transmission->inUse)
  	  {

  		  if(rx_chars[0] != 0xFF)
  		  {
  			  //Collision
  			  setup_listen_mode(installedMSTPPort);
  			  return count;
  		  }
  	  }
#endif
    //Byte received OK; This has been changed now; ReceiveFrameStateMachine will not be executed from UART ISR now
    //Instead the bytes are now stored in a cyclic buffer which is then accessed by freerange driver
    //ReceiveFrameStateMachine(installedMSTPPort, rx_chars[0]);
    MstpCyclicBuffer[installedMSTPPort].buf[MstpCyclicBuffer[installedMSTPPort].wp] = rx_chars[0];
    MstpCyclicBuffer[installedMSTPPort].wp = MstpCyclicBuffer[installedMSTPPort].wp + 1;
    if(MstpCyclicBuffer[installedMSTPPort].wp >= MSTPRECVBUFLEN)
    {
    	MstpCyclicBuffer[installedMSTPPort].buf[MSTPRECVBUFLEN] = 0xFF;
    	MstpCyclicBuffer[installedMSTPPort].wp = 0;
    }
  }
  else
  {
#if (useAutoBaud && enableOnTheFlyBaudRateHunt)
	  //Byte received in error
    //Check if some transmission was in progress
	  MSTPTransmit* transmission = getMstpTransmissionByUart(uartIndex);
	  if(transmission->inUse)
	  {
		  //Collision
		  setup_listen_mode(installedMSTPPort);
		  return count;
	  }
#if BACNET_MSTP_DRIVER_TRACE_RX

    badBytes[installedMSTPPort]++;

#endif

#if useFASTMSTP
    if(fmstpIsTransmissionInProgress(installedMSTPPort) == TRUE)
    {
        //we received something when transmission was already triggerred - > indicates collision
        if(fmstpIsRfeResponseInitiated(installedMSTPPort) == TRUE)
        {
          //abort response transmission
          fmstpAbortRfeResponse(installedMSTPPort);
        }
    }
#endif


    ReceivedErrorByte(installedMSTPPort, rx_chars[0]);
#endif
  }
  return count;

}


/**
 * \ingroup _BACnetMSTPDriver_USART
 * \brief
 * The callback that will be called by the bapi_uart module to obtain the current transmission state
 * for a particular USART.
 */
STATIC bapi_uart_TransmissionState* _mstp_usart_getTransmissionState_ISRCallback(
  const enum bapi_E_UartIndex_ uartIndex /** The source UART that has invoked the ISR. */
  ) {
  return driver_usart_getTransmissionState(uartIndex);
}


/**
 * \ingroup _BACnetMSTPDriver_USART
 * \brief
 * This is the initialize hook
 */
STATIC int32_t Initialize(
  const enum bapi_E_UartIndex_ uartIndex,
  _driver_usartDriverHookSignalEvent_t driverHook_cb_event) {

  int32_t retval = ARM_DRIVER_ERROR;

  if (driverData[uartIndex].m_replacedHookFunctions.Initialize) {
    /* Here we hook our own Signal Event callback */
    retval = (*driverData[uartIndex].m_replacedHookFunctions.Initialize)(uartIndex, driver_usart_onEvent);
  }

  if (retval == ARM_DRIVER_OK) {
    /* Our initialize function saves the original Signal Event callback that was passed to us. */
    driverData[uartIndex].m_signalEventCallback = driverHook_cb_event;

    driverData[uartIndex].m_dataReceived_ISRCallback = bapi_uart_setDataReceived_ISRCallback(uartIndex,
      usart_mstp_dataReceived_ISRCallback);

    driverData[uartIndex].m_messageTransmissionComplete_ISRCallback = bapi_uart_setMsgTransmission_ISRCallbacks(
      uartIndex, _arm_usart_mstp_msgTransmissionComplete_ISRCallback, _mstp_usart_getTransmissionState_ISRCallback);
    /* Assert that the old hook is not the new one. Otherwise well get recursive calls with stack overflow. */
    ASSERT(driverData[uartIndex].m_dataReceived_ISRCallback != usart_mstp_dataReceived_ISRCallback);

    /* Here we could do some own additional post - initialization. */
    bapi_irq_enterCritical();

#if useFASTMSTP
    bapi_HwtimerHandle availableTimer = bapi_hwt_allocateHardwareTimer();


    if(availableTimer != 0)
    {
      unsigned freePort = allocateFreeMSTPPort(uartIndex);
    	bapi_hwt_configureTimer(availableTimer, bapi_Hwt_E_Periodic, 195);

    	bapi_hwt_installTimeoutCallback(availableTimer, SlotTimerCallbackArray[freePort], 0);
    	fmstpRegisterRfeSlotTimer(freePort, availableTimer);
    }
    else
    {
    	//Hardware timer could not be configured for Fast MS/TP functionality
    	bapi_fatalError(0,0);
    }
#else
    allocateFreeMSTPPort(uartIndex);
#endif

    if(frInitialized == false){
      // First time BACnet MSTP driver is initialised
#ifdef FREERANGE_STACK_VER00
    	FreeRangeAppInit();
#else //#ifdef FREERANGE_STACK_VER00
    	//FreeRangeAppInit();// TODO HM
#endif //#ifdef FREERANGE_STACK_VER00
    	uint8_t installedMSTPPort = bacnetMSTP_driver_uartIndexToMSTPPort(uartIndex);
    	MstpCyclicBuffer[installedMSTPPort].wp = 0;
    	MstpCyclicBuffer[installedMSTPPort].rp = 0;
    	MEMSET(MstpCyclicBuffer[installedMSTPPort].buf, 0, MSTPRECVBUFLEN+1);

//    	frStartup();
    	frInitialized = true;
    }
    bapi_irq_exitCritical();
  }
  return retval;
}

/**
 * \ingroup _BACnetMSTPDriver_USART
 * \brief
 * This is the uninitialize hook
 */
STATIC int32_t Uninitialize(const enum bapi_E_UartIndex_ uartIndex) {

	  /* Here we could do some own additional pre - uninitialization. */
    freeMstpTransmissionByUart(uartIndex);

	  /* ...and passes our own Signal Event Callback to the original Initialize function. */
	  int32_t retval = ARM_DRIVER_ERROR;

	  if(driverData[uartIndex].m_dataReceived_ISRCallback) {
	    /* Uninstall our ISR callback */
	    bapi_uart_dataReceived_ISRCallback_t currentCallback = bapi_uart_setDataReceived_ISRCallback(uartIndex, driverData[uartIndex].m_dataReceived_ISRCallback);

	    /* Assert that the current callback was ours. */
	    ASSERT(currentCallback == usart_mstp_dataReceived_ISRCallback);
	    driverData[uartIndex].m_dataReceived_ISRCallback = 0;
	  }

	  if(driverData[uartIndex].m_replacedHookFunctions.Uninitialize){

	    /* Call the original uninitialize function. */
	    retval = (*driverData[uartIndex].m_replacedHookFunctions.Uninitialize)(uartIndex);

	    if(retval == ARM_DRIVER_OK) {
	      /* Our Signal Event call back is de-installed, now we can forget the original
	       * Signal Event callback. */
	    	driverData[uartIndex].m_signalEventCallback = 0;

	    } else {
	      /* Uninitialization failed, so roll back ! */

	      /* Re - hook ourselves into the bapi receive ISR callback event. */
	    	driverData[uartIndex].m_dataReceived_ISRCallback = bapi_uart_setDataReceived_ISRCallback(
	        uartIndex, usart_mstp_dataReceived_ISRCallback);
	    }
	  }

	  return retval;
}

/**
 * \ingroup _BACnetMSTPDriver_USART
 * \brief
 * This is a dummy Receive hook, which should never be called
 */
STATIC int32_t Receive(const enum bapi_E_UartIndex_ uartIndex, void *data, uint32_t num) {
  /* This function is never used in BACnet MS/TP application - the ReceiveFrameStateMachine is
   * interrupt driven and executed as and when individual bytes are received */
  ASSERT(false);
  return ARM_DRIVER_ERROR;
}


/**
 * \ingroup _BACnetMSTPDriver_USART
 * \brief
 * This is the Send hook
 */
STATIC int32_t Send(const enum bapi_E_UartIndex_ uartIndex, const void *data, uint32_t num) {
  ASSERT(data);
  ASSERT(num);

  int32_t retval = ARM_DRIVER_ERROR_BUSY;

  if (!data || !num) {
    return ARM_DRIVER_ERROR_PARAMETER;
  }

  MSTPTransmit* transmission = getMstpTransmissionByUart(uartIndex);

  ASSERT(transmission);

  transmission->inUse = 1;
  transmission->TxData = 0xFF;

  //Send is always called via osCom; hence we know there is a queue available to us and we can right away use that; local buffering not required
  transmission->TxBuffPtr = (uint8_t*)data;

  transmission->TxCount = num;
  transmission->xmit_ff_state = 5;
  bapi_uart_TransmissionState* transmissionState = driver_usart_getTransmissionState(uartIndex);

  bapi_irq_enterCritical();
  if (!bapi_uart_isInUse(transmissionState)) {
    bapi_uart_init_TransmissionState(transmissionState, &transmission->TxData, 1, bapi_uart_E_TxMode_Transparent);
    bapi_uart_startTx(uartIndex);
    retval = ARM_DRIVER_OK;
  }
  bapi_irq_exitCritical();

  return retval;
}

/**
 * \ingroup _BACnetMSTPDriver_USART
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
  NULL
};

} /* namespace bacnetMSTPDriver */


/* These are the C - API functions that cannot go into a name space. */

int32_t bacnetMSTP_driver_hookDriver(
  enum bapi_E_UartIndex_ uartIndex
  ) {

	  bapi_irq_enterCritical();
	  if (!bacnetMSTPDriver::driverData[uartIndex].m_busy
	    && !bacnetMSTPDriver::driverData[uartIndex].m_hooked) {

	    /* Make the oldReceiveFunction nonzero, so that no other thread can step in here. */
		  bacnetMSTPDriver::driverData[uartIndex].m_busy = true;
	    bapi_irq_exitCritical();

	    int32_t retval = driver_usart_setHooks(uartIndex, &bacnetMSTPDriver::_hookFunctions
	      , &bacnetMSTPDriver::driverData[uartIndex].m_replacedHookFunctions);

	    if (retval != ARM_DRIVER_OK) {
	      /* Cleanup upon error. */
	      driver_usart_initHooks(&bacnetMSTPDriver::driverData[uartIndex].m_replacedHookFunctions);
	    }
	    else {

	    }

	    bacnetMSTPDriver::driverData[uartIndex].m_hooked = true;
	    bacnetMSTPDriver::driverData[uartIndex].m_busy = false;

	    return retval;
	  }

	  bapi_irq_exitCritical();

	  /* Hooking/Unhooking already in progress, or the driver is already hooked. */
	  return ARM_DRIVER_ERROR;;
}

int32_t bacnetMSTP_driver_unhookDriver(
  enum bapi_E_UartIndex_ uartIndex
  ) {
	  bapi_irq_enterCritical();
	  if(!bacnetMSTPDriver::driverData[uartIndex].m_busy && !bacnetMSTPDriver::driverData[uartIndex].m_hooked) {

	    bapi_irq_exitCritical();
	    if(bacnetMSTPDriver::driverData[uartIndex].m_signalEventCallback == 0) {

	      int32_t retval = driver_usart_setHooks(uartIndex,
	        &bacnetMSTPDriver::driverData[uartIndex].m_replacedHookFunctions, 0);

	      bacnetMSTPDriver::driverData[uartIndex].m_hooked = false;
	      bacnetMSTPDriver::driverData[uartIndex].m_busy   = false;

	      return retval;
	    }

	    /* Driver is still initialized, cannot unhook. Uninitialize before unhook. */
	    return ARM_DRIVER_ERROR;
	  }

	  bapi_irq_exitCritical();

	  /* Hooking/Unhooking already in progress, or the driver is already unhooked. */
	  return ARM_DRIVER_ERROR;
}


void bacnetMSTP_driver_executeBacApp(void)
{
	frMain();
}

static int32_t _bacnetMSTP_driver_send(const enum bapi_E_UartIndex_ uartIndex, const void* data, uint32_t num)
  {

  /* This is an internal Send function - should be called internally from BACnet MSTP driver context */
  /* Set a flag indicating the driver that this is an internal send call */
  int32_t retval = ARM_DRIVER_ERROR_BUSY;

  MSTPTransmit* transmission = getMstpTransmissionByUart(uartIndex);
  ASSERT(transmission);

  if (transmission->inUse == 1)
    return retval;

  bapi_irq_enterCritical();
  transmission->inUse = 1;
  transmission->isInternalCall = 1;
  bapi_irq_exitCritical();

  transmission->TxData = 0xFF;

#if BACNET_MSTP_DRIVER_TRACE_INTERNAL_TX
  {
    const unsigned port = bacnetMSTP_driver_uartIndexToMSTPPort(uartIndex);
    MSTP_TraceInternalTxWatermark[port] =
      MAX(num, MSTP_TraceInternalTxWatermark[port]);
  }
#endif

  /*
   * Increase MSTPTransmit. TxBuff if this assertion fails.
   * Use MSTP_TraceInternalTxWatermark to find out the
   * minimum size of TxBuff.
   */
  ASSERT(num <= sizeof(transmission->TxBuff));

  MEMCPY(transmission->TxBuff, data, num);

  transmission->TxBuffPtr = transmission->TxBuff;
  transmission->TxCount = num;
  transmission->xmit_ff_state = 5;

  bapi_uart_TransmissionState* transmissionState = driver_usart_getTransmissionState(uartIndex);

  bapi_irq_enterCritical();

  if (!bapi_uart_isInUse(transmissionState)) {
    bapi_uart_init_TransmissionState(transmissionState, &transmission->TxData, 1, bapi_uart_E_TxMode_Transparent);
    bapi_uart_startTx(uartIndex);
    retval = ARM_DRIVER_OK;
  }

  bapi_irq_exitCritical();

  while (transmission->inUse == 1){
    osDelay(200);
  }

  return retval;
}

/**
 * New function to monitor if MSTP UART is busy in transmission
 * Return :
 * True : if busy in sending
 * False : If Free for new transmission
 *
 */
bool bacnetMSTP_driver_IsBusy(uint8_t port)
{
       MSTPTransmit* transmission = getMstpTransmissionByPort(port);
       if(transmission != NULL){
    	   return(transmission->inUse);
       }
       else{
    	   return false;
       }

}


///////////////////////////////////////////////////////////////////////
//  Send a buffer of characters(replaces mstpSerialTX)
//
//in: port  the port number (0=maxSERIAL-1)                     ***212 Begin
//    b   byte to send
//    n   number to send (assuming it contains padding byte 0xFF at the end.
//                        subtract one, because
//                        bacnetMSTP_driver_hook_USART adds a padding byte)
//out:  false SUCCESS

// ??? Gerhard Bahr: "Why are there two function that do exactly the same? Can anybody explain?"

bool SerialTxBuf(byte port,byte *b,word n)
{
  if(MSTPTransmission[port].uartIndex != bapi_E_Uart_Invalid)
  {
    return _bacnetMSTP_driver_send(MSTPTransmission[port].uartIndex, b, n-1);
  }

  /* If Index is not valid -> UART not initialized for BACnet communication. */
  /* We must return OK, so that free ranges keeps working for the initialized
   * UARTs. */
  return ARM_DRIVER_OK;

}

bool appSerialTxBuf(byte port,byte *b,word n)
{

  if (MSTPTransmission[port].uartIndex != bapi_E_Uart_Invalid)
    {
    return _bacnetMSTP_driver_send(MSTPTransmission[port].uartIndex, b, n-1);
  }

  /* If Index is not valid -> UART not initialized for BACnet communication. */
  /* We must return OK, so that free ranges keeps working for the initialized
   * UARTs. */
  return ARM_DRIVER_OK;
}

bool tc_appSerialTxBuf(uint8_t port,uint8_t *b,uint8_t n)
{

  if (MSTPTransmission[port].uartIndex != bapi_E_Uart_Invalid)
    {
    return _bacnetMSTP_driver_send(MSTPTransmission[port].uartIndex, b, n-1);
  }

  /* If Index is not valid -> UART not initialized for BACnet communication. */
  /* We must return OK, so that free ranges keeps working for the initialized
   * UARTs. */
  return ARM_DRIVER_OK;
}

static int MSTP_getchar_present(uint8_t aPort)
{
	if((MstpCyclicBuffer[aPort].wp + 1) == MstpCyclicBuffer[aPort].rp)
	{
		//Overflow
		iprintf_LchSysMstpRFSM(LCHP_HIGH, "MSTP UART data overflow");
	}
	return((MstpCyclicBuffer[aPort].wp > MstpCyclicBuffer[aPort].rp) || (MstpCyclicBuffer[aPort].buf[MSTPRECVBUFLEN] == 0xFF));
}

static uint8_t MSTP_getChar(uint8_t aPort)
{
	uint8_t ch;
	ch = MstpCyclicBuffer[aPort].buf[MstpCyclicBuffer[aPort].rp];
	MstpCyclicBuffer[aPort].rp = MstpCyclicBuffer[aPort].rp + 1;
    if((MstpCyclicBuffer[aPort].rp) >= MSTPRECVBUFLEN)
    {
    	MstpCyclicBuffer[aPort].buf[MSTPRECVBUFLEN] = 0x0;
    	MstpCyclicBuffer[aPort].rp=0;
    }
    return ch;
}

int bacnetMSTP_driver_SerialRx(uint8_t aPort)
{
	if(MSTP_getchar_present(aPort))
	{
		appControlMSTP_RxLed(true);
		return(int)(MSTP_getChar(aPort));
	}
	else
	{
		appControlMSTP_RxLed(false);
		return -1;
	}
}

///* Currently this function does nothing; Micro Seconds time not required in the current implementation */
//unsigned long int GetTimeinMicroSecs(void)
//{
// return 0;
//}

/* microSecsSleep function - currently does nothing*/
void microSecsSleep (dword x)
{
  return;
}
