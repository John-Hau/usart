
#ifndef _BACNETMSTPDriver_DriverUsart_H_
#define _BACNETMSTPDriver_DriverUsart_H_

#include "baseplate.h"
#include "cmsis-driver/Driver_USART.h"


#ifdef EM_DK3750
	#include "boards/EM_DK3750/bapi_io_EM_DK3750.h"
#elif defined (FS_IRMFCU)
  #include "boards/FS_IRMFCU/bapi_io_FS_IRMFCU.h"
#elif defined (FS_IRMLC)
  #include "boards/FS_IRMLC/bapi_io_FS_IRMLC.h"
#elif defined (FS_IRMCT)
  #include "boards/FS_IRMCT/bapi_io_FS_IRMCT.h"
#elif defined (FS_IRM_BIO)
  #include "boards/FS_IRM_BIO/bapi_io_FS_IRM_BIO.h"
#elif defined (FS_IRMLC_KL17Z)
  #include "boards/FS_IRMLC-KL17Z/bapi_io_FS_IRMLC-KL17Z.h"
#elif defined (FS_FRDM_KL46Z)
  #include "boards/FS_FRDM-KL46Z/bapi_io_FS_FRDM-KL46Z.h"
#elif defined (FS_FRDM_K64F)
  #include "boards/FS_FRDM-K64F/bapi_io_FS_FRDM-K64F.h"
#elif defined (FS_FRDM_K66F)
  #include "boards/FS_FRDM-K66F/bapi_io_FS_FRDM-K66F.h"
#elif defined (FS_IRMCT)
  #include "boards/FS_IRMCT/bapi_io_FS_IRMCT.h"
#elif defined (CCS_CVAHU)
  #include "boards/CCS_CVAHU/bapi_io_CCS_CVAHU.h"
#elif defined (FS_IRMFCU_BL)
  #include "boards/FS_IRMFCU_BL/bapi_io_FS_IRMFCU_BL.h"
#elif defined (FS_IRMVAV)
  #include "boards/FS_IRMVAV/bapi_io_FS_IRMVAV.h"
#elif defined (FS_IMXRTEVAL)
  #include "boards/FS_IMXRTEVAL/bapi_io_FS_IMXRTEVAL.h"
#elif defined (FS_IMXRT_TSTAT)
  #include "boards/FS_IMXRT_TSTAT/bapi_io_FS_IMXRT_TSTAT.h"
#elif defined (FS_IPVAV)
  #include "boards/FS_IPVAV/bapi_io_FS_IPVAV.h"
#elif defined (FS_SNAP_ON_IO)
  #include "boards/FS_SNAP_ON_IO/bapi_io_FS_SNAP_ON_IO.h"
#elif defined (FS_BEATS_IO)
  #include "boards/FS_BEATS_IO/bapi_io_FS_BEATS_IO.h"
#else
	#error "Fatal Error: Unknown hardware board."
#endif


/**
 * \defgroup BACnetMSTPDriver_USART BACnet MSTP USART Filter
 * \ingroup usart_filters
 * \brief This USART filter does a special send and receive handling for BACnet/MSTP
 *  with PolarSoft(R) FreeRange(TM) BACnet stack.
 *  \details
 *  For details on the BACnet stack refer to the documentation from PolarSoft and to ANSI/ASHRAE 135.
 *
 *  The <STRONG>Master Node State Machine</STRONG> is executed by bacnetMSTP_driver_executeStateMachine().
 *
 *  The <STRONG>Receive Frame State Machine</STRONG> is executed by the usart_mstp_dataReceived_ISRCallback().
 *
 * \note
 * bapi_E_UartIndex_ <STRONG>uartIndex</STRONG> is the system's UART Index.<BR>
 * unsigned <STRONG>port</STRONG> is the BACnet stack's port number (portMSTP0 .. nMSTPports-1)<BR>
 *
 * \sa cmsis_driver_usart_ext_hook
 */


typedef uint16_t BACnetMSTP_driver_bufferIndex_t;

/**
 * \ingroup BACnetMSTPDriver_USART
 * \brief
 * Install the hooks
 * \param[in] uartIndex
 * \return ARM_DRIVER_OK if successful. ARM_DRIVER_ERROR_BUSY if the driver is initialized.
 */
