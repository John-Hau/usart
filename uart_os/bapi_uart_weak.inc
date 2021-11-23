/*
 * bapi_io_weak.c
 *
 *  Created on: 10.04.2013
 *      Author: e673505
 */

/** \file
 * \brief
 * Weak functions for applications that exclude the bapi-uart library (e.g. bootloader)
 */

#include "baseplate.h" 
 
#ifdef __GNUC__

  C_FUNC void _LPUART_DRV_IRQHandler_() {return;}
  C_FUNC void LPUART_DRV_IRQHandler() __attribute__ ((weak, alias ("_LPUART_DRV_IRQHandler_")));

#elif __IAR_SYSTEMS_ICC__

  C_FUNC __weak void LPUART_DRV_IRQHandler(void) {return;}

#endif
