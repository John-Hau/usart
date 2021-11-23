
#ifndef _bufferDriver_DriverUsart_H_
#define _bufferDriver_DriverUsart_H_

#include "baseplate.h"
#include "cmsis-driver/Driver_USART.h"

/**
 * @file
 * @brief This file declares the API of a usart filter that can be hooked
 * into the CMSIS USART Driver: Buffering USART Filter. It buffers received
 * characters, when the CMSIS Driver is currently not in Receive Mode (not
 * able to receive characters).
 *
 */

/**
 * @defgroup buffering_usart_filter Buffering USART Filter
 * @ingroup usart_filters
 * @brief This filter buffers received characters, when the CMSIS Driver is
 * currently not in Receive Mode.
 *
 * @sa cmsis_driver_usart_ext_hook
 */

/**
 * @ingroup buffering_usart_filter
 * @brief This function hooks the filter into the CMSIS USART driver.
 *
 * @param uartIndex  [in] The USART for which to unhook this filter.
 * @param bufferSize [in] Number of received characters this filter can buffer
 *   for the specified USART.
 *
 * @warning This function can only be called when the buffer driver for the
 * targeted USART is uninitialized!
 *
 */
C_FUNC int32_t buffering_usart_filter_hook(
	enum bapi_E_UartIndex_ uartIndex, uint16_t bufferSize
);

/**
 * @ingroup buffering_usart_filter
 * @brief This function unhooks the filter from the CMSIS USART driver.
 *
 * @param uartIndex [in] The USART for which to unhook this filter.
 *
 * @warning This function can only be called when the buffer driver for the
 * targeted USART is uninitialized!
 *
 */
C_FUNC int32_t buffering_usart_filter_unhook(
  enum bapi_E_UartIndex_ uartIndex
);

typedef uint16_t buffer_driver_index_t;

/**
 * @ingroup buffering_usart_filter
 * @brief Retrieve the current contents of the internal buffer.
 * This function is only for debugging this filter.
 */
C_FUNC buffer_driver_index_t buffering_usart_filter_getBufferContents(
  enum bapi_E_UartIndex_ uartIndex      /**< [in] The UART for which to retrieve the buffer contents. */
  , uint8_t* contents                   /**< [in] Pointer to memory location to copy buffer contents. */
  , buffer_driver_index_t max           /**< [in] Maximum number of uint8_t type characters to obtain.*/
  , buffer_driver_index_t* queueIndexFirst /**< [out] pointer to a variable receiving the array index
                                            * of the first character in the buffer.
                                            */
  , buffer_driver_index_t* queueIndexEnd   /**< [out] pointer to a variable receiving the array index
                                            * of the last character in the buffer. In case that the
                                            * retrieved value is equal to the first array index, the
                                            * buffer might be empty or totally full.
                                            */
);

/**
 * @ingroup buffering_usart_filter
 * @brief Remove received characters from the internal buffer.
 * This function is only for testing this filter.
 */
C_FUNC buffer_driver_index_t buffering_usart_filter_popBufferMultiple(
  enum bapi_E_UartIndex_ uartIndex              /**< The UART for which to pop data words from the buffer. */
  , buffer_driver_index_t count                 /**< Number of data words to pop. Zero flushes the buffer. */
  );

#endif /* _bufferDriver_DriverUsart_H_ */

