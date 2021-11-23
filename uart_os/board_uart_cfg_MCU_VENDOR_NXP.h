/*
 *  $HeadURL: $
 *
 *  $Date: $
 *  $Author: $
 */

#ifndef UART_ASYNCH_MCU_VENDOR_NXP_H_
#define UART_ASYNCH_MCU_VENDOR_NXP_H_

/**
 * \file
 * \brief This file declares the configuration data and runtime variables that need to be provided
 * by the board specific USART source files.
 */

#include "baseplate.h"

#include "boards/board-api/bapi_uart.h"
#include "boards/cmsis/Driver_USART.h"

/* The Kinetis SDK inline functions might have unused parameters. We want to ignore warnings for those. */
#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

//#include "fsl_sim_hal.h"
#include "fsl_device_registers.h"
#include "fsl_lpuart.h"

/* We don't want to ignore unused parameter warnings for our own code. */
#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif

/* Determin UART0_INSTANCE_COUNT if not provided by SDK */
#ifndef UART0_INSTANCE_COUNT
  #ifdef UART0_BASE_PTRS
    #define UART0_INSTANCE_COUNT (1)
  #else
    #define UART0_INSTANCE_COUNT (0)
  #endif
#endif

/* Determin UART_INSTANCE_COUNT if not provided by SDK */
#ifndef LPUART_INSTANCE_COUNT
  #if   defined LPUART9
    #define LPUART_INSTANCE_COUNT (10)
  #elif defined LPUART8
    #define LPUART_INSTANCE_COUNT  (8)
  #elif defined LPUART7
    #define LPUART_INSTANCE_COUNT  (7)
  #elif defined LPUART6
    #define LPUART_INSTANCE_COUNT  (6)
  #elif defined LPUART5
    #define LPUART_INSTANCE_COUNT  (5)
  #elif defined LPUART4
    #define LPUART_INSTANCE_COUNT  (4)
  #elif defined LPUART3
    #define LPUART_INSTANCE_COUNT  (3)
  #elif defined LPUART2
    #define LPUART_INSTANCE_COUNT  (2)
  #elif defined LPUART1
    #define LPUART_INSTANCE_COUNT  (1)
  #else
    #define LPUART_INSTANCE_COUNT  (0)
  #endif
#endif



#if LPUART_INSTANCE_COUNT > 0
/**
 * \ingroup _bapi_uart
 * \brief The common UART Rx/Tx ISR that will be called by all LPUART_IRQ handlers.
 */
C_FUNC void LPUART_DRV_IRQHandler(uint32_t instance);
#endif


/**
 * \ingroup _bapi_uart
 * \brief Enumeration for the 3 different Freescale uart types
 * \warning Type is used as index into an array.
 */
enum _fsl_E_uart_type {
   fslUartTypeIndexInvalid = -1
  ,fslUartTypeIndexUart    =  0
  ,fslUartTypeIndexLpuart  =  1
  ,fslUartTypeIndexLpsci   =  2
  ,fslUartTypeIndexCount
};


/**
 * \ingroup _bapi_uart
 * \brief
 * A structure holds the hardware memory address and interrupt number.
 */
struct _bapi_uart_properties {
  uint32_t   m_address;  /**< The memory address of the USART, LPUART or LPSCI */
  IRQn_Type  m_rxTxIRQn; /**< The RxTx  interrupt number of the USART, LPUART or LPSCI. May be identical with the Error interrupt number. */
  IRQn_Type  m_errIRQn;  /**< The Error interrupt number of the USART, LPUART or LPSCI. May be identical with the RxTx  interrupt number. */
  //uint8_t    m_fslInstance; /**< The Freescale HAL instance identifier of the  USART or LPSCI. */
  enum _fsl_E_uart_type m_uartType;
};

/*
 * \ingroup _bapi_uart
 * \brief For each defined bapi_E_Uart<x> the properties as per \ref struct _bapi_uart_properties.
 * \note This array is sequenced according to enum bapi_E_UartIndex_.
 */
C_DECL const struct _bapi_uart_properties _uart_properties[bapi_E_UartCount];

struct _bapi_uart_flagProperties {
  GPIO_Type*  m_portGpio;
  //PORT_Type * m_portBase;
  enum bapi_E_InterfaceFlag_ m_interfaceFlag;
  uint8_t    m_portPin;
};

/* This concept supports currently only one flag per UART. To be enhanced if required. */
C_DECL const struct _bapi_uart_flagProperties _uartFlagProperties[bapi_E_UartCount];

/**
 * \ingroup _bapi_uart
 * \brief Converts the bapi uart index to the lpsci instance of the Freescale SDK HAL
 */
