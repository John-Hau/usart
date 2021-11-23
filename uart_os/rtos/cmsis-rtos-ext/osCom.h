/*
 * osCom.h
 *
 *  Created on: 07.04.2013
 *      Author: Wolfgang
 */

#ifndef osCom_H_
#define osCom_H_


#include "baseplate.h"
#include "boards/board-api/bapi_uart.h"
#include "cmsis-driver/Driver_USART.h"

#ifdef __IAR_SYSTEMS_ICC__
  #include <LowLevelIOInterface.h>
#else
  #include <unistd.h>
#endif

/* Compile time option */
#define OS_COM_ENABLE_WRITE_WITH_FEEDBACK 1

/**
 * \file
 * This file defines the ARM CMSIS RTOS COM Extension API.
 */

/**
* \ingroup cmsis_os_ext_com
* \def STDIN_FILENO
* The file descriptor for standard in
*/
/**
* \ingroup cmsis_os_ext_com
* \def STDOUT_FILENO
* The file descriptor for standard out
*/
/**
* \ingroup cmsis_os_ext_com
* \def STDERR_FILENO
* The file descriptor for standard err
*/

/**
 * \ingroup cmsis_os_ext_com
 * The number of file descriptors that are reserved for communication devices.
 * There are the following file descriptors reserved:
 *
 *  - 0 -> stdin
 *  - 1 -> stdout
 *  - 2 -> stderr
 *
 *
 * Other communication devices can be assigned file descriptor numbers from 3 to (DEV_FD_COUNT - 1)
 *
 * */

/**
 * \name Standard file descriptors
 */
/* @{ */
#ifndef STDIN_FILENO
  #define STDIN_FILENO 0
#endif
#ifndef STDOUT_FILENO
  #define STDOUT_FILENO 1
#endif
#ifndef STDERR_FILENO
  #define STDERR_FILENO 2
#endif
/* @} */


enum {
    DEV_FD_UNINITIALIZING = STDIN_FILENO - 3
   ,DEV_FD_INITIALIZING   = STDIN_FILENO - 2
   ,DEV_FD_INVALID        = STDIN_FILENO - 1

   /*
    * Valid device file descriptors from 0 .. 127
    */

   ,DEV_FD_COUNT = 128
};




/**
 * \ingroup _cmsis_os_ext_com
 * \brief get the UART index for a file descriptor.
 *
 * \note Works also in a Non RTOS environment.
 *
 * @return The UART index if there is any associated with this file descriptor.
 * Otherwise bapi_E_Uart_Invalid.
 *
 */
C_FUNC enum bapi_E_UartIndex_ _osComFd2Usart(
  int fd /**< The file descriptor for which to look up the UART index. */
  );

/**
 * \ingroup cmsis_os_ext_com
 * \brief get the ARM USART driver for a file descriptor.
 *
 * \note Works also in a Non RTOS environment.
 *
 * @return Pointer to the ARM USART driver if there is any associated with this file descriptor.
 * Otherwise NULL.
 *
 */
C_INLINE ARM_DRIVER_USART* osComArmDriverUsartGetFromFd(
  int fd /**< The file descriptor for which to look up UART driver. */
  ) {
  enum bapi_E_UartIndex_ uartIndex = _osComFd2Usart(fd);
  return driver_usart_getDriver(uartIndex);
}

/**
 * \ingroup cmsis_os_ext_com
 * \deprecated
 * \brief get the ARM USART driver for a USART index.
 *
 * \note Works also in a Non RTOS environment.
 *
 * @return Pointer to the ARM USART driver if successful.
 * Otherwise NULL.
 */
C_INLINE ARM_DRIVER_USART* osComArmDriverUsartGet(
  bapi_E_UartIndex uartIndex /**< [in] The USART index for which to obtain the driver. */
) {
  return driver_usart_getDriver(uartIndex);
}

/**
 * \ingroup cmsis_os_ext_com
 * \brief
 * Initialize a particular UART. Assign a file descriptor and create transmission and receive queues.
 *
 * \note Works also in a Non RTOS environment.
 *
 * @return Either \code ARM_DRIVER_OK \endcode or any ARM_DRIVER_ERROR_* code.
 */
C_FUNC int32_t osComUsartInitialize(
  bapi_E_UartIndex uartIndex /**< The UART to be initialized */
  ,int fd                     /**< The file descriptor that shall be associated with the UART. */
  ,uint16_t txQueueSize       /**< The size of the send queue for that UART. */
  );

/**
 * \ingroup cmsis_os_ext_com
 * \brief
 * Initialize the UART that shall be associated with the console. The file descriptors for
 * stdin, stdout and stderr are assigned implicitely.
 *
 * \note Works also in a Non RTOS environment.
 *
 * @return Either \code ARM_DRIVER_OK \endcode or any ARM_DRIVER_ERROR_* code.
 */
C_FUNC int32_t osComConsoleInitialize(
  bapi_E_UartIndex uartIndex /**< The UART to be initialized */
  ,uint16_t txQueueSize      /**< The size of the send queue for that UART. */
  );


/**
 * \ingroup cmsis_os_ext_com
 * \brief
 * Uninitialize a USART and free up all associated file descriptors.
 *
 * \note Works also in a Non RTOS environment.
 *
 * @return Either \code ARM_DRIVER_OK \endcode or any ARM_DRIVER_ERROR_* code.
 */
