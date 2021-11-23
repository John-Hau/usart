/*
 *  $HeadURL: $
 *
 *  $Date: $
 *  $Author: $
 */

#include "boards/board-api/bapi_usbdev.h"
#include "boards/board-api/bapi_usbdev_ci.hpp"

#if BAPI_HAS_USBDEV_CI > 0 /* If we are using any of the available USB device controller interfaces. */

#include "fsl_device_registers.h"
#include "Driver_USBD.h"
#include "utils/usb_misc.h"

#if defined(LCH_SYS_ENABLE_usb_device) && LCH_SYS_ENABLE_usb_device > 0
#include "boards/board-api/bapi_atomic.h"
#endif

#define _USBD_DEVICE_ASSERT_(expression) \
  ASSERT(expression)

#define ARM_USBD_DRV_VERSION    ARM_DRIVER_VERSION_MAJOR_MINOR(2, 0) /* driver version */


namespace _driver_usbd {

/* Driver Version */
STATIC const ARM_DRIVER_VERSION DriverVersion = {
    ARM_USBD_API_VERSION,
    ARM_USBD_DRV_VERSION
};

}

/*******************************************************************************
 * Code
 ******************************************************************************/

static ARM_DRIVER_VERSION  ARM_USBD_GetVersion (void){
  return _driver_usbd::DriverVersion;
}

template<enum bapi_E_UsbDevCiIndex_ ciIndex> struct _ARM_USBD {

  static ARM_USBD_CAPABILITIES GetCapabilities(void) {
    return *bapi_usbd_getCapabilities(ciIndex);
  }

  static int32_t Initialize(ARM_USBD_SignalDeviceEvent_t cb_device_event,
    ARM_USBD_SignalEndpointEvent_t cb_endpoint_event) {
    /* Allocate a device handle by using the controller id. */
    struct bapi_usbdev_ControllerInterface *controllerInterface = bapi_usb_device_getCiInterface(ciIndex);
    _USBD_DEVICE_ASSERT_(controllerInterface);

    /* Clear the device address */
    controllerInterface->ciIndex = ciIndex;
    controllerInterface->init(cb_device_event, cb_endpoint_event);

    /* Set the device to default state */
    return ARM_DRIVER_OK;
  }

  static int32_t Uninitialize(void) {
    struct bapi_usbdev_ControllerInterface *controllerInterface = bapi_usb_device_getCiInterface(ciIndex);
    _USBD_DEVICE_ASSERT_(controllerInterface);
    return controllerInterface->deinit();
  }

  static int32_t PowerControl(ARM_POWER_STATE state) {
    return ARM_DRIVER_ERROR_UNSUPPORTED; /* TODO: Implement this (when it makes sense). */
  }

  static int32_t DeviceConnect(void) {
    struct bapi_usbdev_ControllerInterface *controllerInterface = bapi_usb_device_getCiInterface(ciIndex);
    _USBD_DEVICE_ASSERT_(controllerInterface);
    return controllerInterface->connect();
  }

  static int32_t DeviceDisconnect(void) {
    struct bapi_usbdev_ControllerInterface *controllerInterface = bapi_usb_device_getCiInterface(ciIndex);
    _USBD_DEVICE_ASSERT_(controllerInterface);
    return controllerInterface->disconnect();
  }

  static ARM_USBD_STATE DeviceGetState(void) {
    struct bapi_usbdev_ControllerInterface *controllerInterface = bapi_usb_device_getCiInterface(ciIndex);
    _USBD_DEVICE_ASSERT_(controllerInterface);
    return controllerInterface->deviceGetState();
  }

  static int32_t DeviceRemoteWakeup(void) {
    struct bapi_usbdev_ControllerInterface *controllerInterface = bapi_usb_device_getCiInterface(ciIndex);
    _USBD_DEVICE_ASSERT_(controllerInterface);
    return controllerInterface->deviceRemoteWakeup();
  }

  static int32_t DeviceSetAddress(uint8_t dev_addr) {
    struct bapi_usbdev_ControllerInterface *controllerInterface = bapi_usb_device_getCiInterface(ciIndex);
    _USBD_DEVICE_ASSERT_(controllerInterface);
    return controllerInterface->deviceSetAddress(dev_addr);
  }

  static int32_t ReadSetupPacket(uint8_t *setup) {
    struct bapi_usbdev_ControllerInterface *controllerInterface = bapi_usb_device_getCiInterface(ciIndex);
    _USBD_DEVICE_ASSERT_(controllerInterface);
    return controllerInterface->readSetupPacket(setup);
  }

  static int32_t EndpointConfigure(uint8_t ep_addr, enum _ARM_USB_ENDPOINT_TYPE ep_type, uint16_t ep_max_packet_size)
    {
    struct bapi_usbdev_ControllerInterface *controllerInterface = bapi_usb_device_getCiInterface(ciIndex);
    _USBD_DEVICE_ASSERT_(controllerInterface);
    return controllerInterface->endpointConfigure(ep_addr, ep_type, ep_max_packet_size);
  }

  static int32_t EndpointUnconfigure(uint8_t ep_addr) {
    struct bapi_usbdev_ControllerInterface *controllerInterface = bapi_usb_device_getCiInterface(ciIndex);
    _USBD_DEVICE_ASSERT_(controllerInterface);
    return controllerInterface->endpointUnconfigure(ep_addr);
  }

  static int32_t EndpointStall(uint8_t ep_addr, bool stall) {
    struct bapi_usbdev_ControllerInterface *controllerInterface = bapi_usb_device_getCiInterface(ciIndex);
    _USBD_DEVICE_ASSERT_(controllerInterface);
    return controllerInterface->endpointStall(ep_addr, stall);
  }

  static int32_t EndpointTransfer(uint8_t ep_addr, uint8_t *data, uint32_t num) {
    struct bapi_usbdev_ControllerInterface *controllerInterface = bapi_usb_device_getCiInterface(ciIndex);
    return controllerInterface->endpointTransfer(ep_addr, data, num);
  }

  static uint32_t EndpointTransferGetResult(uint8_t ep_addr) {
    struct bapi_usbdev_ControllerInterface *controllerInterface = bapi_usb_device_getCiInterface(ciIndex);
    _USBD_DEVICE_ASSERT_(controllerInterface);
    return controllerInterface->endpointTransferGetResult(ep_addr);
  }

  static int32_t EndpointTransferAbort(uint8_t ep_addr) {
    struct bapi_usbdev_ControllerInterface *controllerInterface = bapi_usb_device_getCiInterface(ciIndex);
    ASSERT(NULL != controllerInterface);
    return controllerInterface->endpointTransferAbort(ep_addr);
  }

  static uint16_t GetFrameNumber(void) {
    struct bapi_usbdev_ControllerInterface *controllerInterface = bapi_usb_device_getCiInterface(ciIndex);
    ASSERT(NULL != controllerInterface);
    return controllerInterface->getFrameNumber();
  }

  static USBD_ENDPOINT_STATE EndpointGetState(uint8_t ep_addr) {
    struct bapi_usbdev_ControllerInterface *controllerInterface = bapi_usb_device_getCiInterface(ciIndex);
    ASSERT(NULL != controllerInterface);
    return controllerInterface->endpointGetState(ep_addr);
  }

};

