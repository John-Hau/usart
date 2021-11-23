/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __USB_DEVICE_H__
#define __USB_DEVICE_H__


/**
 * #016 29.Jan.2016 WSC: Use forward declarated struct bapi_usbdev_ControllerInterface for device
 *   handle for better debugging.
 */
#include "baseplate.h"
#include "boards/board-api/bapi_usbdev.h"
#include "boards/cmsis/usb_spec_driver.h"
#include "boards/cmsis/Driver_USBD.h"

#include "utils/utils.h"


/*! @brief USB error code TODO: consolidate _usb_status and arm driver error codes. */
enum _usb_status {
  kStatus_USB_Success           = ARM_DRIVER_OK,                  /*!< Success */
  kStatus_USB_Error             = ARM_DRIVER_ERROR,               /*!< Failed */
  kStatus_USB_Busy              = ARM_DRIVER_ERROR_BUSY,          /*!< Busy */
  kStatus_USB_Timeout           = ARM_DRIVER_ERROR_TIMEOUT,
  kStatus_USB_InvalidParameter  = ARM_DRIVER_ERROR_PARAMETER,     /*!< Invalid parameter */
  kStatus_USB_NotSupported      = ARM_DRIVER_ERROR_UNSUPPORTED,   /*!< Configuration is not supported */

  kStatus_USB_InvalidHandle     = ARM_DRIVER_ERROR_SPECIFIC - 101, /*!< Invalid handle */
  kStatus_USB_InvalidRequest    = ARM_DRIVER_ERROR_SPECIFIC - 102, /*!< Invalid request */
  kStatus_USB_Retry             = ARM_DRIVER_ERROR_SPECIFIC - 103, /*!< Enumeration get configuration retry */
  kStatus_USB_TransferStall     = ARM_DRIVER_ERROR_SPECIFIC - 104, /*!< Transfer stalled */
  kStatus_USB_TransferFailed    = ARM_DRIVER_ERROR_SPECIFIC - 105, /*!< Transfer failed */
  kStatus_USB_AllocFail         = ARM_DRIVER_ERROR_SPECIFIC - 106, /*!< Allocation failed */
  kStatus_USB_LackSwapBuffer    = ARM_DRIVER_ERROR_SPECIFIC - 107, /*!< Lack the swap buffer for khci */
  kStatus_USB_TransferCancel    = ARM_DRIVER_ERROR_SPECIFIC - 108, /*!< The transfer canceled */
  kStatus_USB_BandwidthFail     = ARM_DRIVER_ERROR_SPECIFIC - 109, /*!< Allocate bandwidth failed */
  kStatus_USB_MSDStatusFail     = ARM_DRIVER_ERROR_SPECIFIC - 110, /*!< For msd, the csw status means fail */
};


/*! @brief Whether the device task is enabled.
 * \warning If the use task is enabled without RTOS the main thread must
 *  call usb_DeviceTaskFunction() without short latencies, otherwise,
 *  the setup phase might fail.
 *
 *  TODO: Allow the Setup phase to run totally within ISR context, while
 *    the class communication (e.g. mass storage runs in thread context.
 *    This will reduce the probability of a setup phase failure.
 */
#define USB_DEVICE_CONFIG_USE_TASK     (1)

/*! @brief How many the notification message are supported when the device task enabled. */
#define USB_DEVICE_CONFIG_MAX_MESSAGES (16)


/*!
 * @addtogroup usb_device_driver
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! @brief Defines USB device stack version major*/
#define USB_DEVICE_STACK_VERSION_MAJOR  (0x01U)
/*! @brief Defines USB device stack version minor*/
#define USB_DEVICE_STACK_VERSION_MINOR  (0x00U)
/*! @brief Defines USB device stack version bugfix*/
#define USB_DEVICE_STACK_VERSION_BUGFIX (0x00U)


/*! @brief Control endpoint maxPacketSize */
#define USB_CONTROL_MAX_PACKET_SIZE (64U)

#if (FSL_USB_EHCI_COUNT && (USB_CONTROL_MAX_PACKET_SIZE != (64U)))
#error For high speed, USB_CONTROL_MAX_PACKET_SIZE must be 64!!!
#endif

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus*/


/*!
 * @name USB device APIs
 * @{
 */


/*******************************************************************************
 * API
 ******************************************************************************/