C_FUNC int32_t osComUsartUninitialize(
  bapi_E_UartIndex uartIndex /**< The UART to be uninitialized */
  );

/**
 * \ingroup cmsis_os_ext_com
 * \brief
 * Pass this value as msecBlockTime parameter to osComRead(int, char, int, MsecType, bool),
 * if you want to let osComRead(int, char, int, MsecType, bool) only return, if at least
 * one character is available.
 */
#define osComWaitForever MSEC_MAX

/**
 * \ingroup cmsis_os_ext_com
 * \brief
 * Nonblocking raw read from a device.
 *
 * \note Works also in a Non RTOS environment.
 *
 * @return The number of characters read, or -1 if no character available
 */
C_FUNC int osComRead(
  int fd,                 /**< [in] Descriptor for the device you want to read from. */
  char *ptr,              /**< [in] Pointer to the characters that are being read. */
  int len,                /**< [in] Number of characters to be read. */
  MsecType msecBlockTime, /**< [in] Timeout in milliseconds by when the function will return
                           * even without having 'len' number of characters received.Can be
                           * set to osComWaitForever to wait infinite for a character.
                           * This parameter is only applicable, in case that an RTOS is available.
                           * In case no RTOS is available, the function will
                           * always return immediately. */
  bool flushFirst         /**< [in] If there are any bytes already received, flush them first. */
  );

/**
 * \ingroup cmsis_os_ext_com
 * \brief
 * Unbuffered read of a single character from a device. This function is optionally non-blocking.
 *
 * \note Works also in a Non RTOS environment.
 *
 * \note The C-Library getc() may allocate 1Kb of buffer just to read a single character.
 * Hence this function has a much smaller RAM consumption.
 * @return The read character, or -1 if no character read.
 */
C_INLINE int osComGetc(
   int fd                 /**< [in] Descriptor for the device you want to read from. */
  ,MsecType msecBlockTime /**< [in] Timeout in milliseconds by when the function will return
                           * even without having a character received. Can be set to
                           * osComWaitForever to wait infinite for a character. */
  ) {
  char c;
  if( osComRead(fd, &c, sizeof(c), msecBlockTime, false) <= 0 ) {
    return -1;
  }

  return c;
}

/**
 * \ingroup cmsis_os_ext_com
 * \brief
 * Unbuffered read of a single character from the stdin device. This function is optionally non-blocking.
 *
 * \note Works also in a Non RTOS environment.
 *
 * @return The read character, or -1 if no character read.
 */
C_INLINE int osComGetchar(
   MsecType msecBlockTime /**< [in] Timeout in milliseconds by when the function will return
                           * even without having a character received. Can be set to
                           * osComWaitForever to wait infinite for a character. */
  ) {
  return osComGetc(STDIN_FILENO, msecBlockTime);
}


#if OS_COM_ENABLE_WRITE_WITH_FEEDBACK

/**
 * \ingroup cmsis_os_ext_com
 * \brief
 * The callback to be called when a message transmission is finalized.
 *
 * \note Works also in a Non RTOS environment.
 *
 * \warning. This callback is running in an ISR (Interrupt Service Routine) context.
 *   That means, that it must be quick and and not do any memory allocation.
 *
 * @param fd The file descriptor that identifies the communication channel.
 * @param event The transmission event as per ARM_CMSIS_DRIVER:
 *    - ARM_USART_EVENT_TX_COMPLETE
 *    - ARM_USART_EVENT_SEND_COMPLETE
 * @param userParam The userParam value that was passed to the osComWriteWithFeedback()
 */
typedef void(*osComWriteCallback_t)(int fd, uint32_t event, void* userParam);


/**
 * \ingroup cmsis_os_ext_com
 * \brief
 *  Write to a device.
 *
 * \note Works also in a Non RTOS environment.
 *
 * @return
 * Number of characters that have been written, or
 * ARM_DRIVER_ERROR_BUSY if send queue is full.
 */
C_FUNC int osComWriteWithFeedback(
  int fd,                 /**< [in] File descriptor for the device you want to write to. */
  const char *ptr,        /**< [in] Pointer to the characters to write. */
  int len,                /**< [in] Number of characters to be written. */
  osComWriteCallback_t callback,  /**< [in] The callback function that will be called when
                                   * the message transmission of this message is complete.
                                   */
  void* userParam                 /**< [in] The value of this parameter will be passed to
                                   * the callback
                                   */
  );


#endif /* #if OS_COM_ENABLE_WRITE_WITH_FEEDBACK */

/**
 * \ingroup cmsis_os_ext_com
 * \brief
 *  Write to a device.
 *
 * \note Works also in a Non RTOS environment.
 *
 * @return
 * Number of characters that have been written, or
 * ARM_DRIVER_ERROR_BUSY if send queue is full.
 */
#if OS_COM_ENABLE_WRITE_WITH_FEEDBACK

C_INLINE int osComWrite(int fd, const char *ptr, int len) {
  return osComWriteWithFeedback(fd, ptr, len, 0, 0);
}

#else
C_FUNC int osComWrite(
  int fd,                 /**< [in] File descriptor for the device you want to write to. */
  const char *ptr,        /**< [in] Pointer to the characters to write. */
  int len                 /**< [in] Number of characters to be written. */
  );
#endif



#endif /* osCom_H_ */