#define _USBD_DRIVER_VALUE_(ciIndex) { \
  ARM_USBD_GetVersion, \
  _ARM_USBD<ciIndex>::GetCapabilities, \
  _ARM_USBD<ciIndex>::Initialize, \
  _ARM_USBD<ciIndex>::Uninitialize, \
  _ARM_USBD<ciIndex>::PowerControl, \
  _ARM_USBD<ciIndex>::DeviceConnect, \
  _ARM_USBD<ciIndex>::DeviceDisconnect, \
  _ARM_USBD<ciIndex>::DeviceGetState, \
  _ARM_USBD<ciIndex>::DeviceRemoteWakeup, \
  _ARM_USBD<ciIndex>::DeviceSetAddress, \
  _ARM_USBD<ciIndex>::ReadSetupPacket, \
  _ARM_USBD<ciIndex>::EndpointConfigure, \
  _ARM_USBD<ciIndex>::EndpointUnconfigure, \
  _ARM_USBD<ciIndex>::EndpointStall, \
  _ARM_USBD<ciIndex>::EndpointTransfer, \
  _ARM_USBD<ciIndex>::EndpointTransferGetResult, \
  _ARM_USBD<ciIndex>::EndpointTransferAbort, \
  _ARM_USBD<ciIndex>::GetFrameNumber, \
  _ARM_USBD<ciIndex>::EndpointGetState \
  }

static const struct _ARM_DRIVER_USBD s_usbDrivers[] = {
  _USBD_DRIVER_VALUE_(bapi_E_UsbDevCi0)
#if BAPI_HAS_USBDEV_CI > 1
  ,_USBD_DRIVER_VALUE_(bapi_E_UsbDevCi1)
#endif
#if BAPI_HAS_USBDEV_CI > 2
  ,_USBD_DRIVER_VALUE_(bapi_E_UsbDevCi2)
#endif
#if BAPI_HAS_USBDEV_CI > 3
  ,_USBD_DRIVER_VALUE_(bapi_E_UsbDevCi3)
#endif
#if BAPI_HAS_USBDEV_CI > 4
  #error "More than 4 USB Device Contoller interfaces defined. Please enhance according to the scheme above."
#endif
};

C_FUNC ARM_DRIVER_USBD* driver_usbd_getDriver(
  enum bapi_E_UsbDevCiIndex_ ciIndex /**< [in] The USBD index for which to obtain the driver. */
  ) {
  return &s_usbDrivers[ciIndex];
}


#endif /* #if BAPI_HAS_USBDEV_CI > 0 */