C_FUNC int32_t bacnetMSTP_driver_hookDriver(
  enum bapi_E_UartIndex_ uartIndex
);

/**
 * \ingroup BACnetMSTPDriver_USART
 * \brief
 * Uninstalls the hooks
 * @param[in] uartIndex
 * \return ARM_DRIVER_OK if successful. ARM_DRIVER_ERROR_BUSY if the driver is initialized.
 */
C_FUNC int32_t bacnetMSTP_driver_unhookDriver(
  enum bapi_E_UartIndex_ uartIndex
);


#if TARGET_RTOS == RTOS_NoRTOS
/**
 * \ingroup BACnetMSTPDriver_USART
 * \brief
 * This function is executed every 5 ms and is responsible for processing the MS/TP packets received
 * and keep track of timings: To be used in an Non RTOS Environment
 * @param None
 * @return None
 */
C_FUNC void bacnetMSTP_driver_bgnd_timerpoll(void);
#else
/**
 * \ingroup BACnetMSTPDriver_USART
 * \brief Execute the BACnet/MSTP <STRONG>Master Node State Machine</STRONG>
 * \details
 *  <P>
 *  calls BACnet stack <STRONG>frWork()</STRONG>.<BR>
 *  frWork() calls macWork().<BR>
 *  macWork() calls work functions for every network layer (Eth, PTP, IP, and MSTP).<BR>
 *  mstpWork() loops over all BACnet/MSTP ports and calls
 *  <STRONG>MasterNodeStateMachine()</STRONG> for every BACnet/MSTP port.
 *  </P>
 * \param[in] aTimeElasped    The time elapsed since previous call in milliseconds
 * \return None
 */
C_FUNC void bacnetMSTP_driver_executeStateMachine(uint32_t aTimeElasped);
#endif

/**
 * \ingroup BACnetMSTPDriver_USART
 * \brief
 * This function lets the BACnet stack process received messages (network and application layer)
 * \details
 *  <P>
 *  This function executes BACnet stack routine <STRONG>frMain()</STRONG>.<BR>
 *  frMain() calls <STRONG>frNLpump()</STRONG> to process network layer messages and
 *  <STRONG>frALpump()</STRONG> to process application layer messages.<BR>
 *  This function must be called at least every 5ms.
 *  </P>
 * @param None
 * @return None
 */
C_FUNC void bacnetMSTP_driver_executeBacApp(void);

/**
 * \ingroup BACnetMSTPDriver_USART
 * \brief
 *  This function returns the BACnet port ID for a particular UART Index.
 * \details Get the MSTP port (as identified by BACnet stack portMSTP0.. nMSTPports-1 for
 *  particular USART.
 * \note portMSTP0 may be greater than 0.
 * \param[in] uartIndex
 * \return BACnet port ID (portMSTP0 .. nMSTPports-1). nMSTPports indicates an error.
 */
C_FUNC unsigned bacnetMSTP_driver_uartIndexToMSTPPort(enum bapi_E_UartIndex_ uartIndex);

/**
 * \ingroup BACnetMSTPDriver_USART
 * \brief
 * This function returns the UART Index for a particular BACnet port ID.
 * @param[in] port    The BACnet port ID (portMSTP0 .. nMSTPports-1)
 * @return UART Index
 */
C_FUNC bapi_E_UartIndex bacnetMSTP_driver_MSTPPortToUartIndex(unsigned port);

/**
 * \ingroup bacnetMSTP_driver_SerialRx
 * \brief
 * This function is called by bacnet stack to get the cahracters received from MSTP UART
 *  This function is called from ReceiveFrameStateMachine
 *  </P>
 * @param None
 * @return None
 */
C_FUNC int bacnetMSTP_driver_SerialRx(uint8_t aPort);

C_FUNC bool bacnetMSTP_driver_IsBusy(uint8_t port);
C_FUNC bool tc_appSerialTxBuf(uint8_t port,uint8_t *b,uint8_t n);
#endif /* _BACNETMSTPDriver_DriverUsart_H_ */

