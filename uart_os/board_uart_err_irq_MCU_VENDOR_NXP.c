/*
 * board_uart_err_irq_MCU_VENDOR_NXP.c
 *
 *  Created on: 07.11.2019
 *      Author: e578153
 */


/**
 * \file
 * \brief This file implements USART error interrupt service handlers of MCUs that have a separate
 *   interrupt number for the USART errors. Some MCUs don't require this handlers because they
 *   share the Rx Tx interupt number with the USART error interrupt number.
 */

#include "hardware-board.h"
#include "board_uart_cfg_MCU_VENDOR_NXP.h"

/* The Kinetis SDK inline functions might have unused parameters. We want to ignore warnings for those. */
#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

#include "fsl_device_registers.h"

/* We don't want to ignore unused parameter warnings for our own code. */
#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif


#if defined (K64F12_SERIES) || defined (K24F12_SERIES) || defined (K66F18_SERIES)

#if (UART_INSTANCE_COUNT > 0)
/* Implementation of UART0 error handler named in startup code. */
extern void UART_DRV_IRQHandler(uint32_t instance);
void UART0_ERR_IRQHandler(void)
{
    UART_DRV_IRQHandler(0);
}
#endif

#if (UART_INSTANCE_COUNT > 1)
/* Implementation of UART1 error handler named in startup code. */
extern void UART_DRV_IRQHandler(uint32_t instance);
void UART1_ERR_IRQHandler(void)
{
    UART_DRV_IRQHandler(1);
}
#endif

#if (UART_INSTANCE_COUNT > 2)
/* Implementation of UART2 error handler named in startup code. */
extern void UART_DRV_IRQHandler(uint32_t instance);
void UART2_ERR_IRQHandler(void)
{
    UART_DRV_IRQHandler(2);
}
#endif

#if (UART_INSTANCE_COUNT > 3)
/* Implementation of UART3 error handler named in startup code. */
extern void UART_DRV_IRQHandler(uint32_t instance);
void UART3_ERR_IRQHandler(void)
{
    UART_DRV_IRQHandler(3);
}
#endif

#if (UART_INSTANCE_COUNT > 4)
/* Implementation of UART4 error handler named in startup code. */
extern void UART_DRV_IRQHandler(uint32_t instance);
void UART4_ERR_IRQHandler(void)
{
    UART_DRV_IRQHandler(4);
}
#endif

#if (UART_INSTANCE_COUNT > 5)
/* Implementation of UART5 error handler named in startup code. */
extern void UART_DRV_IRQHandler(uint32_t instance);
void UART5_ERR_IRQHandler(void)
{
    UART_DRV_IRQHandler(5);
}
#endif

#elif defined (KL46Z4_SERIES)

/* KL46Z4_SERIES MCUs don't have extra ERR handlers. */

#elif defined (KL17Z4_SERIES)

/* KL17Z4_SERIES MCUs don't have extra ERR handlers. */
#elif defined (MIMXRT1062_SERIES)

/* MIMXRT1062_SERIES MCUs don't have extra ERR handlers. */

#elif defined(MIMXRT1051_SERIES)

/* MIMXRT1051_SERIES MCUs don't have extra ERR handlers. */

#else
    #error "No valid CPU defined!"
#endif