C_INLINE uint32_t uartIndex2FslInstance(bapi_E_UartIndex uartIndex) {
  /* There is a 1 to 1 mapping */
  return LPUART_GetInstance((LPUART_Type *)_uart_properties[uartIndex].m_address);
}

/**
 * \ingroup _bapi_uart
 * \brief
 * A structure holds the uart indexes of a Freescale UART instance and an lpci instance.
 */
struct _fsl_uart_instance_properties {
  int8_t m_bapiUartInstanceUartIndex;   /**< The bapi uart index for a Freescale UART instance */

  /**
   * Clock src depending on the UART type:
   *    -) If uart type is a standard UART this value is always 0
   *    -) If uart type is LPUART, place values from \ref enum _clock_lpuart_src
   *    -) If uart type is LPSCI,  place values from \ref enum _clock_lpsci_src
   */
  int8_t m_clockUartSrc;

//  enum _fsl_E_uart_type m_uartType;
};

/*
 * \ingroup _bapi_uart
 * \brief For each available UART an LPSCI instance, the bapi uart index.
 *
 * \note This array is sequenced according to the UART respectively LPSCI instances.
 * It is the reverse mapping information of the m_fsfInstance of in _uart_properties[] array.
 * Both mappings must be consistent with each other.
 */
C_DECL const struct _fsl_uart_instance_properties _fsl_uartInstanceProperties[LPUART_INSTANCE_COUNT];

/**
 * \ingroup _bapi_uart
 * \brief Converts the UART instance of the Freescale SDK HAL to the bapi uart index
 */
C_INLINE enum bapi_E_UartIndex_ _fslUartInstance2UartIndex(uint32_t instance) {
  ASSERT(instance < ARRAY_SIZE(_fsl_uartInstanceProperties));
  return S_CAST(enum bapi_E_UartIndex_, _fsl_uartInstanceProperties[instance].m_bapiUartInstanceUartIndex);
}

#if LPUART_INSTANCE_COUNT > 0 && !_BAPI_NO_FS_LPUART_USAGE
/*
 * \ingroup _bapi_uart
 * \brief For each available LPUART , the bapi uart index.
 *
 * \note This array is sequenced according to the LPUART instances.
 * It is the reverse mapping information of the m_fslInstance of in _uart_properties[] array.
 * Both mappings must be consistent with each other.
 */
C_DECL const struct _fsl_uart_instance_properties _fsl_lpuartInstanceProperties[LPUART_INSTANCE_COUNT];

/**
 * \ingroup _bapi_uart
 * \brief Converts the LPUART instance of the Freescale SDK HAL to the bapi uart index
 */
C_INLINE enum bapi_E_UartIndex_ _fslLpuartInstance2UartIndex(uint32_t lpinstance) {
  ASSERT(lpinstance < ARRAY_SIZE(_fsl_lpuartInstanceProperties));
  return S_CAST(enum bapi_E_UartIndex_, _fsl_lpuartInstanceProperties[lpinstance-1].m_bapiUartInstanceUartIndex);
}
#endif

/**
 * \ingroup _bapi_uart
 * \brief Returns Uart type for a bapi uart index
 */
C_INLINE enum _fsl_E_uart_type _fslUartType(enum bapi_E_UartIndex_ uartIndex) {
  return _uart_properties[uartIndex].m_uartType;
}

/**
 * \ingroup _bapi_uart
 * \brief Holds all the USART releated call back functions.
 */
C_DECL struct _uart_Callbacks _uart_callbacks[bapi_E_UartCount];

/**
 * \ingroup _bapi_uart
 * \brief Holds the baud rate of UART peripherals
 */
C_DECL uint32_t _uart_Baudrate[bapi_E_UartCount];


/** ingroup _bapi_uart
 * \brief For each USART and interrupt type a counter that counts the disable interrupt nesting. */
C_DECL uint8_t _uart_irq_disable_count[bapi_uart_IRQT_Count][bapi_E_UartCount];

C_DECL bapi_E_UartMode _bapi_uart_mode[bapi_E_UartCount];

/** ingroup _bapi_uart
 * \brief For each USART a nesting counter that counts the disabling of the transmitter. */
C_DECL uint8_t _uart_disable_tx_count[bapi_E_UartCount];

/** ingroup _bapi_uart
 * \brief For each USART a nesting counter that counts the disabling of the receiver. */
C_DECL uint8_t _uart_disable_rx_count[bapi_E_UartCount];

#endif /* UART_ASYNCH_MCU_VENDOR_NXP_H_ */