/**
\brief Access structure of the USB Device Driver.
*/
typedef struct _ARM_DRIVER_USBD {
  ARM_DRIVER_VERSION    (*GetVersion)                (void);                                              ///< Pointer to \ref ARM_USBD_GetVersion : Get driver version.
  ARM_USBD_CAPABILITIES (*GetCapabilities)           (void);                                              ///< Pointer to \ref ARM_USBD_GetCapabilities : Get driver capabilities.
  int32_t               (*Initialize)                 (ARM_USBD_SignalDeviceEvent_t   cb_device_event,
                                                       ARM_USBD_SignalEndpointEvent_t cb_endpoint_event); ///< Pointer to \ref ARM_USBD_Initialize : Initialize USB Device Interface.
  int32_t               (*Uninitialize)              (void);                                              ///< Pointer to \ref ARM_USBD_Uninitialize : De-initialize USB Device Interface.
  int32_t               (*PowerControl)              (ARM_POWER_STATE state);                             ///< Pointer to \ref ARM_USBD_PowerControl : Control USB Device Interface Power.
  int32_t               (*DeviceConnect)             (void);                                              ///< Pointer to \ref ARM_USBD_DeviceConnect : Connect USB Device.
  int32_t               (*DeviceDisconnect)          (void);                                              ///< Pointer to \ref ARM_USBD_DeviceDisconnect : Disconnect USB Device.
  ARM_USBD_STATE        (*DeviceGetState)            (void);                                              ///< Pointer to \ref ARM_USBD_DeviceGetState : Get current USB Device State.
  int32_t               (*DeviceRemoteWakeup)        (void);                                              ///< Pointer to \ref ARM_USBD_DeviceRemoteWakeup : Trigger USB Remote Wakeup.
  int32_t               (*DeviceSetAddress)          (uint8_t dev_addr);                                  ///< Pointer to \ref ARM_USBD_DeviceSetAddress : Set USB Device Address.
  int32_t               (*ReadSetupPacket)           (uint8_t *setup);                                    ///< Pointer to \ref ARM_USBD_ReadSetupPacket : Read setup packet received over Control Endpoint.
  int32_t               (*EndpointConfigure)         (uint8_t ep_addr,
                                                       enum _ARM_USB_ENDPOINT_TYPE ep_type,
                                                       uint16_t ep_max_packet_size);                      ///< Pointer to \ref ARM_USBD_EndpointConfigure : Configure USB Endpoint.
  int32_t               (*EndpointUnconfigure)       (uint8_t ep_addr);                                   ///< Pointer to \ref ARM_USBD_EndpointUnconfigure : Unconfigure USB Endpoint.
  int32_t               (*EndpointStall)             (uint8_t ep_addr, bool stall);                       ///< Pointer to \ref ARM_USBD_EndpointStall : Set/Clear Stall for USB Endpoint.
  int32_t               (*EndpointTransfer)          (uint8_t ep_addr, uint8_t *data, uint32_t num);      ///< Pointer to \ref ARM_USBD_EndpointTransfer : Read data from or Write data to USB Endpoint.
  uint32_t              (*EndpointTransferGetResult) (uint8_t ep_addr);                                   ///< Pointer to \ref ARM_USBD_EndpointTransferGetResult : Get result of USB Endpoint transfer.
  int32_t               (*EndpointTransferAbort)     (uint8_t ep_addr);                                   ///< Pointer to \ref ARM_USBD_EndpointTransferAbort : Abort current USB Endpoint transfer.
  uint16_t              (*GetFrameNumber)            (void);                                              ///< Pointer to \ref ARM_USBD_GetFrameNumber : Get current USB Frame Number.
  USBD_ENDPOINT_STATE   (*EndpointGetState)          (uint8_t ep_addr);                                   ///< 28.Feb.2016 WSC: Extension: Pointer to \ref ARM_USBD_EndpointGetState : Get current USB Device Endpoint State.
} const ARM_DRIVER_USBD;

/**
 * \ingroup cmsis_driver_usbd
 * \brief
 * Obtain the CMSIS _ARM_DRIVER_USART access structure for a particular USART
 * by USART index.
 *
 * The CMSIS Driver specifications defines just fixed names for the USBD
 * driver instances. This does badly support access to drivers at runtime
 * via USBD index. This supplementary function closes this gap.
 *
 * \return Pointer to the USART Driver structure.
 */
C_FUNC ARM_DRIVER_USBD* driver_usbd_getDriver(
  enum bapi_E_UsbDevCiIndex_ ciIndex /**< [in] The USBD index for which to obtain the driver. */
  );

#if ((defined(FSL_USB_KHCI_COUNT)) && (FSL_USB_KHCI_COUNT > 0U))
struct bapi_usbdev_ControllerInterface;
/*!
 * @brief Device KHCI isr function.
 * \deprecated
 *
 * The function is KHCI interrupt service routine.
 *
 * @param[in] handle The device handle got from #USB_DeviceInit.
 */
C_FUNC void USB_DeviceKhciIsrFunction(struct bapi_usbdev_ControllerInterface *controllerInterface);
#endif

#if ((defined(FSL_USB_EHCI_COUNT)) && (FSL_USB_EHCI_COUNT > 0U))
struct bapi_usbdev_ControllerInterface;
/*!
 * @brief Device EHCI isr function.
 * \deprecated
 *
 * The function is EHCI interrupt service routine.
 *
 * @param[in] handle The device handle got from #USB_DeviceInit.
 */
C_FUNC void USB_DeviceEhciIsrFunction(struct bapi_usbdev_ControllerInterface *controllerInterface);
}
#endif


/*! @}*/

#if defined(__cplusplus)
}
#endif /* __cplusplus*/

/*! @}*/

#endif /* __USB_DEVICE_H__ */
