/*
 *  $HeadURL: $
 *
 *  $Date: $
 *  $Author: $
 */

#ifndef uart_address_map_FS_SNAP_ON_IO_H_
#define uart_address_map_FS_SNAP_ON_IO_H_

/** \file
 * \brief
 * This file defines the uart index enumeration for the FS_IRMFCU board.
 * */

#define BAPI_HAS_USART 8

/**
 * \addtogroup bapi_uart
 * @{
 */

enum bapi_E_UartIndex_ {
#if (BAPI_HAS_USART > 0)
   bapi_E_Uart_Invalid = -1         /**< Generic identifier for UART instance 0. Complies with the vendor documentation. */
  ,bapi_E_Uart1                     /**< Generic identifier for UART instance 1. Complies with the vendor documentation. */
  ,bapi_E_Uart2                     /**< Generic identifier for UART instance 2. Complies with the vendor documentation. */
  ,bapi_E_Uart3                     /**< Generic identifier for UART instance 3. Complies with the vendor documentation. */
  ,bapi_E_Uart4                     /**< Generic identifier for UART instance 4. Complies with the vendor documentation. */
  ,bapi_E_Uart5                     /**< Generic identifier for UART instance 5. Complies with the vendor documentation. */
  ,bapi_E_Uart6                     /**< Generic identifier for UART instance 5. Complies with the vendor documentation. */
  ,bapi_E_Uart7                     /**< Generic identifier for UART instance 5. Complies with the vendor documentation. */
  ,bapi_E_Uart8                     /**< Generic identifier for UART instance 5. Complies with the vendor documentation. */
#endif
   ,bapi_E_UartCount                 /**< Number of UARTs. */
  ,bapi_E_UartLast = bapi_E_UartCount - 1
#if (BAPI_HAS_USART <= 0)
  ,bapi_E_Uart0 = bapi_E_UartCount
#endif
};

/** @}*/

#endif /* uart_address_map_FS_SNAP_ON_IO_H_ */
